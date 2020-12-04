//
// Created by sr9000 on 04/12/2020.
//

#include <assert.h>
#include <stdlib.h>

#include "config/conf_log.h"
#include "debugtrace.h"
#include "misc/heap.h"
#include "subservice/log_levels.h"

void assign_default_values(pconf_log_t log_config)
{
  assert(log_config != NULL);
  log_config->logf_name = NULL;
  log_config->log_level = LOG_INFO;
}

void initialize_conf_log_defaults(pconf_log_t log_config)
{
  assert(log_config != NULL);
  safefree(log_config->logf_name);
  assign_default_values(log_config);
}

int assign_conf_log(pconf_log_t dest, pconf_log_t src)
{
  TRACECALLEX(assign_conf_log, "&dest = %p, &src = %p", (void *)dest, (void *)src);

  assert(dest != NULL);
  assert(src != NULL);
  initialize_conf_log_defaults(dest);

  dest->logf_name = safestrdup(src->logf_name);
  if (dest->logf_name == NULL)
  {
    TRACERETURNEX(-1, "Cannot alloc memory to copy src->logf_name (%p)", (void *)src->logf_name);
  }

  dest->log_level = src->log_level;

  TRACERETURN(0);
}
