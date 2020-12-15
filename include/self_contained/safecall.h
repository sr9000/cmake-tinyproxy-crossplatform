//
// Created by sr9000 on 14/12/2020.
//

#ifndef CMAKE_TINYPROXY_SAFECALL_H
#define CMAKE_TINYPROXY_SAFECALL_H

#include "self_contained/debugtrace.h"

#define TRACE_SUCCESS TRACE_RETURN(0)
#define TRACE_FAIL    TRACE_RETURN(-1)
#define TRACE_NULL    TRACE_RETURN(NULL)
#define TRACE_VOID    TRACE_RET_VOID

#define TRACE_SAFE(call)                                                                           \
  do                                                                                               \
  {                                                                                                \
    if ((call))                                                                                    \
    {                                                                                              \
      TRACE_FAIL;                                                                                  \
    }                                                                                              \
  } while (0)

#define TRACE_SAFE_R(call, ret)                                                                    \
  do                                                                                               \
  {                                                                                                \
    if ((call))                                                                                    \
    {                                                                                              \
      TRACE_RETURN(ret);                                                                           \
    }                                                                                              \
  } while (0)

#define TRACE_SAFE_FIN(call, ret, ...)                                                             \
  do                                                                                               \
  {                                                                                                \
    if ((call))                                                                                    \
    {                                                                                              \
      __VA_ARGS__;                                                                                 \
      TRACE_RETURN(ret);                                                                           \
    }                                                                                              \
  } while (0)

#define TRACE_SAFE_X(call, ret, fmt, ...)                                                          \
  do                                                                                               \
  {                                                                                                \
    if ((call))                                                                                    \
    {                                                                                              \
      TRACE_RETURN_X(ret, fmt, __VA_ARGS__);                                                       \
    }                                                                                              \
  } while (0)

#endif // CMAKE_TINYPROXY_SAFECALL_H
