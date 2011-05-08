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
#include "breakpoint.h"

typedef bool (*elem_equal_op)(void *elem, void *data);
typedef void *(*elem_create_op)(void *data);

struct list_ops
{
  elem_create_op create;
  elem_equal_op equal;
};

struct list_node
{
  struct list_ops *ops;
  void *elem;
  struct list_node *next;
};

static void list_init(struct list_node **node, struct list_ops *ops)
{
  *node = malloc(sizeof(**node));
  (*node)->next = NULL;
  (*node)->ops = ops;
  (*node)->elem = NULL;
}

static void list_add_or_remove(struct list_node *node, void *data)
{
  struct list_node *last = NULL;
  while (node) {
    if (node->elem && node->ops->equal(node->elem, data)) {
      last->next = node->next;
      free(node);
      return;
    }

    last = node;
    node = node->next;
  }
  if (!node) {
    last->next = malloc(sizeof(*last->next));
    last->next->ops = last->ops;
    last->next->elem = last->ops->create(data);
    last->next->next = NULL;
  }
}

static void list_clear(struct list_node *node)
{
  struct list_node *orig = node;
  node = node->next;
  while (node) {
    struct list_node *temp = node->next;
    free(node->elem);
    free(node);
    node = temp;
  }
  orig->next = NULL;
}

static bool list_elem_exists(struct list_node *node, void *data)
{
  while (node) {
    if (node->elem && node->ops->equal(node->elem, data))
      return true;
    node = node->next;
  }
  return false;
}

static void list_enumerate(struct list_node *node, enumerate_func func, void *data)
{
  while (node) {
    if (node->elem)
      func(node->elem, data);
    node = node->next;
  }
}

#define CREATE_NAME(type) create_##type

#define DEFINE_CREATE(type)                  \
  static void *CREATE_NAME(type)(void *data) \
  {                                          \
    struct type *p = malloc(sizeof(*p));     \
    memcpy(p, data, sizeof(*p));             \
    return p;                                \
  }

#define EQUALS_NAME(type) type##_equal

#define DEFINE_EQUALS(type)                             \
  static bool EQUALS_NAME(type)(void *elem, void *data) \
  {                                                     \
    return !memcmp(elem, data, sizeof(struct type));    \
  }

#define DEFINE_OPS(type) \
  DEFINE_CREATE(type) DEFINE_EQUALS(type)   \
  static struct list_ops type##_ops = {     \
    &CREATE_NAME(type), &EQUALS_NAME(type), \
  };

DEFINE_OPS(breakpoint)
DEFINE_OPS(watchpoint)

void toggle_breakpoint(struct world *mzx_world, struct robot *cur_robot, int pos)
{
  struct breakpoint bp = { cur_robot, pos };
  list_add_or_remove(mzx_world->debug_watch.breakpoints, &bp);
}

void clear_breakpoints(struct world *mzx_world)
{
  list_clear(mzx_world->debug_watch.breakpoints);
}

void init_breakpoints(struct world *mzx_world)
{
  list_init(&mzx_world->debug_watch.breakpoints, &breakpoint_ops);
}

bool breakpoint_exists(struct world *mzx_world,
                       struct robot *cur_robot, int pos)
{
  struct breakpoint bp = { cur_robot, pos };
  return list_elem_exists(mzx_world->debug_watch.breakpoints, &bp);
}

void foreach_breakpoint(struct world *mzx_world, struct robot *cur_robot, enumerate_func func)
{
  struct breakpoint bp = { cur_robot, 0 };
  list_enumerate(mzx_world->debug_watch.breakpoints, func, &bp);
}

void toggle_watchpoint(struct world *mzx_world, struct counter *counter)
{
  struct watchpoint wp = { counter };
  list_add_or_remove(mzx_world->debug_watch.watchpoints, &wp);
}

void clear_watchpoints(struct world *mzx_world)
{
  list_clear(mzx_world->debug_watch.watchpoints);
}

void init_watchpoints(struct world *mzx_world)
{
  list_init(&mzx_world->debug_watch.watchpoints, &watchpoint_ops);
}

bool watchpoint_exists(struct world *mzx_world,
                       struct counter *counter)
{
  struct watchpoint wp = { counter };
  return list_elem_exists(mzx_world->debug_watch.watchpoints, &wp);
}

void foreach_watchpoint(struct world *mzx_world, struct counter *counter, enumerate_func func)
{
  struct watchpoint wp = { counter };
  list_enumerate(mzx_world->debug_watch.watchpoints, func, &wp);
}
