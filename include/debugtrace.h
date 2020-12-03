//
// Created by sr9000 on 03/12/2020.
//

#ifndef CMAKE_TINYPROXY_DEBUGTRACE_H
#define CMAKE_TINYPROXY_DEBUGTRACE_H

#ifdef NDEBUG
#define TRACECALL(fname)                                                                           \
  do                                                                                               \
  {                                                                                                \
  } while (0)
#define TRACECALLEX(fname, fmt, args...)                                                           \
  do                                                                                               \
  {                                                                                                \
  } while (0)

#define TRACERETVOID                     return
#define TRACERETURN(res)                 return (res)
#define TRACERETURNEX(res, fmt, args...) return (res)
#else // NDEBUG
#include <stdio.h>

// Suppress warnings when GCC is in -pedantic mode and not -std=c99
#if (__GNUC__ >= 3 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96))
#pragma GCC system_header
#endif

#define TRACECALL(fname)                                                                           \
  fprintf(stderr, "TRACE {enter " #fname "} [%s:%d]\n", __FILE__, __LINE__);                       \
  char _8BF664F4[] = #fname

#define TRACECALLEX(fname, fmt, args...)                                                           \
  fprintf(stderr, "TRACE {enter " #fname " (" fmt ")} [%s:%d]\n", ##args, __FILE__, __LINE__);     \
  char _8BF664F4[] = #fname

#define TRACERETVOID                                                                               \
  fprintf(stderr, "TRACE {leave %s} [%s:%d]\n", _8BF664F4, __FILE__, __LINE__);                    \
  return

#define TRACERETURN(res)                                                                           \
  fprintf(stderr, "TRACE {leave %s with (%s)} [%s:%d]\n", _8BF664F4, #res, __FILE__, __LINE__);    \
  return (res)

#define TRACERETURNEX(res, fmt, args...)                                                           \
  fprintf(stderr, "TRACE {leave %s with (%s): " fmt "} [%s:%d]\n", _8BF664F4, #res, ##args,        \
          __FILE__, __LINE__);                                                                     \
  return (res)
#endif // NDEBUG

#endif // CMAKE_TINYPROXY_DEBUGTRACE_H
