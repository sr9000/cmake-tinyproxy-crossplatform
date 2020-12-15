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
    TRACE_CALL(create_##ptype);                                                                    \
                                                                                                   \
    ptype obj = NULL;                                                                              \
                                                                                                   \
    obj = safemalloc(sizeof(*obj));                                                                \
    if (obj == NULL)                                                                               \
    {                                                                                              \
      TRACE_RETURN_X(NULL, "%s", "cannot allocate memory.");                                       \
    }                                                                                              \
                                                                                                   \
    {                                                                                              \
      __VA_ARGS__;                                                                                 \
    }                                                                                              \
                                                                                                   \
    TRACE_RETURN(obj);                                                                             \
  }

#define DELETE_IMPL(ptype, ...)                                                                    \
  int delete_##ptype(ptype *arg)                                                                   \
  {                                                                                                \
    TRACE_CALL_X(delete_##ptype, #ptype "* = %p", (void *)arg);                                    \
                                                                                                   \
    if (arg == NULL)                                                                               \
    {                                                                                              \
      TRACE_RETURN_X(-1, "%s", "no " #ptype " to delete");                                         \
    }                                                                                              \
                                                                                                   \
    if (*arg == NULL)                                                                              \
    {                                                                                              \
      TRACE_RETURN_X(0, "%s", #ptype " had already been deleted");                                 \
    }                                                                                              \
                                                                                                   \
    {                                                                                              \
      ptype obj = *arg;                                                                            \
      __VA_ARGS__;                                                                                 \
    }                                                                                              \
                                                                                                   \
    safefree(*arg);                                                                                \
    TRACE_RETURN(0);                                                                               \
  }

#define CLONE_IMPL(ptype, ...)                                                                     \
  ptype clone_##ptype(ptype src)                                                                   \
  {                                                                                                \
    TRACE_CALL_X(clone_##ptype, #ptype " = %p", (void *)src);                                      \
                                                                                                   \
    if (src == NULL)                                                                               \
    {                                                                                              \
      TRACE_RETURN_X(NULL, "%s", "no " #ptype " to clone");                                        \
    }                                                                                              \
                                                                                                   \
    ptype dst = create_##ptype();                                                                  \
                                                                                                   \
    {                                                                                              \
      __VA_ARGS__;                                                                                 \
    }                                                                                              \
                                                                                                   \
    TRACE_RETURN(dst);                                                                             \
  }

#endif // CMAKE_TINYPROXY_OBJECT_H
