//
// Created by sr9000 on 21/12/2020.
//

#include "config/conf_filt.h"

#include "self_contained/safecall.h"

CREATE_IMPL(pconf_filt_t, {
  obj->enabled = false;
  obj->file_path = NULL;
  obj->does_full_url_filtering = false;
  obj->is_case_sensitive = false;
  obj->is_extended = false;
  obj->policy = FILTER_WHITE_LIST;
})

DELETE_IMPL(pconf_filt_t, { safefree(obj->file_path); })

CLONE_IMPL(pconf_filt_t, {
  dst->file_path = NULL;
  TRACE_SAFE_FIN(NULL == (dst->file_path = safestrdup(src->file_path)), NULL,
                 { delete_pconf_filt_t(&dst); });
  dst->enabled = src->enabled;
  dst->does_full_url_filtering = src->does_full_url_filtering;
  dst->is_extended = src->is_extended;
  dst->is_case_sensitive = src->is_case_sensitive;
  dst->policy = src->policy;
})
