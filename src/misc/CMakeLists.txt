
add_library(tinyproxy_base64 base64.c "${TINYPROXY_BASE64_HEADERS}")

add_library(tinyproxy_heap heap.c "${TINYPROXY_HEAP_HEADERS}")

add_library(tinyproxy_list list.c "${TINYPROXY_LIST_HEADERS}")
target_link_libraries(tinyproxy_list tinyproxy_heap)

add_library(tinyproxy_hashmap hashmap.c "${TINYPROXY_HASHMAP_HEADERS}")
target_link_libraries(tinyproxy_hashmap tinyproxy_heap)

add_library(tinyproxy_text text.c "${TINYPROXY_TEXT_HEADERS}")

add_library(tinyproxy_file_api file_api.c "${TINYPROXY_FILE_API_HEADERS}")

install(TARGETS
        tinyproxy_base64
        tinyproxy_heap
        tinyproxy_list
        tinyproxy_hashmap
        tinyproxy_text
        tinyproxy_file_api
        EXPORT Tinyproxy)
