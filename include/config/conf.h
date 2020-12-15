/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 2004 Robert James Kaes <rjkaes@users.sourceforge.net>
 * Copyright (C) 2009 Michael Adam <obnox@samba.org>
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

#ifndef TINYPROXY_CONF_H
#define TINYPROXY_CONF_H

#include "conf_acl.h"
#include "conf_anon.h"
#include "conf_log.h"
#include "misc/hashmap.h"
#include "misc/list.h"

typedef struct
{
  char *name;
  char *value;
} http_header_t;

// main configuration structure
struct config_s
{
  pconf_log_t log;

  // map of headers which should be let through when the anonymous feature is turned on
  pconf_anon_t anon;

  // acl rules
  pconf_acl_t acl;

  plist_t access_list;

  plist_t basicauth_list;
  char *config_file;
  unsigned int port;
  char *stathost;
  unsigned int quit; // boolean
  char *user;
  char *group;
  plist_t listen_addrs;
#ifdef FILTER_ENABLE
  char *filter;
  unsigned int filter_url;           // boolean
  unsigned int filter_extended;      // boolean
  unsigned int filter_casesensitive; // boolean
#endif                               // FILTER_ENABLE
#ifdef XTINYPROXY_ENABLE
  unsigned int add_xtinyproxy; // boolean
#endif
#ifdef REVERSE_SUPPORT
  struct reversepath *reversepath_list;
  unsigned int reverseonly;  // boolean
  unsigned int reversemagic; // boolean
  char *reversebaseurl;
#endif
#ifdef UPSTREAM_SUPPORT
  struct upstream *upstream_list;
#endif // UPSTREAM_SUPPORT
  char *pidpath;
  unsigned int idletimeout;
  char *bind_address;
  unsigned int bindsame;

  // the configured name to use in the HTTP "Via" header field
  char *via_proxy_name;

  unsigned int disable_viaheader; // boolean

  // error page support.  Map error numbers to file paths
  phashmap_t errorpages;

  // error page to be displayed if appropriate page cannot be located in the errorpages structure
  char *errorpage_undef;

  // the HTML statistics page
  char *statpage;

  // store the list of port allowed by CONNECT.
  plist_t connect_ports;

  // extra headers to be added to outgoing HTTP requests
  plist_t add_headers;
};

extern int try_load_config_file(const char *config_fname, struct config_s *conf,
                                struct config_s *defaults);

int config_compile_regex(void);

#endif // TINYPROXY_CONF_H
