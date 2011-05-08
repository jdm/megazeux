/* MegaZeux
 *
 * Copyright (C) 1996 Greg Janson
 * Copyright (C) 1999 Charles Goetzman
 * Copyright (C) 2002 B.D.A. (Koji) - Koji_Takeo@worldmailer.com
 * Copyright (C) 2002 Gilead Kutnick <exophase@adelphia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "compat.h"
#include "platform.h"

#include "configure.h"
#include "event.h"
#include "helpsys.h"
#include "sfx.h"
#include "graphics.h"
#include "window.h"
#include "data.h"
#include "game.h"
#include "error.h"
#include "idput.h"
#include "audio.h"
#include "util.h"
#include "world.h"
#include "counter.h"
#include "run_stubs.h"
#include "network/network.h"
#include "network/host.h"

#include "debugger/debugger.h"
#include "debugger/debugger_child.h"
#include "debugger/breakpoint.h"
#include "fsafeopen.h"
#include "editor/robo_ed.h"
#include "editor/world.h"

#ifndef VERSION
#error Must define VERSION for MegaZeux version string
#endif

#ifndef VERSION_DATE
#define VERSION_DATE
#endif

#define CAPTION "MegaZeux " VERSION VERSION_DATE

#ifdef __amigaos__
#define __libspec LIBSPEC
#else
#define __libspec
#endif

#define _warn(...) warn("[child] " __VA_ARGS__)
#define _info(...) info("[child] " __VA_ARGS__)

static bool process_network_input(struct host *parent, struct world *mzx_world,
                                  struct robot *robot);
static bool process_message(char *message, struct world *mzx_world,
                            struct robot *robot);

struct host *parent;

static int line_number = 0;

static int offset_to_line_number(struct robot *robot, int offset)
{
    char *program = robot->program_bytecode;
    int cur_offset = 1;
    int current_line_num = 1;
    while(cur_offset < offset)
    {
        current_line_num++;
        cur_offset += program[cur_offset] + 2;
    }
    return current_line_num;
}

static int get_message_arg(char *message, int arg)
{
    int val;
    memcpy(&val, message + 1 + sizeof(val) * arg, sizeof(val));
    return val;
}

static bool process_message(char *message, struct world *mzx_world,
                            struct robot *robot)
{
    switch(message[0])
    {
        case RELOAD_PROGRAM:
        {
            FILE *fp = fsafeopen(DEBUGGER_BYTECODE, "rb");
            int size, offset;

            offset = get_message_arg(message, 0);
            _info("reloading program at offset %d\n", offset);
            if(!fp) {
                _warn("bytecode file missing\n");
                return false;
            }
            size = ftell_and_rewind(fp);
            reallocate_robot(robot, size);
            fread(robot->program_bytecode, size, 1, fp);
            fclose(fp);

            offset = get_message_arg(message, 0);
            robot->cur_prog_line = offset;
            line_number = offset_to_line_number(robot, offset);
            _info("setting current line to %d\n", line_number);

            clear_breakpoints(mzx_world);
            //mzx_world->debug_watch.watch = robot; //XXX not needed if we hardcode global robot
            break;
        }

        case CURRENT_LINE:
        {
            int offset = get_message_arg(message, 0);
            _info("setting current offset to %d\n", offset);
            if(follow_active_line)
               line_number = offset_to_line_number(robot, offset);
            robot->cur_prog_line = offset;
            _info("setting current line to %d\n", line_number);
            break;
        }

        case TOGGLE_BREAKPOINT:
        {
            int offset = get_message_arg(message, 0);
            int line = offset_to_line_number(robot, offset);
            _info("setting breakpoint at %d (offset %d)\n", line, offset);
            toggle_breakpoint(mzx_world, robot, line);
            break;
        }

        case UPDATE_COORDS:
        {
            int x = get_message_arg(message, 0);
            int y = get_message_arg(message, 1);
            robot->xpos = x;
            robot->ypos = y;
            break;
        }

        case STOP_PROCESS:
            _info("abort abort\n");
            return false;

        case STEP:
        case CONTINUE:
        case SWITCH_WATCH:
        case BREAK:
            _warn("Child->Parent message received in child process\n");
            return false;

        default:
            _warn("Nonsense message type (%d) received.\n", message[0]);
            return false;
    }
    return true;
}

static bool process_network_input(struct host *parent, struct world *mzx_world,
                                  struct robot *robot)
{
  char buffer[80];
  int i = 10;
  while (i--)
  {
    int status;
    status = host_poll_raw(parent, 10);
    if(status < 0)
    {
        _warn("Parent communication error\n");
        return false;
    }
    else if(status > 0)
    {
        bool rv;
        if (!host_recv_raw(parent, buffer, 1) ||
            !host_recv_raw(parent, buffer + 1, message_size((enum message_type)buffer[0]) - 1))
        {
            _warn("Receive error\n");
            return false;
        }
        if (!process_message(buffer, mzx_world, robot))
          return false;
    }
    else return true;
  }
  return true;
}

__libspec int main(int argc, char *argv[])
{
    int err = 1;

    // Keep this 7.2k structure off the stack..
    static struct world mzx_world;

    if(!platform_init()) {
        _warn("error initializing platform\n");
        goto err_out;
    }

    // We need to store the current working directory so it's
    // always possible to get back to it..
    getcwd(current_dir, MAX_PATH);

    if(mzx_res_init(argv[0], 1)) {
        _warn("error initializing resources\n");
        goto err_free_res;
    }

    //editor_init();

    // Figure out where all configuration files should be loaded
    // form. For game.cnf, et al this should eventually be wrt
    // the game directory, not the config.txt's path.
    get_path(mzx_res_get_by_id(CONFIG_TXT), config_dir, MAX_PATH);

    // Move into the configuration file's directory so that any
    // "include" statements are done wrt this path. Move back
    // into the "current" directory after loading.
    _info("Changing to %s\n", config_dir);
    chdir(config_dir);

    default_config(&mzx_world.conf);
    set_config_from_file(&mzx_world.conf, mzx_res_get_by_id(CONFIG_TXT));
    set_config_from_command_line(&mzx_world.conf, &argc, argv);

    load_editor_config(&mzx_world, &argc, argv);

    // At this point argv should have all the config options
    // of the form var=value removed, leaving only unparsed
    // parameters. Interpret the first unparsed parameter
    // as a file to load (overriding startup_file etc.)
    //if(argc > 1)
    //    strncpy(mzx_world.conf.startup_file, argv[1], 256);

    //init_macros(&mzx_world);

    //chdir(current_dir);
    _info("Moving to %s\n", argv[1]);
    chdir(argv[1]);

    //counter_fsg();

    //initialize_joysticks();

    //set_mouse_mul(8, 14);

    if(!network_layer_init(&mzx_world.conf))
    {
        _warn("Network layer disabled.\n");
        goto err_free_res;
    }

    parent = host_create(HOST_TYPE_TCP, HOST_FAMILY_IPV4);
    if(!parent)
    {
        _warn("Error creating host\n");
        goto err_destroy_host_exit;
    }

    if(!host_connect(parent, "localhost", DEBUGGER_PORT))
    {
        _warn("Error connecting to parent app\n");
        goto err_destroy_host_exit;
    }

    host_blocking(parent, false);

    init_event();

    if(!init_video(&mzx_world.conf, CAPTION)) {
        _warn("error initializing video\n");
        goto err_destroy_host_exit;
    }
    //init_audio(&(mzx_world.conf));

    clear_screen(32, 7);
    default_palette();
    create_blank_world(&mzx_world);

    mzx_world.debugging = true;

    while(1)
    {
        int key;
        robot_editor_ext(&mzx_world, &mzx_world.global_robot, &line_number);
        if(!process_network_input(parent, &mzx_world, &mzx_world.global_robot))
            break;
        update_screen();
        update_event_status_delay();
        key = get_key(keycode_internal);
        if(key == IKEY_ESCAPE)
            break;
    }

    //warp_mouse(39, 12);
    //cursor_off();
    //default_scroll_values(&mzx_world);

    /*#ifdef CONFIG_HELPSYS
    help_open(&mzx_world, mzx_res_get_by_id(MZX_HELP_FIL));
    #endif*/

    /*strncpy(curr_file, mzx_world.conf.startup_file, MAX_PATH - 1);
    curr_file[MAX_PATH - 1] = '\0';
    strncpy(curr_sav, mzx_world.conf.default_save_name, MAX_PATH - 1);
    curr_sav[MAX_PATH - 1] = '\0';

    mzx_world.mzx_speed = mzx_world.conf.mzx_speed;
    mzx_world.default_speed = mzx_world.mzx_speed;*/

    // Run main game (mouse is hidden and palette is faded)
    //title_screen(&mzx_world);

    //vquick_fadeout();

    /*if(mzx_world.active)
    {
        clear_world(&mzx_world);
        clear_global_data(&mzx_world);
        }*/

#ifdef CONFIG_HELPSYS
    //help_close(&mzx_world);
#endif

    //quit_audio();

    err = 0;
err_destroy_host_exit:
    if(parent)
        debugger_host_send(STOP_PROCESS, -1);

    host_destroy(parent);
    //err_network_layer_exit:
    network_layer_exit(&mzx_world.conf);
    /*if(mzx_world.update_done)
      free(mzx_world.update_done);*/
    //free_extended_macros(&mzx_world);
err_free_res:
    mzx_res_free();
    platform_quit();
err_out:
    return err;
}
