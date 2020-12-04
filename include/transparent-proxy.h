/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 2008 Robert James Kaes <rjk@wormbytes.ca>
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

#ifndef TINYPROXY_TRANSPARENT_PROXY_H
#define TINYPROXY_TRANSPARENT_PROXY_H
#ifdef TRANSPARENT_PROXY

#include "common.h"

#include "conns.h"
#include "misc/hashmap.h"
#include "reqs.h"

extern int do_transparent_proxy(pproxy_t proxy, struct conn_s *connptr, phashmap_t hashofheaders,
                                struct request_s *request, struct config_s *conf, char **url);

#endif // TRANSPARENT_PROXY
#endif // TINYPROXY_TRANSPARENT_PROXY_H
