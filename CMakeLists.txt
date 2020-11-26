cmake_minimum_required(VERSION 3.17)
project(cmake_tinyproxy C)

set(CMAKE_C_STANDARD 99)

set(MAJOR_VERSION 1)
set(MINOR_VERSION 10)
set(MICRO_VERSION 0)
set(REAL_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION})


# Custom cmake scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(HelpProxyMisc)

global_proxy_list(PROXY_DEFINITIONS)
global_proxy_list(PROXY_LIBRARIES)

proxy_require_lib_with_func(inet_aton resolv HAVE_LIBRESOLV PROXY_LIBRARIES PROXY_DEFINITIONS)
proxy_require_lib_with_func(gethostbyname nsl HAVE_LIBNSL PROXY_LIBRARIES PROXY_DEFINITIONS)

set(BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Build type")
option(OPT_XTINYPROXY_ENABLE "Include the X-Tinyproxy header" ON)
option(OPT_FILTER_ENABLE "Enable filtering of domains/URLS" ON)
# ADDITIONAL_OBJECTS="$ADDITIONAL_OBJECTS filter.o"

option(OPT_UPSTREAM_SUPPORT "Enable upstream proxying" ON)
option(OPT_REVERSE_SUPPORT "Enable reverse proxying" ON)
# ADDITIONAL_OBJECTS="$ADDITIONAL_OBJECTS reverse-proxy.o"

option(OPT_TRANSPARENT_PROXY "Enable transparent proxying code" ON)
# ADDITIONAL_OBJECTS="$ADDITIONAL_OBJECTS transparent-proxy.o"

proxy_option_to_definition(OPT_XTINYPROXY_ENABLE XTINYPROXY_ENABLE PROXY_DEFINITIONS)
proxy_option_to_definition(OPT_FILTER_ENABLE FILTER_ENABLE PROXY_DEFINITIONS)
proxy_option_to_definition(OPT_UPSTREAM_SUPPORT UPSTREAM_SUPPORT PROXY_DEFINITIONS)
proxy_option_to_definition(OPT_REVERSE_SUPPORT REVERSE_SUPPORT PROXY_DEFINITIONS)
proxy_option_to_definition(OPT_TRANSPARENT_PROXY TRANSPARENT_PROXY PROXY_DEFINITIONS)

include_directories(include)

add_subdirectory(include)
add_subdirectory(src)