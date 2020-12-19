//
// Created by sr9000 on 19/12/2020.
//

#include "config/conf_auth.h"

#include "self_contained/safecall.h"

int clean_auth_cred_t(conf_auth_cred_t *rule)
{
  TRACE_CALL(clean_acl_rule_t);
  TRACE_SAFE_X(NULL == rule, -1, "%s", "there is no cred to init");
  safefree(rule->user);
  safefree(rule->pass);
  TRACE_SUCCESS;
}

int init_auth_cred_t(conf_auth_cred_t *cred, const char *user, const char *pass)
{
  TRACE_CALL(init_acl_rule_t);
  TRACE_SAFE_X(NULL == cred, -1, "%s", "there is no cred to init");

  cred->user = NULL;
  cred->pass = NULL;

  TRACE_SAFE_FIN(NULL == (cred->user = safestrdup(user)), -1, clean_auth_cred_t(cred));
  TRACE_SAFE_FIN(NULL == (cred->pass = safestrdup(pass)), -1, clean_auth_cred_t(cred));

  TRACE_SUCCESS;
}

int assign_auth_cred_t(conf_auth_cred_t *dst, conf_auth_cred_t *src)
{
  TRACE_CALL(assign_acl_rule_t);
  TRACE_SAFE_X(NULL == dst, -1, "%s", "there is no cred to init");
  TRACE_SAFE_X(NULL == src, -1, "%s", "there is no cred to assign");
  TRACE_SAFE(init_auth_cred_t(dst, src->user, src->pass));
  TRACE_SUCCESS;
}

CREATE_IMPL(pconf_auth_t, { obj->count = 0; })

DELETE_IMPL(pconf_auth_t, {
  for (size_t i = 0; i < obj->count; ++i)
  {
    safefree(obj->creds[i].user);
    safefree(obj->creds[i].pass);
  }
  obj->count = 0;
})

CLONE_IMPL(pconf_auth_t, {
  for (dst->count = 0; dst->count < src->count; ++(dst->count))
  {
    const size_t i = dst->count;

    dst->creds[i].user = NULL;
    dst->creds[i].pass = NULL;

    dst->creds[i].user = safestrdup(src->creds[i].user);
    if (NULL == dst->creds[i].user)
    {
      // (1+i) cause use 1-based counting in message
      TRACE_MSG("failed on copying %zu(-th) cred's user of %zu", 1 + i, src->count);
      break;
    }

    dst->creds[i].pass = safestrdup(src->creds[i].pass);
    if (NULL == dst->creds[i].pass)
    {
      // (1+i) cause use 1-based counting in message
      TRACE_MSG("failed on copying %zu(-th) cred's pass of %zu", 1 + i, src->count);
      break;
    }
  }

  if (dst->count < src->count)
  {
    delete_pconf_auth_t(&dst);
    TRACE_NULL;
  }
})

int add_cred_conf_auth(pconf_auth_t auth_config, const char *user, const char *pass)
{
  TRACE_CALL_X(add_cred_conf_auth, "auth_config = %p, user = %s, pass = *****",
               (void *)auth_config, user);
  TRACE_SAFE_X(MAX_AUTH_CREDS <= auth_config->count, -1, "exceedes auth creds count limit (%zu)",
               MAX_AUTH_CREDS);

  const size_t i = auth_config->count;
  TRACE_SAFE(init_auth_cred_t(auth_config->creds + i, user, pass));

  ++(auth_config->count);

  TRACE_SUCCESS;
}
