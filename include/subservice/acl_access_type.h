//
// Created by sr9000 on 15/12/2020.
//

#ifndef CMAKE_TINYPROXY_ACL_ACCESS_TYPE_H
#define CMAKE_TINYPROXY_ACL_ACCESS_TYPE_H

typedef enum
{
  ACL_ALLOW,
  ACL_DENY
} acl_access_t;

static char s_acl_allow[] = "acl-allow";
static char s_acl_deny[] = "acl-deny";

static char* acl_access_t2char(acl_access_t t)
{
  switch (t)
  {
  case ACL_ALLOW:
    return s_acl_allow;
  case ACL_DENY:
    return s_acl_deny;
  default:
    return NULL;
  }
}

#endif // CMAKE_TINYPROXY_ACL_ACCESS_TYPE_H
