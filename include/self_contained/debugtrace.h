//
// Created by sr9000 on 03/12/2020.
//

#ifndef CMAKE_TINYPROXY_DEBUGTRACE_H
#define CMAKE_TINYPROXY_DEBUGTRACE_H

#ifdef NDEBUG
#define TRACE_CALL(fname)                                                                          \
  do                                                                                               \
  {                                                                                                \
  } while (0)
#define TRACE_CALL_X(fname, fmt, ...)                                                              \
  do                                                                                               \
  {                                                                                                \
  } while (0)

#define TRACE_RET_VOID                return
#define TRACE_RETURN(res)             return (res)
#define TRACE_RETURN_X(res, fmt, ...) return (res)
#define TRACE_MSG(fmt, ...)                                                                        \
  do                                                                                               \
  {                                                                                                \
  } while (0)
#else // NDEBUG
#include <stdio.h>

// Suppress warnings when GCC is in -pedantic mode and not -std=c99
#if (__GNUC__ >= 3 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96))
#pragma GCC system_header
#endif

#define TRACE_CALL(fname)                                                                          \
  fprintf(stderr, "TRACE {enter " #fname "} [%s:%d]\n", __FILE__, __LINE__);                       \
  char _8BF664F4[] = #fname

#define TRACE_CALL_X(fname, fmt, ...)                                                              \
  fprintf(stderr, "TRACE {enter " #fname ": " fmt "} [%s:%d]\n", __VA_ARGS__, __FILE__,           \
          __LINE__);                                                                               \
  char _8BF664F4[] = #fname

#define TRACE_RET_VOID                                                                             \
  fprintf(stderr, "TRACE {leave %s} [%s:%d]\n", _8BF664F4, __FILE__, __LINE__);                    \
  return

#define TRACE_RETURN(res)                                                                          \
  fprintf(stderr, "TRACE {leave %s with (%s)} [%s:%d]\n", _8BF664F4, #res, __FILE__, __LINE__);    \
  return (res)

#define TRACE_RETURN_X(res, fmt, ...)                                                              \
  fprintf(stderr, "TRACE {leave %s with (%s): " fmt "} [%s:%d]\n", _8BF664F4, #res, __VA_ARGS__,   \
          __FILE__, __LINE__);                                                                     \
  return (res)

#define TRACE_MSG(fmt, ...)                                                                        \
  fprintf(stderr, "TRACE {message %s: " fmt "} [%s:%d]\n", _8BF664F4, __VA_ARGS__, __FILE__,       \
          __LINE__)
#endif // NDEBUG

#endif // CMAKE_TINYPROXY_DEBUGTRACE_H
