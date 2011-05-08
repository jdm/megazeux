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

/* Declarations for debugger_host.c */

#ifndef __DEBUGGER_HOST_H
#define __DEBUGGER_HOST_H

#include "compat.h"
#include "debugger/debugger.h"

__M_BEGIN_DECLS

CORE_LIBSPEC bool debugger_start(struct world *mzx_world);
CORE_LIBSPEC void debugger_end(struct world *mzx_world);
CORE_LIBSPEC bool debugger_run(struct world *mzx_world);
CORE_LIBSPEC void debugger_send(enum message_type type, ...);
CORE_LIBSPEC void debugger_watch(struct world *mzx_world, int id);

__M_END_DECLS

#endif
