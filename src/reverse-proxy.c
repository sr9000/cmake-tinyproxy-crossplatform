/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 1999-2005 Robert James Kaes <rjkaes@users.sourceforge.net>
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

/* Allow tinyproxy to be used as a reverse proxy. */

#include "main.h"

#include "reverse-proxy.h"

#include "config/conf.h"
#include "conns.h"
#include "html-error.h"
#include "misc/heap.h"
#include "self_contained/debugtrace.h"
#include "subservice/log.h"

/*
 * Add entry to the reversepath list
 */
int reversepath_add(const char *path, const char *url, struct reversepath **reversepath_list)
{
  TRACE_CALL_X(reversepath_add, "path = %s, url = %s, &reversepath = %p", path, url,
              (void *)reversepath_list);

  struct reversepath *reverse;

  if (url == NULL)
  {
    TRACE_RETURN_X(-1, "%s", "Illegal reverse proxy rule: missing url");
  }

  if (!strstr(url, "://"))
  {
    TRACE_RETURN_X(-1, "Skipping reverse proxy rule: '%s' is not a valid url", url);
  }

  if (path && *path != '/')
  {
    TRACE_RETURN_X(-1,
                  "Skipping reverse proxy rule: path '%s' "
                  "doesn't start with a /",
                  path);
  }

  reverse = (struct reversepath *)safemalloc(sizeof(struct reversepath));
  if (!reverse)
  {
    TRACE_RETURN_X(-1, "%s", "Unable to allocate memory in reversepath_add()");
  }

  if (!path)
  {
    reverse->path = safestrdup("/");
  }
  else
  {
    reverse->path = safestrdup(path);
  }

  reverse->url = safestrdup(url);

  reverse->next = *reversepath_list;
  *reversepath_list = reverse;

  TRACE_RETURN(0);
}

/*
 * Check if a request url is in the reversepath list
 */
struct reversepath *reversepath_get(char *url, struct reversepath *reverse)
{
  while (reverse)
  {
    if (strstr(url, reverse->path) == url)
      return reverse;

    reverse = reverse->next;
  }

  return NULL;
}

/**
 * Free a reversepath list
 */

void free_reversepath_list(struct reversepath *reverse)
{
  while (reverse)
  {
    struct reversepath *tmp = reverse;
    reverse = reverse->next;
    safefree(tmp->url);
    safefree(tmp->path);
    safefree(tmp);
  }
}

/*
 * Rewrite the URL for reverse proxying.
 */
char *reverse_rewrite_url(pproxy_t proxy, struct conn_s *connptr, phashmap_t hashofheaders,
                          char *url)
{
  char *rewrite_url = NULL;
  char *cookie = NULL;
  char *cookieval;
  struct reversepath *reverse = NULL;

  /* Reverse requests always start with a slash */
  if (*url == '/')
  {
    /* First try locating the reverse mapping by request url */
    reverse = reversepath_get(url, config.reversepath_list);
    if (reverse)
    {
      rewrite_url = (char *)safemalloc(strlen(url) + strlen(reverse->url) + 1);
      strcpy(rewrite_url, reverse->url);
      strcat(rewrite_url, url + strlen(reverse->path));
    }
    else if (config.reversemagic &&
             hashmap_entry_by_key(hashofheaders, "cookie", (void **)&cookie) > 0)
    {

      /* No match - try the magical tracking cookie next */
      if ((cookieval = strstr(cookie, REVERSE_COOKIE "=")) &&
          (reverse =
               reversepath_get(cookieval + strlen(REVERSE_COOKIE) + 1, config.reversepath_list)))
      {

        rewrite_url = (char *)safemalloc(strlen(url) + strlen(reverse->url) + 1);
        strcpy(rewrite_url, reverse->url);
        strcat(rewrite_url, url + 1);

        log_message(proxy->log, LOG_INFO, "Magical tracking cookie says: %s", reverse->path);
      }
    }
  }

  if (rewrite_url == NULL)
  {
    return NULL;
  }

  log_message(proxy->log, LOG_CONN, "Rewriting URL: %s -> %s", url, rewrite_url);

  /* Store reverse path so that the magical tracking cookie can be set */
  if (config.reversemagic && reverse)
    connptr->reversepath = safestrdup(reverse->path);

  return rewrite_url;
}
