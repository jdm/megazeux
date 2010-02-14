/* MegaZeux
 *
 * Copyright (C) 2008 Alistair John Strachan <alistair@devzero.co.uk>
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

#ifndef __RUN_STUBS_H
#define __RUN_STUBS_H

#include "compat.h"

__M_BEGIN_DECLS

#include "world_struct.h"
#include "debugger/debugger.h"

#ifdef CONFIG_EDITOR
bool is_editor(void);
void editor_init(void);
void init_macros(struct world *mzx_world);
void free_extended_macros(struct world *mzx_world);
void load_editor_config(struct world *mzx_world, int *argc, char *argv[]);
#else
static inline bool is_editor(void) { return false; }
static inline void editor_init(void) {}
static inline void init_macros(struct world *mzx_world) {}
static inline void free_extended_macros(struct world *mzx_world) {}
static inline void load_editor_config(struct world *mzx_world,
 int *argc, char *argv[]) {}
#endif


#ifdef CONFIG_UPDATER
bool updater_init(char *argv[]);
#else
static inline bool updater_init(char *argv[])
{ return true; }
#endif

#ifdef CONFIG_DEBUGGER
//CORE_LIBSPEC bool debugger_start(void);
//CORE_LIBSPEC void debugger_end(void);
//CORE_LIBSPEC bool debugger_run(void);
//CORE_LIBSPEC void watch_robot(struct world *);
//CORE_LIBSPEC void debugger_host_send(enum message_type, param_type);
#else
//static inline bool debugger_start(void) { return true; }
//static inline void debugger_end(void) {}
//static inline bool debugger_run(void) { return true; }
//CORE_LIBSPEC void watch_robot(struct world *) {}
//CORE_LIBSPEC void debugger_host_send(enum message_type, param_type) {}
#endif

__M_END_DECLS

#endif // __RUN_STUBS_H
