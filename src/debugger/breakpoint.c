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

#include <stdlib.h>
#include "../world_struct.h"
#include "../robot_struct.h"
#include "breakpoint.h"

void toggle_breakpoint(struct world *mzx_world, struct robot *cur_robot, int pos)
{
  struct breakpoint *bp = &mzx_world->debug_watch.breakpoints;
  struct breakpoint *last = NULL;
  while(bp)
  {
    if(bp->target == cur_robot && bp->pos == pos)
    {
      // |last| is guaranteed non-null as the first breakpoint is a guard value
      last->next = bp->next;
      free(bp);
      break;
    }
    
    last = bp;
    bp = bp->next;
  }
  if(!bp)
  {
    last->next = malloc(sizeof(struct breakpoint));
    last->next->target = cur_robot;
    last->next->pos = pos;
    last->next->next = NULL;
  }
}

void clear_breakpoints(struct world *mzx_world)
{
  struct breakpoint *bp = mzx_world->debug_watch.breakpoints.next;
  while(bp)
  {
    struct breakpoint *temp = bp->next;
    free(bp);
    bp = temp;
  }
  mzx_world->debug_watch.breakpoints.next = NULL;
}
