//
// Created by sr9000 on 04/12/2020.
//

#include "config/conf_log.h"

#include "self_contained/safecall.h"
#include "subservice/log_levels.h"

CREATE_IMPL(pconf_log_t, {
  obj->logf_name = NULL;
  obj->log_level = LOG_INFO;
})

DELETE_IMPL(pconf_log_t, { safefree(obj->logf_name); })

CLONE_IMPL(pconf_log_t, {
  if (NULL != src->logf_name)
  {
    dst->logf_name = safestrdup(src->logf_name);
    if (NULL == dst->logf_name)
    {
      TRACE_MSG("cannot alloc memory to copy src->logf_name (%p)", (void *)src->logf_name);
      delete_pconf_log_t(&dst);
      TRACE_NULL;
    }
  }

  dst->log_level = src->log_level;
})
