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

#ifndef TINYPROXY_LIST_H
#define TINYPROXY_LIST_H

#include <stddef.h>
#include <sys/types.h>

// We're using a typedef here to "hide" the implementation details of the list. Sure, it's a
// pointer, but the struct is hidden in the C file. So, just use the plist_t like it's a cookie.
typedef struct list_s *plist_t;

// Create an list. The list initially has no elements and no storage has been allocated for the
// entries.
// Always use list_delete() to free memory after usage
//
// A NULL is returned if memory could not be allocated for the list.
extern plist_t list_create(void);

// Deletes an list.  All the entries when this function is run.
//
// Returns: 0 on success
//          negative if a NULL list is supplied
extern int list_delete(plist_t list);

// When you insert a piece of data into the list, the data will be duplicated, so you must free
// your copy if it was created on the heap. The data must be non-NULL and the length must be greater
// than zero.
//
// Returns: negative on error
//          0 upon successful insert.
extern int list_append(plist_t list, void *data, size_t len);
extern int list_prepend(plist_t list, void *data, size_t len);

// A pointer to the data at position "pos" (zero based) is returned and the size pointer contains
// the length of the data stored.
//
// The pointer points to the actual data in the list, so you have the power to modify the data,
// but do it responsibly since the library doesn't take any steps to prevent you from messing up the
// list.  (A better prule is, don't modify the data since you'll likely mess up the "length"
// parameter of the data.)  However, DON'T try to realloc or free the data; doing so will break the
// list.
//
// If pointer "psize" is NULL the size of the data is not returned.
//
// Returns: NULL on error
//          valid pointer to data
extern void *list_getentry(plist_t list, size_t pos, size_t *psize);

// Returns the number of enteries (or the length) of the list.
//
// Returns: negative if list is not valid
//          positive length of list otherwise
extern ssize_t list_length(plist_t list);

#endif // TINYPROXY_LIST_H
