
add_library(tinyproxy_base64 base64.c)

add_library(tinyproxy_heap heap.c)

add_library(tinyproxy_list list.c)
target_link_libraries(tinyproxy_list tinyproxy_heap)

add_library(tinyproxy_hashmap hashmap.c)
target_link_libraries(tinyproxy_hashmap tinyproxy_heap)

add_library(tinyproxy_text text.c)
