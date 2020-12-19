//
// Created by sr9000 on 04/12/2020.
//

#ifndef CMAKE_TINYPROXY_TINYPROXY_H
#define CMAKE_TINYPROXY_TINYPROXY_H

#include "self_contained/object.h"
#include "subservice/acl.h"
#include "subservice/anonymous.h"
#include "subservice/basicauth.h"
#include "subservice/log.h"

typedef struct
{
  plog_t log;
  panon_t anon;
  pacl_t acl;
  pauth_t auth;
} *pproxy_t;

CREATE_DECL(pproxy_t);
DELETE_DECL(pproxy_t);

extern int configure_proxy(pproxy_t proxy, pconf_log_t log_config, pconf_anon_t anon_config,
                           pconf_acl_t acl_config, pconf_auth_t auth_config);

#endif // CMAKE_TINYPROXY_TINYPROXY_H
