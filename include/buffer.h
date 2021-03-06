/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 1999 Robert James Kaes <rjkaes@users.sourceforge.net>
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

#ifndef TINYPROXY_BUFFER_H
#define TINYPROXY_BUFFER_H

#include "tinyproxy.h"

#include <libwebsockets.h>

#include "subservice/network.h"

struct buffer_s;

extern struct buffer_s *new_buffer(void);
extern void delete_buffer(struct buffer_s *buffptr);
extern size_t buffer_size(struct buffer_s *buffptr);

// add a new line to the given buffer. The data IS copied into the structure
extern int add_to_buffer(struct buffer_s *buffptr, unsigned char *data, size_t length);

extern ssize_t read_buffer(pproxy_t proxy, int fd, struct buffer_s *buffptr);
extern ssize_t write_buffer(pproxy_t proxy, int fd, struct buffer_s *buffptr);
extern ssize_t write_websocket_buffer(pproxy_t proxy, struct lws *wsi, struct buffer_s *buffptr);
extern ssize_t read_ws_buffer(pproxy_t proxy, struct buffer_s *buffptr, unsigned char *data,
                              size_t len);
#endif // TINYPROXY_BUFFER_H
