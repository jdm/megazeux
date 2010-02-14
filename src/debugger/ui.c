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

#include "ui.h"
#include "debugger.h"
#include "../debugger_host.h"
#include "../world_struct.h"
#include "../robot_struct.h"
#include "../window.h"
#include "../graphics.h"
#include "../fsafeopen.h"

#include <string.h>

#ifdef CONFIG_DEBUGGER

void watch_remote_robot(struct world *mzx_world)
{
  struct breakpoint *bp = &mzx_world->debug_watch.breakpoints;
  struct robot *cur_robot = mzx_world->debug_watch.watch;
  debugger_send(RELOAD_PROGRAM, cur_robot->cur_prog_line);
  while(bp)
  {
    if(bp->target == cur_robot)
      debugger_send(TOGGLE_BREAKPOINT, bp->pos);
    bp = bp->next;
  }
}

bool watch_robot(struct world *mzx_world)
{
  int dialog_result;
  int num_robots = mzx_world->current_board->num_robots + 1;
  char **robot_list = calloc(num_robots, sizeof(char *));
  int i;
  int cp_len;
  int selected = 0;
  struct dialog di;
  struct element *elements[1];
  bool robot_selected = false;
  
  m_show();
  
  for(i = 0; i < num_robots; i++)
  {
    robot_list[i] = calloc(16, 1);
    cp_len = strlen(mzx_world->current_board->robot_list[i]->robot_name);
    if(mzx_world->debug_watch.watch)
    {
      if(!strcmp(mzx_world->current_board->robot_list[i]->robot_name,
                 mzx_world->debug_watch.watch->robot_name))
        selected = i;
    }
    memset(robot_list[i], ' ', 15);
    memcpy(robot_list[i], mzx_world->current_board->robot_list[i]->robot_name, cp_len);
  }
  
  do
  {
    elements[0] = construct_list_box(2, 2, (const char **)robot_list,
                                   num_robots, 9, 15, 0, &selected, false);

    construct_dialog_ext(&di, "Watch Robot", 30, 7,
                         19, 13, elements, 1, 0, 0, 0, NULL);

    dialog_result = run_dialog(mzx_world, &di);
    if(dialog_result == 0)
    {
      FILE *bc_file;
      struct robot *cur_robot = mzx_world->current_board->robot_list[selected];
      mzx_world->debug_watch.watch = cur_robot;
            
      bc_file = fsafeopen(DEBUGGER_BYTECODE, "wb");
      if(bc_file)
      {
        fwrite(cur_robot->program_bytecode, cur_robot->program_bytecode_length, 1, bc_file);
        fclose(bc_file);
      }
      watch_remote_robot(mzx_world);

      dialog_result = -1;
      robot_selected = true;
    }
    destruct_dialog(&di);
  } while(dialog_result != -1);

  m_hide();

  for(i = 0;i < num_robots; i++)
    free(robot_list[i]);

  free(robot_list);

  return robot_selected;
}

#endif // CONFIG_DEBUGGER
