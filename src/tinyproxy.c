//
// Created by sr9000 on 04/12/2020.
//

#include "tinyproxy.h"

#include "self_contained/safecall.h"

CREATE_IMPL(pproxy_t, {
  obj->log = NULL;
  obj->anon = NULL;

  obj->log = create_plog_t();
  TRACE_SAFE_FIN(NULL == obj->log, NULL, delete_pproxy_t(&obj));

  obj->anon = create_panon_t();
  TRACE_SAFE_FIN(NULL == obj->anon, NULL, delete_pproxy_t(&obj));
})

DELETE_IMPL(pproxy_t, {
  TRACE_SAFE(delete_plog_t(&obj->log));
  TRACE_SAFE(delete_panon_t(&obj->anon));
})

int configure_proxy(pproxy_t proxy, pconf_log_t log_config, pconf_anon_t anon_config)
{
  TRACE_CALL_X(configure_proxy, "proxy = %p, log_config = %p", (void *)proxy, (void *)log_config);

  TRACE_SAFE_X(NULL == proxy, -1, "%s", "there is no proxy object to configure");
  TRACE_SAFE_X(NULL == log_config, -1, "%s",
               "there is no log configuration to configure proxy logging");
  TRACE_SAFE_X(NULL == anon_config, -1, "%s",
               "there is no anon configuration to configure proxy anon headers");

  TRACE_SAFE(delete_plog_t(&proxy->log));
  TRACE_SAFE(NULL == (proxy->log = create_configured_log(log_config)));

  TRACE_SAFE(delete_panon_t(&proxy->anon));
  TRACE_SAFE(NULL == (proxy->anon = create_configured_anon(anon_config)));

  TRACE_SUCCESS;
}
