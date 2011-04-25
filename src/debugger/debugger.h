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
#include <stdarg.h>

__M_BEGIN_DECLS

#ifdef CONFIG_DEBUGGER

#define DEBUGGER_PORT 2993
#define DEBUGGER_APP "./mzxdbg"
#define DEBUGGER_BYTECODE "_debugger.active.bc"

enum message_type
{
  RELOAD_PROGRAM,    // offset
  CURRENT_LINE,      // offset
  STEP,              //
  CONTINUE,          //
  BREAK,             //
  STOP_PROCESS,      //
  TOGGLE_BREAKPOINT, // line
  SWITCH_WATCH,      //
  UPDATE_COORDS      // x, y
};

struct host;

void send_message(struct host *h, enum message_type type, va_list args);
void debugger_host_send(enum message_type type, ...);
unsigned int message_size(enum message_type);

#endif // CONFIG_DEBUGGER

__M_END_DECLS

#endif
