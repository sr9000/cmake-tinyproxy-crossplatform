//
// Created by sr9000 on 04/12/2020.
//

#ifndef CMAKE_TINYPROXY_CONF_LOG_H
#define CMAKE_TINYPROXY_CONF_LOG_H

typedef struct
{
  char *logf_name;
  int log_level;
} conf_log_t, *pconf_log_t;

extern void initialize_conf_log_defaults(pconf_log_t log_config);
extern int assign_conf_log(pconf_log_t dest, pconf_log_t src);

#endif // CMAKE_TINYPROXY_CONF_LOG_H
