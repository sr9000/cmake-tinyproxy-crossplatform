/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 2002       Petr Lampa <lampa@fit.vutbr.cz>
 * Copyright (C) 2008       Robert James Kaes <rjk@wormbytes.ca>
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

/*
 * This section of code is used for the transparent proxy option.  You will
 * need to configure your firewall to redirect all connections for HTTP
 * traffic to tinyproxy for this to work properly.
 */

#include "transparent-proxy.h"

#include "conf.h"
#include "conns.h"
#include "html-error.h"
#include "log.h"
#include "misc/heap.h"
#include "reqs.h"
#include "text.h"

/*
 * Build a URL from parts.
 */
static int build_url(char **url, const char *host, int port, const char *path)
{
  int len;

  assert(url != NULL);
  assert(host != NULL);
  assert(port > 0 && port < 32768);
  assert(path != NULL);

  len = strlen(host) + strlen(path) + 14;
  *url = (char *)saferealloc(*url, len);
  if (*url == NULL)
    return -1;

  return snprintf(*url, len, "http://%s:%d%s", host, port, path);
}

int do_transparent_proxy(struct conn_s *connptr, phashmap_t hashofheaders, struct request_s *request,
                         struct config_s *conf, char **url)
{
  socklen_t length;
  char *data;
  size_t ulen = strlen(*url);
  ssize_t i;

  length = hashmap_entry_by_key(hashofheaders, "host", (void **)&data);
  if (length <= 0)
  {
    struct sockaddr_in dest_addr;

    if (getsockname(connptr->client_fd, (struct sockaddr *)&dest_addr, &length) < 0)
    {
      log_message(LOG_ERR, "process_request: cannot get destination IP for %d", connptr->client_fd);
      indicate_http_error(connptr, 400, "Bad Request", "detail", "Unknown destination", "url", *url,
                          NULL);
      return 0;
    }

    request->host = (char *)safemalloc(17);
    strlcpy(request->host, inet_ntoa(dest_addr.sin_addr), 17);

    request->port = ntohs(dest_addr.sin_port);

    request->path = (char *)safemalloc(ulen + 1);
    strlcpy(request->path, *url, ulen + 1);

    build_url(url, request->host, request->port, request->path);
    log_message(LOG_INFO, "process_request: trans IP %s %s for %d", request->method, *url,
                connptr->client_fd);
  }
  else
  {
    request->host = (char *)safemalloc(length + 1);
    if (sscanf(data, "%[^:]:%hu", request->host, &request->port) != 2)
    {
      strlcpy(request->host, data, length + 1);
      request->port = HTTP_PORT;
    }

    request->path = (char *)safemalloc(ulen + 1);
    strlcpy(request->path, *url, ulen + 1);

    build_url(url, request->host, request->port, request->path);
    log_message(LOG_INFO, "process_request: trans Host %s %s for %d", request->method, *url,
                connptr->client_fd);
  }

  if (conf->listen_addrs == NULL)
  {
    return 1;
  }

  for (i = 0; i < list_length(conf->listen_addrs); i++)
  {
    const char *addr;

    addr = (char *)list_getentry(conf->listen_addrs, i, NULL);

    if (addr && strcmp(request->host, addr) == 0)
    {
      log_message(LOG_ERR,
                  "transparent: destination IP %s is local "
                  "on socket fd %d",
                  request->host, connptr->client_fd);
      indicate_http_error(connptr, 400, "Bad Request", "detail",
                          "You tried to connect to the "
                          "machine the proxy is running on",
                          "url", *url, NULL);
      return 0;
    }
  }

  return 1;
}
