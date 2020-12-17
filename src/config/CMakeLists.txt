add_library(tinyproxy_conf_help
        conf_anon.c
        conf_log.c
        conf_acl.c
        "${TINYPROXY_CONF_ANON_HEADERS}")
target_link_libraries(tinyproxy_conf_help tinyproxy_heap)

install(TARGETS tinyproxy_conf_help EXPORT Tinyproxy)
