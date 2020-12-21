/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 1999 George Talusan <gstalusan@uwaterloo.ca>
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

#ifndef TINYPROXY_FILTER_H
#define TINYPROXY_FILTER_H

#include <stdbool.h>

#include "config/conf_filt.h"
#include "self_contained/object.h"
#include "subservice/log.h"

typedef struct filter_s *pfilter_t;

CREATE_DECL(pfilter_t);
DELETE_DECL(pfilter_t);

extern pfilter_t create_configured_filter(pconf_filt_t filt_config);

extern bool is_enabled(pfilter_t filter);
extern int activate_filtering(plog_t log, pfilter_t filter);

// return true to allow, false to block
extern bool does_pass_filter(plog_t log, pfilter_t filter, const char *host, const char *url);

#endif // TINYPROXY_FILTER_H
