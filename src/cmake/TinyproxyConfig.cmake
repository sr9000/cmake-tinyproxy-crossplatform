# include("${CMAKE_CURRENT_LIST_DIR}/TinyproxyTargets.cmake")
# ...
# (compute PREFIX relative to file location)
# ...
set(PREFIX ${CMAKE_CURRENT_LIST_DIR}/../../..)
set(Tinyproxy_INCLUDE_DIRS ${PREFIX}/include)
set(Tinyproxy_LIBRARIES
        ${PREFIX}/lib/libtinyproxy_lib.a
        ${PREFIX}/lib/libtinyproxy_acl.a
        ${PREFIX}/lib/libtinyproxy_anon.a
        ${PREFIX}/lib/libtinyproxy_net.a
        ${PREFIX}/lib/libtinyproxy_log.a
        ${PREFIX}/lib/libtinyproxy_conf_help.a
        ${PREFIX}/lib/libtinyproxy_file_api.a
        ${PREFIX}/lib/libtinyproxy_text.a
        ${PREFIX}/lib/libtinyproxy_hashmap.a
        ${PREFIX}/lib/libtinyproxy_list.a
        ${PREFIX}/lib/libtinyproxy_heap.a
        ${PREFIX}/lib/libtinyproxy_base64.a
        )
