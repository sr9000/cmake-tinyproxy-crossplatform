//
// Created by sr9000 on 04/12/2020.
//

#ifndef CMAKE_TINYPROXY_CONF_LOG_H
#define CMAKE_TINYPROXY_CONF_LOG_H

#include "self_contained/object.h"

typedef struct
{
  char *logf_name;
  int log_level;
} *pconf_log_t;

CREATE_DECL(pconf_log_t);
DELETE_DECL(pconf_log_t);
CLONE_DECL(pconf_log_t);

#endif // CMAKE_TINYPROXY_CONF_LOG_H
