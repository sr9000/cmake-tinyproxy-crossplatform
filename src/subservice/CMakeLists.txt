
add_library(tinyproxy_log log.c "${TINYPROXY_LOG_HEADERS}")
target_link_libraries(tinyproxy_log
        tinyproxy_heap
        tinyproxy_file_api)
