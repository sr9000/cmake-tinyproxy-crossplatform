//
// Created by sr9000 on 17/12/2020.
//

#ifndef CMAKE_TINYPROXY_TINYPROXY_LIB_H
#define CMAKE_TINYPROXY_TINYPROXY_LIB_H

extern int run_proxy(void *nothing);

#ifdef MINGW
extern DWORD spawn_proxy(void *nothing);
#else // MINGW
extern pid_t spawn_proxy(void *nothing);
#endif // MINGW

#endif // CMAKE_TINYPROXY_TINYPROXY_LIB_H
