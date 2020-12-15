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

#include "config/conf_acl.h"
#include "log.h"
#include "self_contained/object.h"
#include "subservice/acl_access_type.h"

typedef struct acl_s *pacl_t;

CREATE_DECL(pacl_t);
DELETE_DECL(pacl_t);

extern pacl_t create_configured_acl(pconf_acl_t acl_config);

extern int check_acl(plog_t plog, pacl_t acl, const char *ip, const char *host);
extern void flush_access_list(plist_t access_list);

#endif // TINYPROXY_ACL_H
