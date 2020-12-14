//
// Created by sr9000 on 04/12/2020.
//

#ifndef CMAKE_TINYPROXY_TINYPROXY_H
#define CMAKE_TINYPROXY_TINYPROXY_H

#include "subservice/log.h"
#include "self_contained/object.h"

typedef struct
{
  plog_t log;
} *pproxy_t;

CREATE_DECL(pproxy_t);
DELETE_DECL(pproxy_t);

extern int configure_log(pproxy_t, pconf_log_t);

#endif // CMAKE_TINYPROXY_TINYPROXY_H
