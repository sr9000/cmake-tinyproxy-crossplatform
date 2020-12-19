//
// Created by sr9000 on 15/12/2020.
//

#ifndef CMAKE_TINYPROXY_CONF_ACL_H
#define CMAKE_TINYPROXY_CONF_ACL_H

#include <stddef.h>

#include "self_contained/object.h"
#include "subservice/acl_access_type.h"

typedef struct
{
  char *location;
  acl_access_t access;
} conf_acl_rule_t;

#define MAX_ACL_RULES ((size_t) 1000)

typedef struct
{
  conf_acl_rule_t rules[MAX_ACL_RULES];
  size_t count;
} *pconf_acl_t;

CREATE_DECL(pconf_acl_t);
DELETE_DECL(pconf_acl_t);
CLONE_DECL(pconf_acl_t);

extern int add_rule_conf_acl(pconf_acl_t acl_config, const char *location, acl_access_t access_type);

#endif // CMAKE_TINYPROXY_CONF_ACL_H
