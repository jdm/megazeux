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

#ifndef __DEBUGGER_BREAKPOINT_H
#define __DEBUGGER_BREAKPOINT_H

#include "../compat.h"

__M_BEGIN_DECLS

struct robot;
struct counter;

typedef void (*enumerate_func)(void *elem, void *data);

struct breakpoint
{
  struct robot *target;
  int pos;
};

struct watchpoint
{
  struct counter *target;
};

void init_breakpoints(struct world *mzx_world);
void toggle_breakpoint(struct world *mzx_world, struct robot *cur_robot, int pos);
void clear_breakpoints(struct world *mzx_world);
bool breakpoint_exists(struct world *mzx_world, struct robot *cur_robot, int pos);
void foreach_breakpoint(struct world *mzx_world, struct robot *cur_robot, enumerate_func func);

void init_watchpoints(struct world *mzx_world);
void toggle_watchpoint(struct world *mzx_world, struct counter *counter);
void clear_watchpoints(struct world *mzx_world);
bool watchpoint_exists(struct world *mzx_world, struct counter *counter);
void foreach_watchpoint(struct world *mzx_world, struct counter *counter, enumerate_func func);

__M_END_DECLS

#endif // __DEBUGGER_BREAKPOINT_H
