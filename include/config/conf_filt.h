//
// Created by sr9000 on 21/12/2020.
//

#ifndef CMAKE_TINYPROXY_CONF_FILT_H
#define CMAKE_TINYPROXY_CONF_FILT_H

#include <stdbool.h>

#include "self_contained/object.h"
#include "subservice/filter_policy.h"

typedef struct
{
  // file with filter rules
  char *file_path;
  bool enabled; // is filtering process active

  filter_policy_t policy;

  bool does_full_url_filtering; // full url filtering if true, else by host
  bool is_extended;             // extended regexp in filter list
  bool is_case_sensitive;       // case sensitive regexp in filter list
} *pconf_filt_t;

CREATE_DECL(pconf_filt_t);
DELETE_DECL(pconf_filt_t);
CLONE_DECL(pconf_filt_t);

#endif // CMAKE_TINYPROXY_CONF_FILT_H
