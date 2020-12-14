//
// Created by sr9000 on 14/12/2020.
//

#ifndef CMAKE_TINYPROXY_CONF_ANON_H
#define CMAKE_TINYPROXY_CONF_ANON_H

#define MAX_ANON_THROUGH_HEADERS 1000

typedef struct
{
  char *headers[MAX_ANON_THROUGH_HEADERS];
  int count;
} conf_anon_t, *pconf_anon_t;

extern void initialize_conf_anon_defaults(pconf_anon_t anon_config);
extern int assign_conf_anon(pconf_anon_t dest, pconf_anon_t src);

#endif // CMAKE_TINYPROXY_CONF_ANON_H
