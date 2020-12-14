//
// Created by sr9000 on 14/12/2020.
//

#ifndef CMAKE_TINYPROXY_OBJECT_H
#define CMAKE_TINYPROXY_OBJECT_H

#include "debugtrace.h"
#include "misc/heap.h"

#define CREATE_DECL(ptype) extern ptype create_##ptype()
#define DELETE_DECL(ptype) extern int delete_##ptype(ptype *)
#define CLONE_DECL(ptype)  extern ptype clone_##ptype(ptype)

#define CREATE_IMPL(ptype, ...)                                                                    \
  ptype create_##ptype()                                                                           \
  {                                                                                                \
    TRACECALL(create_##ptype);                                                                     \
                                                                                                   \
    ptype obj = NULL;                                                                              \
                                                                                                   \
    obj = safemalloc(sizeof(*obj));                                                                \
    if (obj == NULL)                                                                               \
    {                                                                                              \
      TRACERETURNEX(NULL, "%s", "Cannot allocate memory.");                                        \
    }                                                                                              \
                                                                                                   \
    {                                                                                              \
      __VA_ARGS__;                                                                                 \
    }                                                                                              \
                                                                                                   \
    TRACERETURN(obj);                                                                              \
  }

#define DELETE_IMPL(ptype, ...)                                                                    \
  int delete_##ptype(ptype *arg)                                                                   \
  {                                                                                                \
    TRACECALLEX(delete_##ptype, #ptype "* = %p", (void *)arg);                                     \
                                                                                                   \
    if (arg == NULL)                                                                               \
    {                                                                                              \
      TRACERETURNEX(-1, "%s", "No " #ptype " to delete");                                          \
    }                                                                                              \
                                                                                                   \
    if (*arg == NULL)                                                                              \
    {                                                                                              \
      TRACERETURNEX(0, "%s", #ptype " had already been deleted");                                  \
    }                                                                                              \
                                                                                                   \
    {                                                                                              \
      ptype obj = *arg;                                                                            \
      __VA_ARGS__;                                                                                 \
    }                                                                                              \
                                                                                                   \
    safefree(*arg);                                                                                \
    TRACERETURN(0);                                                                                \
  }

#define CLONE_IMPL(ptype, ...)                                                                     \
  ptype clone_##ptype(ptype src)                                                                    \
  {                                                                                                \
    TRACECALLEX(clone_##ptype, #ptype " = %p", (void *)src);                                               \
                                                                                                   \
    if (src == NULL)                                                                               \
    {                                                                                              \
      TRACERETURNEX(NULL, "%s", "No " #ptype " to clone");                                         \
    }                                                                                              \
                                                                                                   \
    pconf_log_t dst = create_##ptype();                                                            \
                                                                                                   \
    {                                                                                              \
      __VA_ARGS__;                                                                                 \
    }                                                                                              \
                                                                                                   \
    TRACERETURN(dst);                                                                              \
  }

#endif // CMAKE_TINYPROXY_OBJECT_H
