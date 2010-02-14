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

/* Common declarations for host and remote debugging */

#ifndef __DEBUGGER_H
#define __DEBUGGER_H

#include "compat.h"

__M_BEGIN_DECLS

#ifdef CONFIG_DEBUGGER

#define DEBUGGER_PORT 2993
#define DEBUGGER_APP "./mzxdbg"
#define DEBUGGER_BYTECODE "_debugger.active.bc"

typedef int param_type;

struct debugger_message
{
  char type;
  param_type param;
};

enum message_type
{
  RELOAD_PROGRAM,    // param: bytecode program offset
  CURRENT_LINE,      // param: bytecode program offset
  STEP,              // param: none
  CONTINUE,          // param: none
  BREAK,             // param: none
  STOP_PROCESS,      // param: none
  TOGGLE_BREAKPOINT, // param: line number
  SWITCH_WATCH       // param: none
};

struct host;

void send_message(struct host *h, enum message_type type, param_type param);
void debugger_host_send(enum message_type type, param_type param);

#endif // CONFIG_DEBUGGER

__M_END_DECLS

#endif
