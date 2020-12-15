
add_library(tinyproxy_log log.c "${TINYPROXY_LOG_HEADERS}")
target_link_libraries(tinyproxy_log tinyproxy_conf_help)

add_library(tinyproxy_anon anonymous.c "${TINYPROXY_ANON_HEADERS}")
target_link_libraries(tinyproxy_anon
        tinyproxy_log
        tinyproxy_conf_help)

add_library(tinyproxy_acl acl.c "${TINYPROXY_ACL_HEADERS}")
target_link_libraries(tinyproxy_acl
        tinyproxy_log
        tinyproxy_conf_help
        tinyproxy_list)
