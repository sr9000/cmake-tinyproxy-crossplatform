cmake_minimum_required(VERSION 3.17)
project(cmake_tinyproxy C)

set(CMAKE_C_STANDARD 11)

set(MAJOR_VERSION 2)
set(MINOR_VERSION 0)
set(MICRO_VERSION 0)
set(REAL_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION})


# Custom cmake scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(HelpProxyMisc)

global_proxy_list(PROXY_DEFINITIONS)
global_proxy_list(PROXY_LIBRARIES)

if (MINGW)
    global_proxy_list_append(PROXY_DEFINITIONS MINGW)
    proxy_require_lib_with_func(inet_addr wsock32 HAVE_WSOCK32 PROXY_LIBRARIES PROXY_DEFINITIONS)
    proxy_require_lib_with_func(inet_addr ws2_32 HAVE_WS2_32 PROXY_LIBRARIES PROXY_DEFINITIONS)
    proxy_require_lib_with_func(regcomp systre HAVE_SYSTRE PROXY_LIBRARIES PROXY_DEFINITIONS)
else (MINGW)
    proxy_require_lib_with_func(inet_aton resolv HAVE_LIBRESOLV PROXY_LIBRARIES PROXY_DEFINITIONS)
    proxy_require_lib_with_func(gethostbyname nsl HAVE_LIBNSL PROXY_LIBRARIES PROXY_DEFINITIONS)
endif (MINGW)

set(BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Build type")

option(OPT_XTINYPROXY_ENABLE "Include the X-Tinyproxy header" ON)
option(OPT_FILTER_ENABLE "Enable filtering of domains/URLS" ON)
option(OPT_UPSTREAM_SUPPORT "Enable upstream proxying" ON)
option(OPT_REVERSE_SUPPORT "Enable reverse proxying" ON)
option(OPT_TRANSPARENT_PROXY "Enable transparent proxying code" ON)

proxy_option_to_definition(OPT_XTINYPROXY_ENABLE XTINYPROXY_ENABLE PROXY_DEFINITIONS)
proxy_option_to_definition(OPT_FILTER_ENABLE FILTER_ENABLE PROXY_DEFINITIONS)
proxy_option_to_definition(OPT_UPSTREAM_SUPPORT UPSTREAM_SUPPORT PROXY_DEFINITIONS)
proxy_option_to_definition(OPT_REVERSE_SUPPORT REVERSE_SUPPORT PROXY_DEFINITIONS)
proxy_option_to_definition(OPT_TRANSPARENT_PROXY TRANSPARENT_PROXY PROXY_DEFINITIONS)

include_directories(include)

add_subdirectory(src)
