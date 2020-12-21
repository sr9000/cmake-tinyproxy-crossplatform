//
// Created by sr9000 on 21/12/2020.
//

#include "config/conf_filt.h"

#include "self_contained/safecall.h"

CREATE_IMPL(pconf_filt_t, {
  obj->file_name = NULL;
  obj->enabled_url_filter = false;
  obj->is_case_sensitive = false;
  obj->is_extended = false;
  obj->default_policy = FILTER_ALLOW;
})

DELETE_IMPL(pconf_filt_t, { safefree(obj->file_name); })

CLONE_IMPL(pconf_filt_t, {
  dst->file_name = NULL;
  TRACE_SAFE_FIN(NULL == (dst->file_name = safestrdup(src->file_name)), NULL,
                 { delete_pconf_filt_t(&dst); });
  dst->enabled_url_filter = src->enabled_url_filter;
  dst->is_extended = src->is_extended;
  dst->is_case_sensitive = src->is_case_sensitive;
  dst->default_policy = src->default_policy;
})
