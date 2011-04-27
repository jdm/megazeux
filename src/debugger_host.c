/* MegaZeux
 *
 * Copyright (C) 2010 Josh Matthews <josh@joshmatthews.net>
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

// Debugger host code

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef __WIN32__
#include <windows.h>
#else
#include <sys/wait.h>
#endif

#include "world_struct.h"
#include "debugger/debugger.h"
#include "debugger_host.h"
#include "network/host.h"
#include "util.h"
#include "data.h"
#include "debugger/ui.h"
#include "debugger/breakpoint.h"

struct host *parent, *child;

static bool debugger_launch(void);
static bool launch_app(char *args[]);

static int line_number_to_offset(struct robot *robot, int line_num)
{
  char *program = robot->program_bytecode;
  int cur_offset = 1;
  int current_line_num = 1;
  while(current_line_num < line_num)
  {
    current_line_num++;
    cur_offset += program[cur_offset] + 2;
  }
  return cur_offset;
}

static int get_message_arg(char *message, int arg)
{
    int val;
    memcpy(&val, message + 1 + sizeof(val) * arg, sizeof(val));
    return val;
}

static bool process_message(char *message, struct world *mzx_world)
{
  switch(message[0])
  {
    case STEP:
      if(mzx_world->debugging == STOPPED)
      {
        info("instructed to step\n");
        if(mzx_world->debug_watch.commands_executed > -1)
          mzx_world->debugging = STEPPING;
        else
          mzx_world->debugging = STEPPING_OTHERS;
      }
      break;
    case CONTINUE:
      info("instructed to continue\n");
      mzx_world->debugging = RUNNING;
      break;
    case BREAK:
      info("instructed to break\n");
      mzx_world->debugging = STOPPED;
      break;
    case TOGGLE_BREAKPOINT:
    {
      struct robot *watch = mzx_world->current_board->robot_list[mzx_world->debug_watch.watch_id];
      int line = get_message_arg(message, 0);
      int offset = line_number_to_offset(watch, line);
      info("instructed to toggle breakpoint at line %d (offset %d)\n", line, offset);
      toggle_breakpoint(mzx_world, watch, offset);
      break;
    }
    case SWITCH_WATCH:
      info("instructed to switch watches\n");
      watch_robot(mzx_world);
      break;
    case STOP_PROCESS:
      info("instructed to stop\n");
      return false;

    case RELOAD_PROGRAM:
    case CURRENT_LINE:
      warn("parent->child message received in parent\n");
      return false;

    default:
      warn("unknown message (%d) received from debugger\n", message[0]);
      return false;
  }
  return true;
}

void debugger_send(enum message_type type, ...)
{
    va_list args;
    va_start(args, type);
  if(child)
    send_message(child, type, args);
  else
    warn("Tried to send message (%d) to non-existent child\n", type);
  va_end(args);
}

bool debugger_start(struct world *mzx_world)
{
  child = NULL;

  if(!host_layer_init(&mzx_world->conf))
  {
    warn("Error initializing socket layer\n");
    return false;
  }

  parent = host_create(HOST_TYPE_TCP, HOST_FAMILY_IPV4);
  if(!parent)
  {
    warn("Error creating host\n");
    goto exit_socket_layer;
  }

  host_blocking(parent, false);

  if(!host_bind(parent, "localhost", DEBUGGER_PORT))
  {
    warn("Failed to bind parent\n");
    goto exit_host_destroy;
  }

  if(!host_listen(parent))
  {
    warn("Failed to listen with parent\n");
    goto exit_host_destroy;
  }

  if(!debugger_launch())
    goto exit_host_destroy;

  return true;

exit_host_destroy:
  host_destroy(parent);
exit_socket_layer:
  host_layer_exit();

  return false;
}

bool debugger_run(struct world *mzx_world)
{
  int status;

  if(!child)
  {
    child = host_accept(parent);
    if(child)
    {
      host_blocking(child, false);
      watch_remote_robot(mzx_world);
    }
    else
      return true;
  }

  status = host_poll_raw(child, 10);
  if(status < 0)
  {
    warn("Debugger process communication error\n");
    return false;
  }
  else if(status > 0)
  {
    char type, *buffer;
    bool rv;
    if (!host_recv_raw(child, &type, 1) ||
        !(buffer = malloc(message_size((enum message_type)type))) ||
        !host_recv_raw(child, buffer + 1, message_size((enum message_type)type) - 1))
    {
      warn("Receive error\n");
      return false;
    }
    buffer[0] = type;
    rv = process_message(buffer, mzx_world);
    free(buffer);
    return rv;
  }
  return true;
}

void debugger_end(struct world *mzx_world)
{
  if(child)
  {
    debugger_send(STOP_PROCESS, -1);
    host_destroy(child);
    child = NULL;
  }
  host_destroy(parent);
  parent = NULL;
  host_layer_exit();

  clear_breakpoints(mzx_world);
  mzx_world->debugging = NOT_DEBUGGING;
}

static bool debugger_launch()
{
  bool ret;
  char *args[] = {(char*)DEBUGGER_APP, current_dir, NULL};
  getcwd(current_dir, MAX_PATH);    
  chdir(config_dir);
  ret = launch_app(args);
  chdir(current_dir);
  return ret;
}

#ifdef __WIN32__
static bool launch_app(char *args[])
{
  STARTUPINFO startup_info;
  PROCESS_INFORMATION process_info;
  char *cmdline;
  int i, size = 0;

  memset(&startup_info, 0, sizeof(startup_info));
  memset(&process_info, 0, sizeof(process_info));
  
  for(i = 0; args[i]; i++)
    size += strlen(args[i]) + 1;
  cmdline = calloc(size, 1);
  for(i = 0; args[i]; i++)
  {
    strcat(cmdline, args[i]);
    if(args[i + 1])
      strcat(cmdline, " ");
  }
  
  startup_info.cb = sizeof(startup_info);
  startup_info.dwFlags = 0;
  if (!CreateProcess(NULL,
                     cmdline, NULL, NULL,
                     FALSE, 0, NULL, NULL,
                     &startup_info, &process_info))
  {
    warn("Error launching debugger process\n");
    return false;
  }

  free(cmdline);
  
  // Handles must be closed or they will leak
  CloseHandle(process_info.hThread);
  
  info("child process: %d\n", process_info.hProcess);
  CloseHandle(process_info.hProcess);
  return true;
}
#else // !__WIN32__
static bool launch_app(char *args[])
{
  pid_t pid = fork();
  if(pid < 0) {
    warn("Fork error\n");
    return false;
  }
  
  if(!pid) // Child process
  {
    execvp(DEBUGGER_APP, args);
    warn("Error launching debugger process\n");
    _exit(127);
  }
  else info("child process: %d\n", pid);
  
  return true;
}
#endif
