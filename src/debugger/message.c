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

// Debugger message code

#include <stdarg.h>
#include <stdlib.h>
#include "debugger.h"
#include "debugger/debugger_child.h"
#include "network/host.h"
#include "util.h"

static unsigned int message_args(enum message_type type)
{
  switch (type)
  {
    case STEP:
    case CONTINUE:
    case BREAK:
    case STOP_PROCESS:
    case SWITCH_WATCH:
      return 0;
    case RELOAD_PROGRAM:
    case CURRENT_LINE:
    case TOGGLE_BREAKPOINT:
      return 1;
    case UPDATE_COORDS:
      return 2;
    default:
      return -1;
  }
}

unsigned int message_size(enum message_type type)
{
  return 1 + message_args(type) * sizeof(int);
}

void send_message(struct host *h, enum message_type type, va_list args)
{
  char *message;
  unsigned int i, size;
  int arg;
  size = message_size(type);
  message = malloc(size);
  if (!message)
    warn("Error allocating buffer\n");
  message[0] = (char)type;
  i = 0;
  while (i < message_args(type)) {
    arg = va_arg(args, int);
    memcpy(message + 1 + sizeof(int) * i, &arg, sizeof(arg));
    i++;
  }
  if (!host_send_raw(h, message, size))
    warn("Error sending message (%d)\n", type);
  free(message);
}

void debugger_host_send(enum message_type type, ...)
{
  va_list args;
  va_start(args, type);
  send_message(parent, type, args);
  va_end(args);
}
