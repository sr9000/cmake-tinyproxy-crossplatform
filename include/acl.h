/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 2000 Robert James Kaes <rjkaes@users.sourceforge.net>
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

#ifndef TINYPROXY_ACL_H
#define TINYPROXY_ACL_H

#include "child.h"
#include "misc/list.h"

typedef enum
{
  ACL_ALLOW,
  ACL_DENY
} acl_access_t;

extern int insert_acl(char *location, acl_access_t access_type, plist_t *access_list);
extern int check_acl(pproxy_t proxy, const char *ip, const char *host, plist_t access_list);
extern void flush_access_list(plist_t access_list);

#endif // TINYPROXY_ACL_H
