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

/* Handles insertion and searches for headers which should be let through
 * when the anonymous feature is turned on.
 */

#include <stdbool.h>

#include "subservice/anonymous.h"

#include "misc/hashmap.h"
#include "self_contained/safecall.h"

#define BUCKETS_COUNT ((unsigned int)32)

struct anon_s
{
  phashmap_t headers;
  bool enabled;
};

CREATE_IMPL(panon_t, {
  obj->enabled = false;
  obj->headers = hashmap_create(BUCKETS_COUNT);
  if (NULL == obj->headers)
  {
    TRACE_MSG("%s", "cannot create hashmap");
    delete_panon_t(&obj);
    TRACE_NULL;
  }
})

DELETE_IMPL(panon_t, {
  obj->enabled = false;
  TRACE_SAFE(hashmap_delete(obj->headers));
  obj->headers = NULL;
})

int add_header_anon(panon_t anon, const char *header)
{
  TRACE_CALL_X(add_header_conf_anon, "anon = %p, header = %s", (void *)anon, header);

  const ssize_t rsearch = hashmap_search(anon->headers, header);
  if (rsearch > 0)
  {
    // the key was already found, so return a positive number
    TRACE_SUCCESS;
  }

  TRACE_SAFE_X(rsearch < 0, -1, "error when trying search key \"%s\" on map %p", header,
               (void *)anon->headers);

  // insert the new key
  char data = 1;
  int r = hashmap_insert(anon->headers, header, &data, sizeof(data));
  TRACE_SAFE_X(r, -1, "cannot insert key \"%s\"", header);

  TRACE_SUCCESS;
}

panon_t create_configured_anon(pconf_anon_t anon_config)
{
  TRACE_CALL_X(create_configured_anon, "anon_config = %p", (void *)anon_config);
  panon_t obj;
  TRACE_SAFE_R(NULL == (obj = create_panon_t()), NULL);

  if (anon_config->count == 0)
  {
    obj->enabled = false;
    TRACE_RETURN_X(obj, "%s", "anon headers disabled cause no anon headers configured");
  }

  obj->enabled = true;

  {
    // if ANONYMOUS is turned on, make sure that Content-Length is in the list of allowed headers,
    // since it is required in a HTTP/1.0 request
    //
    // also add the Content-Type header since it goes hand in hand with Content-Length
    char mandatory_headers[][20] = {"Content-Length", "Content-Type"};
    const size_t n = sizeof(mandatory_headers) / sizeof(mandatory_headers[0]);
    for (size_t i = 0; i < n; ++i)
    {
      TRACE_SAFE_FIN(add_header_anon(obj, mandatory_headers[i]), NULL, { delete_panon_t(&obj); });
    }
  }

  for (size_t i = 0; i < anon_config->count; ++i)
  {
    TRACE_SAFE_FIN(add_header_anon(obj, anon_config->headers[i]), NULL, { delete_panon_t(&obj); });
  }

  TRACE_RETURN(obj);
}

short int is_anonymous_enabled(panon_t anon)
{
  return anon->enabled;
}

/*
 * Search for the header.  This function returns a positive value greater than
 * zero if the string was found, zero if it wasn't and negative upon error.
 */
ssize_t anonymous_search(panon_t anon, const char *s)
{
  TRACE_CALL_X(anonymous_search, "anon = %p, s = %s", (void *)anon, s);
  TRACE_SAFE_X(NULL == anon, -1, "%s", "no anon struct to search header");
  return hashmap_search(anon->headers, s);
}
