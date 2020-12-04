//
// Created by sr9000 on 04/12/2020.
//

#ifndef CMAKE_TINYPROXY_LOG_LEVELS_H
#define CMAKE_TINYPROXY_LOG_LEVELS_H

#ifdef MINGW
#define LOG_EMERG   0 // <syslog.h>
#define LOG_ALERT   1 // <syslog.h>
#define LOG_CRIT    2 // <syslog.h>
#define LOG_ERR     3 // <syslog.h>
#define LOG_WARNING 4 // <syslog.h>
#define LOG_NOTICE  5 // <syslog.h>
#define LOG_INFO    6 // <syslog.h>
#define LOG_DEBUG   7 // <syslog.h>

#define MAP_FAILED NULL // <sys/mman.h>

#else // MINGW
#include <syslog.h>
#endif // MINGW

#endif // CMAKE_TINYPROXY_LOG_LEVELS_H
