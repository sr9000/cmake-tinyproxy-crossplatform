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

#include <string.h>

#include "subservice/basicauth.h"

#include "misc/base64.h"
#include "misc/heap.h"
#include "self_contained/safecall.h"

int basicauth_add(plist_t authlist, const char *user, const char *pass);

struct auth_s
{
  plist_t creds;
};

CREATE_IMPL(pauth_t, {
  obj->creds = NULL;
  TRACE_SAFE_FIN(NULL == (obj->creds = list_create()), NULL, delete_pauth_t(&obj));
})

DELETE_IMPL(pauth_t, { list_delete(obj->creds); })

pauth_t create_configured_auth(pconf_auth_t auth_config)
{
  TRACE_CALL(create_configured_auth);
  pauth_t auth;
  TRACE_SAFE_R(NULL == (auth = create_pauth_t()), NULL);

  for (size_t i = 0; i < auth_config->count; ++i)
  {
    TRACE_SAFE_FIN(
        basicauth_add(auth->creds, auth_config->creds[i].user, auth_config->creds[i].pass), NULL,
        { delete_pauth_t(&auth); });
  }

  TRACE_RETURN(auth);
}

// create basic-auth token in buf
// returns: -1 if any error occurred
//           0 if all is ok
#define MAXLEN ((int)(256 + 2))
int make_auth_string(char *buf, size_t bufsize, const char *user, const char *pass)
{
  TRACE_CALL_X(make_auth_string, "&buf = %p, bufsize = %zu, user = %s, pass = *****", (void *)buf,
               bufsize, user);
  TRACE_SAFE(NULL == buf);
  TRACE_SAFE(NULL == user);
  TRACE_SAFE(NULL == pass);

  char tmp[MAXLEN];
  int l = snprintf(tmp, MAXLEN, "%s:%s", user, pass);

  TRACE_SAFE(l < 0 || l >= MAXLEN);
  TRACE_SAFE(bufsize < (BASE64ENC_BYTES((size_t)l) + 1));
  base64enc(buf, tmp, l);

  TRACE_SUCCESS;
}

// add entry to the basicauth list
int basicauth_add(plist_t authlist, const char *user, const char *pass)
{
  TRACE_CALL_X(basicauth_add, "&authlist = %p, user = %s, pass = *****", (void *)authlist, user);

  char b[BASE64ENC_BYTES((256 + 2) - 1) + 1];

  TRACE_SAFE(make_auth_string(b, sizeof(b), user, pass));
  TRACE_SAFE(list_append(authlist, b, strlen(b) + 1));

  TRACE_SUCCESS;
}

bool is_basicauth_required(pauth_t auth)
{
  return (list_length(auth->creds) > 0);
}

// check if a user/password combination (encoded as base64) is in the basicauth list
// return:  true on success
//         false on failure
bool does_pass_auth_chek(plog_t log, pauth_t auth, const char *authstring)
{
  if (NULL == auth)
  {
    log_message(log, LOG_ERR, "%s", "auth object is not initialized");
    return false;
  }

  ssize_t list_len = list_length(auth->creds);
  if (list_len < 0)
  {
    log_message(log, LOG_WARNING, "%s", "auth object is broken");
    return false;
  }

  for (size_t i = 0; i < ((size_t)(list_len)); ++i)
  {
    size_t entry_size;
    const char *entry = list_getentry(auth->creds, i, &entry_size);
    if (0 == strcmp(authstring, entry))
    {
      return true;
    }
  }

  return false;
}
