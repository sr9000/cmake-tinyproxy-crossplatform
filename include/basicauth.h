/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 2005 Robert James Kaes <rjkaes@users.sourceforge.net>
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

#ifndef TINYPROXY_BASICAUTH_H
#define TINYPROXY_BASICAUTH_H

#include <stddef.h>

#include "misc/list.h"

extern ssize_t basicauth_string(const char *user, const char *pass, char *buf, size_t bufsize);
extern void basicauth_add(plist_t authlist, const char *user, const char *pass);
extern int basicauth_check(plist_t authlist, const char *authstring);

#endif // TINYPROXY_BASICAUTH_H
