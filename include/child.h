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

#ifndef TINYPROXY_CHILD_H
#define TINYPROXY_CHILD_H

#include <inttypes.h>

#include "misc/list.h"
#include "tinyproxy.h"

typedef enum
{
  CHILD_MAXCLIENTS,
  CHILD_MAXSPARESERVERS,
  CHILD_MINSPARESERVERS,
  CHILD_STARTSERVERS,
  CHILD_MAXREQUESTSPERCHILD
} child_config_t;

extern short int child_pool_create(pproxy_t proxy);
extern int child_listening_sockets(pproxy_t proxy, plist_t listen_addrs, uint16_t port);
extern void child_close_sock(void);
extern void child_main_loop(pproxy_t proxy);
extern void child_kill_children(pproxy_t proxy, int sig);

extern short int child_configure(child_config_t type, unsigned int val);

#endif // TINYPROXY_CHILD_H
