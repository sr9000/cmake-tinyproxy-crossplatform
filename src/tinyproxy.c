//
// Created by sr9000 on 04/12/2020.
//

#include "tinyproxy.h"

#include "self_contained/safecall.h"

CREATE_IMPL(pproxy_t, { TRACE_SAFE_R(NULL == (obj->log = create_plog_t()), NULL); })
DELETE_IMPL(pproxy_t, { TRACE_SAFE(delete_plog_t(&obj->log)); })

int configure_log(pproxy_t proxy, pconf_log_t log_config)
{
  TRACE_CALL_X(configure_log, "proxy = %p, log_config = %p", (void *)proxy, (void *)log_config);

  TRACE_SAFE_X(NULL == proxy, -1, "%s", "there is no proxy object to configure");
  TRACE_SAFE_X(NULL == log_config, -1, "%s",
               "there is no log configuration to configure proxy logging");

  TRACE_SAFE(delete_plog_t(&proxy->log));
  TRACE_SAFE(NULL == (proxy->log = create_configured_log(log_config)));

  TRACE_SUCCESS;
}
