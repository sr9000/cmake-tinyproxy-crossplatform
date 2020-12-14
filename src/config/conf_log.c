//
// Created by sr9000 on 04/12/2020.
//

#include "config/conf_log.h"

#include "subservice/log_levels.h"

CREATE_IMPL(pconf_log_t, {
  obj->logf_name = NULL;
  obj->log_level = LOG_INFO;
})

DELETE_IMPL(pconf_log_t, { safefree(obj->logf_name); })

CLONE_IMPL(pconf_log_t, {
  if (src->logf_name != NULL)
  {
    dst->logf_name = safestrdup(src->logf_name);
    if (dst->logf_name == NULL)
    {
      delete_pconf_log_t(&dst);
      TRACERETURNEX(NULL, "Cannot alloc memory to copy src->logf_name (%p)",
                    (void *)src->logf_name);
    }
  }

  dst->log_level = src->log_level;
})
