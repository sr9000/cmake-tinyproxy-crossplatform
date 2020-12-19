//
// Created by sr9000 on 15/12/2020.
//

#include "config/conf_acl.h"
#include "self_contained/safecall.h"

static char s_acl_allow[] = "acl-allow";
static char s_acl_deny[] = "acl-deny";

char *acl_access_t2char(acl_access_t t)
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

int init_acl_rule_t(conf_acl_rule_t *rule, const char *location, acl_access_t access)
{
  TRACE_CALL(init_acl_rule_t);
  TRACE_SAFE_X(NULL == rule, -1, "%s", "there is no rule to init");
  TRACE_SAFE(NULL == (rule->location = safestrdup(location)));
  rule->access = access;
  TRACE_SUCCESS;
}

int clean_acl_rule_t(conf_acl_rule_t *rule)
{
  TRACE_CALL(clean_acl_rule_t);
  TRACE_SAFE_X(NULL == rule, -1, "%s", "there is no rule to init");
  safefree(rule->location);
  TRACE_SUCCESS;
}

int assign_acl_rule_t(conf_acl_rule_t *dst, conf_acl_rule_t *src)
{
  TRACE_CALL(assign_acl_rule_t);
  TRACE_SAFE_X(NULL == dst, -1, "%s", "there is no rule to init");
  TRACE_SAFE_X(NULL == src, -1, "%s", "there is no rule to assign");
  TRACE_SAFE(init_acl_rule_t(dst, src->location, src->access));
  TRACE_SUCCESS;
}

CREATE_IMPL(pconf_acl_t, { obj->count = 0; })

DELETE_IMPL(pconf_acl_t, {
  for (size_t i = 0; i < obj->count; ++i)
  {
    TRACE_SAFE(clean_acl_rule_t(obj->rules + i));
  }
  obj->count = 0;
})

CLONE_IMPL(pconf_acl_t, {
  for (dst->count = 0; dst->count < src->count; ++(dst->count))
  {
    const size_t i = dst->count;
    if (assign_acl_rule_t(dst->rules + i, src->rules + i))
    {
      // (1 + i) cause 1-based value in message
      TRACE_MSG("failed at copying %zu(-th) rule of %zu", 1 + i, src->count);
      break;
    }
  }

  if (dst->count < src->count)
  {
    delete_pconf_acl_t(&dst);
    TRACE_NULL;
  }
})

int add_rule_conf_acl(pconf_acl_t acl_config, const char *location, acl_access_t access_type)
{
  TRACE_CALL_X(add_rule_conf_acl, "acl_config = %p, location = %s, access_type = %s",
               (void *)acl_config, location, acl_access_t2char(access_type));
  TRACE_SAFE_X(MAX_ACL_RULES <= acl_config->count, -1, "exceedes acl rules count limit (%zu)",
               MAX_ACL_RULES);

  const size_t i = acl_config->count;
  TRACE_SAFE(init_acl_rule_t(acl_config->rules + i, location, access_type));

  ++(acl_config->count);

  TRACE_SUCCESS;
}
