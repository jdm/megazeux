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

/* Declarations for ui.c */

#ifndef __DEBUGGER_UI_H
#define __DEBUGGER_UI_H

#include "compat.h"

__M_BEGIN_DECLS

struct world;

#ifdef CONFIG_DEBUGGER
CORE_LIBSPEC void watch_remote_robot(struct world *mzx_world);
CORE_LIBSPEC bool watch_robot(struct world *mzx_world);
#endif

__M_END_DECLS

#endif
