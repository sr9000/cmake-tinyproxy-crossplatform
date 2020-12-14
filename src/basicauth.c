/* tinyproxy - A fast light-weight HTTP proxy
 * This file: Copyright (C) 2016-2017 rofl0r
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

#include "main.h"

#include "basicauth.h"
#include "self_contained/debugtrace.h"

#include "config/conf.h"
#include "conns.h"
#include "html-error.h"
#include "misc/base64.h"
#include "misc/heap.h"
#include "subservice/log.h"

/*
 * Create basic-auth token in buf.
 * Returns strlen of token on success,
 * -1 if user/pass missing
 * 0 if user/pass too long
 */
ssize_t basicauth_string(const char *user, const char *pass, char *buf, size_t bufsize)
{
  char tmp[256 + 2];
  int l;
  if (!user || !pass)
    return -1;
  l = snprintf(tmp, sizeof tmp, "%s:%s", user, pass);
  if (l < 0 || l >= (ssize_t)sizeof tmp)
    return 0;
  if (bufsize < (BASE64ENC_BYTES((unsigned)l) + 1))
    return 0;
  base64enc(buf, tmp, l);
  return BASE64ENC_BYTES(l);
}

/*
 * Add entry to the basicauth list
 */
int basicauth_add(plist_t authlist, const char *user, const char *pass)
{
  TRACECALLEX(basicauth_add, "&authlist = %p, user = %s, pass = *****", (void *)authlist, user);

  char b[BASE64ENC_BYTES((256 + 2) - 1) + 1];
  ssize_t ret;

  ret = basicauth_string(user, pass, b, sizeof b);
  if (ret == -1)
  {
    TRACERETURNEX(-1, "%s", "Illegal basicauth rule: missing user or pass");
  }
  else if (ret == 0)
  {
    TRACERETURNEX(-1, "%s", "User / pass in basicauth rule too long");
  }

  if (list_append(authlist, b, ret + 1) == -ENOMEM)
  {
    TRACERETURNEX(-1, "%s", "Unable to allocate memory in basicauth_add()");
  }

  TRACERETURN(0);
}

/*
 * Check if a user/password combination (encoded as base64)
 * is in the basicauth list.
 * return 1 on success, 0 on failure.
 */
int basicauth_check(plist_t authlist, const char *authstring)
{
  ssize_t vl, i;
  size_t el;
  const char *entry;

  vl = list_length(authlist);
  if (vl == -EINVAL)
    return 0;

  for (i = 0; i < vl; i++)
  {
    entry = list_getentry(authlist, i, &el);
    if (strcmp(authstring, entry) == 0)
      return 1;
  }
  return 0;
}
