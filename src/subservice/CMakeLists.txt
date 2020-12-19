
add_library(tinyproxy_log log.c "${TINYPROXY_LOG_HEADERS}")
target_link_libraries(tinyproxy_log tinyproxy_conf_help tinyproxy_heap)

add_library(tinyproxy_anon anonymous.c "${TINYPROXY_ANON_HEADERS}")
target_link_libraries(tinyproxy_anon
        tinyproxy_log
        tinyproxy_conf_help
        tinyproxy_heap)

add_library(tinyproxy_net network.c "${TINYPROXY_NET_HEADERS}")
target_link_libraries(tinyproxy_net tinyproxy_heap)

add_library(tinyproxy_acl acl.c "${TINYPROXY_ACL_HEADERS}")
target_link_libraries(tinyproxy_acl
        tinyproxy_log
        tinyproxy_conf_help
        tinyproxy_list
        tinyproxy_net
        tinyproxy_heap)

add_library(tinyproxy_auth basicauth.c "${TINYPROXY_AUTH_HEADERS}")
target_link_libraries(tinyproxy_auth
        tinyproxy_log
        tinyproxy_conf_help
        tinyproxy_list
        tinyproxy_heap
        tinyproxy_base64)

install(TARGETS
        tinyproxy_log
        tinyproxy_anon
        tinyproxy_net
        tinyproxy_acl
        tinyproxy_auth
        EXPORT Tinyproxy)
