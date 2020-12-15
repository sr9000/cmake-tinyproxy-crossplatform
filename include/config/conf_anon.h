//
// Created by sr9000 on 14/12/2020.
//

#ifndef CMAKE_TINYPROXY_CONF_ANON_H
#define CMAKE_TINYPROXY_CONF_ANON_H

#include "self_contained/object.h"

#define MAX_ANON_THROUGH_HEADERS ((size_t)1000)

typedef struct
{
  char *headers[MAX_ANON_THROUGH_HEADERS];
  size_t count;
} *pconf_anon_t;

CREATE_DECL(pconf_anon_t);
DELETE_DECL(pconf_anon_t);
CLONE_DECL(pconf_anon_t);

extern int add_header_conf_anon(pconf_anon_t anon_config, const char *header);

#endif // CMAKE_TINYPROXY_CONF_ANON_H
