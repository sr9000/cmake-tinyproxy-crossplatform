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

#ifndef TINYPROXY_ANONYMOUS_H
#define TINYPROXY_ANONYMOUS_H

#include "self_contained/object.h"
#include "config/conf_anon.h"

typedef struct anon_s *panon_t;

CREATE_DECL(panon_t);
DELETE_DECL(panon_t);

extern panon_t create_configured_anon(pconf_anon_t anon_config);

extern short int is_anonymous_enabled(panon_t anon);
extern ssize_t anonymous_search(panon_t anon, const char *s);

#endif // TINYPROXY_ANONYMOUS_H
