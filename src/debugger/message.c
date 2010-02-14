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

#include "debugger.h"
#include "debugger/debugger_child.h"
#include "network/host.h"
#include "util.h"

void send_message(struct host *h, enum message_type type, param_type param)
{
  struct debugger_message message = {
    .type = (char)type,
    .param = param
  };

  if(!host_send_raw(h, (const char *)&message, sizeof(message)))
    warn("Error sending message (%d,%d)\n", type, param);
}

void debugger_host_send(enum message_type type, param_type param)
{
  send_message(parent, type, param);
}
