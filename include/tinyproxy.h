//
// Created by sr9000 on 04/12/2020.
//

#ifndef CMAKE_TINYPROXY_TINYPROXY_H
#define CMAKE_TINYPROXY_TINYPROXY_H

#include "subservice/log.h"
#include "subservice/anonymous.h"
#include "self_contained/object.h"

typedef struct
{
  plog_t log;
  panon_t anon;
} *pproxy_t;

CREATE_DECL(pproxy_t);
DELETE_DECL(pproxy_t);

extern int configure_proxy(pproxy_t proxy, pconf_log_t log_config, pconf_anon_t anon_config);

#endif // CMAKE_TINYPROXY_TINYPROXY_H
