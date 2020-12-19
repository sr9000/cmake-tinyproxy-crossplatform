//
// Created by sr9000 on 19/12/2020.
//

#ifndef CMAKE_TINYPROXY_CONF_AUTH_H
#define CMAKE_TINYPROXY_CONF_AUTH_H

#include "self_contained/object.h"

#define MAX_AUTH_CREDS ((size_t)1000)

typedef struct {
  char* user;
  char* pass;
} conf_auth_cred_t;

typedef struct
{
  conf_auth_cred_t creds[MAX_AUTH_CREDS];
  size_t count;
} *pconf_auth_t;

CREATE_DECL(pconf_auth_t);
DELETE_DECL(pconf_auth_t);
CLONE_DECL(pconf_auth_t);

extern int add_cred_conf_auth(pconf_auth_t auth_config, const char *user, const char *pass);

#endif // CMAKE_TINYPROXY_CONF_AUTH_H
