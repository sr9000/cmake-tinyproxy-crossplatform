//
// Created by sr9000 on 04/12/2020.
//

#include "tinyproxy.h"

#include "self_contained/safecall.h"

CREATE_IMPL(pproxy_t, {
  obj->log = NULL;
  obj->anon = NULL;
  obj->acl = NULL;
  obj->auth = NULL;
  obj->filter = NULL;

  obj->log = create_plog_t();
  TRACE_SAFE_FIN(NULL == obj->log, NULL, delete_pproxy_t(&obj));

  obj->anon = create_panon_t();
  TRACE_SAFE_FIN(NULL == obj->anon, NULL, delete_pproxy_t(&obj));

  obj->acl = create_pacl_t();
  TRACE_SAFE_FIN(NULL == obj->anon, NULL, delete_pproxy_t(&obj));

  obj->auth = create_pauth_t();
  TRACE_SAFE_FIN(NULL == obj->auth, NULL, delete_pproxy_t(&obj));

  obj->filter = create_pfilter_t();
  TRACE_SAFE_FIN(NULL == obj->filter, NULL, delete_pproxy_t(&obj));
})

DELETE_IMPL(pproxy_t, {
  TRACE_SAFE(delete_plog_t(&obj->log));
  TRACE_SAFE(delete_panon_t(&obj->anon));
  TRACE_SAFE(delete_pacl_t(&obj->acl));
  TRACE_SAFE(delete_pauth_t(&obj->auth));
  TRACE_SAFE(delete_pfilter_t(&obj->filter));
})

int configure_proxy(pproxy_t proxy, pconf_log_t log_config, pconf_anon_t anon_config,
                    pconf_acl_t acl_config, pconf_auth_t auth_config, pconf_filt_t filt_config)
{
  TRACE_CALL_X(configure_proxy, "proxy = %p, log_config = %p", (void *)proxy, (void *)log_config);

  TRACE_SAFE_X(NULL == proxy, -1, "%s", "there is no proxy object to configure");
  TRACE_SAFE_X(NULL == log_config, -1, "%s",
               "there is no log configuration to configure proxy logging");
  TRACE_SAFE_X(NULL == anon_config, -1, "%s",
               "there is no anon configuration to configure proxy anon headers");
  TRACE_SAFE_X(NULL == acl_config, -1, "%s",
               "there is no acl configuration to configure proxy access list");
  TRACE_SAFE_X(NULL == auth_config, -1, "%s",
               "there is no auth configuration to configure proxy basicauth creds");
  TRACE_SAFE_X(NULL == filt_config, -1, "%s",
               "there is no filter configuration to configure proxy filter rules");

  TRACE_SAFE(delete_plog_t(&proxy->log));
  TRACE_SAFE(NULL == (proxy->log = create_configured_log(log_config)));

  TRACE_SAFE(delete_panon_t(&proxy->anon));
  TRACE_SAFE(NULL == (proxy->anon = create_configured_anon(anon_config)));

  TRACE_SAFE(delete_pacl_t(&proxy->acl));
  TRACE_SAFE(NULL == (proxy->acl = create_configured_acl(acl_config)));

  TRACE_SAFE(delete_pauth_t(&proxy->auth));
  TRACE_SAFE(NULL == (proxy->auth = create_configured_auth(auth_config)));

  TRACE_SAFE(delete_pfilter_t(&proxy->filter));
  TRACE_SAFE(NULL == (proxy->filter = create_configured_filter(filt_config)));

  TRACE_SUCCESS;
}
