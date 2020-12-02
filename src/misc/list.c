/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 2002 Robert James Kaes <rjkaes@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* A list implementation.  The list can be of an arbitrary length, and
 * the data for each entry is an lump of data (the size is stored in the
 * list.)
 */

#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "misc/heap.h"
#include "misc/list.h"

// These structures are the storage for the "list". Entries are stored in struct listentry_s
// (the data and the length), and the "list" structure is implemented as a linked-list. The
// struct list_s stores a pointer to the first list (list[0]) and a count of the number of
// entries (or how long the list is.)
struct listentry_s
{
  void *data;
  size_t len;

  struct listentry_s *next;
};

struct list_s
{
  size_t num_entries;
  struct listentry_s *head;
  struct listentry_s *tail;
};

// Create an list. The list initially has no elements and no storage has been allocated for the
// entries.
// Always use list_delete() to free memory after usage
//
// A NULL is returned if memory could not be allocated for the list.
plist_t list_create(void)
{
  plist_t list;

  list = (plist_t)safemalloc(sizeof(struct list_s));
  if (!list)
    return NULL;

  list->num_entries = 0;
  list->head = list->tail = NULL;

  return list;
}

// Deletes an list. All the entries when this function is run.
//
// Returns: 0 on success
//          negative if a NULL list is supplied
int list_delete(plist_t list)
{
  struct listentry_s *ptr, *next;

  if (!list)
    return -EINVAL;

  ptr = list->head;
  while (ptr)
  {
    next = ptr->next;
    safefree(ptr->data);
    safefree(ptr);

    ptr = next;
  }

  safefree(list);

  return 0;
}

typedef enum
{
  INSERT_PREPEND,
  INSERT_APPEND
} list_pos_t;

// Appends an entry into the list. The entry is an arbitrary collection of bytes of _len_ octets.
// The data is copied into the list, so the original data must be freed to avoid a memory leak.
// The "data" must be non-NULL and the "len" must be greater than zero. "pos" is either 0 to prepend
// the data, or 1 to append the data.
//
// Returns: 0 on success
//          negative number if there are errors
static int list_insert(plist_t list, void *data, size_t len, list_pos_t pos)
{
  struct listentry_s *entry;

  if (!list || !data || len <= 0 || (pos != INSERT_PREPEND && pos != INSERT_APPEND))
    return -EINVAL;

  entry = (struct listentry_s *)safemalloc(sizeof(struct listentry_s));
  if (!entry)
    return -ENOMEM;

  entry->data = safemalloc(len);
  if (!entry->data)
  {
    safefree(entry);
    return -ENOMEM;
  }

  memcpy(entry->data, data, len);
  entry->len = len;
  entry->next = NULL;

  // If there is no head or tail, create them
  if (!list->head && !list->tail)
  {
    list->head = list->tail = entry;
  }
  else if (pos == INSERT_PREPEND)
  {
    // prepend the entry
    entry->next = list->head;
    list->head = entry;
  }
  else
  {
    // append the entry
    list->tail->next = entry;
    list->tail = entry;
  }

  list->num_entries++;

  return 0;
}

// The following two function are used to make the API clearer. As you can see they simply call the
// list_insert() function with appropriate arguments.
int list_append(plist_t list, void *data, size_t len)
{
  return list_insert(list, data, len, INSERT_APPEND);
}

int list_prepend(plist_t list, void *data, size_t len)
{
  return list_insert(list, data, len, INSERT_PREPEND);
}

// A pointer to the data at position "pos" (zero based) is returned. If the list is out of bound,
// data is set to NULL.
//
// Returns: negative upon an error
//          length of data if position is valid
void *list_getentry(plist_t list, size_t pos, size_t *psize)
{
  struct listentry_s *ptr;
  size_t loc;

  if (!list || pos >= list->num_entries)
    return NULL;

  loc = 0;
  ptr = list->head;

  while (loc != pos)
  {
    ptr = ptr->next;
    loc++;
  }

  if (psize)
    *psize = ptr->len;

  return ptr->data;
}

// Returns the number of entries (or the length) of the list.
//
// Returns: negative if list is not valid
//          positive length of list otherwise
ssize_t list_length(plist_t list)
{
  if (!list)
    return -EINVAL;

  return list->num_entries;
}
