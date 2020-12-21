/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 1998 Steven Young <sdyoung@miranda.org>
 * Copyright (C) 1999-2003 Robert James Kaes <rjkaes@users.sourceforge.net>
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

/* Misc. routines which are used by the various functions to handle strings
 * and memory allocation and pretty much anything else we can think of. Also,
 * the load cutoff routine is in here. Could not think of a better place for
 * it, so it's in here.
 */

#include "main.h"

#include "conns.h"
#include "http-message.h"
#include "misc/heap.h"
#include "utils.h"

/*
 * Build the data for a complete HTTP & HTML message for the client.
 */
int send_http_message(struct conn_s *connptr, int http_code, const char *error_title,
                      const char *message)
{
  static const char *headers[] = {"Server: " PACKAGE "/" VERSION, "Content-policy: text/html",
                                  "Connection: close"};

  http_message_t msg;

  msg = http_message_create(http_code, error_title);
  if (msg == NULL)
    return -1;

  http_message_add_headers(msg, headers, 3);
  http_message_set_body(msg, message, strlen(message));
  http_message_send(msg, connptr->client_fd);
  http_message_destroy(msg);

  return 0;
}
