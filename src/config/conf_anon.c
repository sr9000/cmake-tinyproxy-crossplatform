//
// Created by sr9000 on 14/12/2020.
//

#include "config/conf_anon.h"

#include "self_contained/safecall.h"

CREATE_IMPL(pconf_anon_t, { obj->count = 0; })

DELETE_IMPL(pconf_anon_t, {
  for (size_t i = 0; i < obj->count; ++i)
  {
    safefree(obj->headers[i]);
  }
  obj->count = 0;
})

CLONE_IMPL(pconf_anon_t, {
  for (dst->count = 0; dst->count < src->count; ++(dst->count))
  {
    const size_t i = dst->count;

    dst->headers[i] = safestrdup(src->headers[i]);
    if (dst->headers[i] == NULL)
    {
      // (1+i) cause use 1-based counting in message
      TRACE_MSG("failed on copying %zu(-th) header of %zu", 1 + i, src->count);
      break;
    }
  }

  if (dst->count < src->count)
  {
    delete_pconf_anon_t(&dst);
    TRACE_NULL;
  }
})

int add_header_conf_anon(pconf_anon_t anon_config, const char *header)
{
  TRACE_CALL_X(add_header, "anon_config = %p, header = %s", (void *)anon_config, header);

  TRACE_SAFE_X(MAX_ANON_THROUGH_HEADERS <= anon_config->count, -1,
               "exceedes header count limit (%zu)", MAX_ANON_THROUGH_HEADERS);

  const size_t i = anon_config->count;
  TRACE_SAFE_X(NULL == (anon_config->headers[i] = safestrdup(header)), -1,
               "cannot copy header \"%s\"", header);

  ++(anon_config->count);

  TRACE_SUCCESS;
}
