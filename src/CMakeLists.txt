
add_compile_definitions(
        PACKAGE_BUGREPORT="https://tinyproxy.github.io/"
        PACKAGE_NAME="Tinyproxy"
        PACKAGE_STRING="Tinyproxy ${REAL_VERSION}"
        PACKAGE_TARNAME="tinyproxy"
        PACKAGE="tinyproxy"
        PACKAGE_URL="https://tinyproxy.github.io/"
        PACKAGE_VERSION="${REAL_VERSION}"
        VERSION="${REAL_VERSION}"
        TINYPROXY_VERSION="${REAL_VERSION}"
        TINYPROXY_STATHOST="tinyproxy.stats"
        ${PROXY_DEFINITIONS}
)

add_subdirectory(misc)

set(TINYPROXY_SOURCES
        acl.c
        anonymous.c
        basicauth.c
        buffer.c
        child.c
        conf.c
        connect-ports.c
        conns.c
        daemon.c
        html-error.c
        http-message.c
        log.c
        network.c
        reqs.c
        sock.c
        stats.c
        upstream.c
        utils.c
        )

if (OPT_FILTER_ENABLE)
    list(APPEND TINYPROXY_SOURCES filter.c)
endif (OPT_FILTER_ENABLE)

if (OPT_REVERSE_SUPPORT)
    list(APPEND TINYPROXY_SOURCES reverse-proxy.c)
endif (OPT_REVERSE_SUPPORT)

if (OPT_TRANSPARENT_PROXY)
    list(APPEND TINYPROXY_SOURCES transparent-proxy.c)
endif (OPT_TRANSPARENT_PROXY)

add_executable(tinyproxy
        main.c
        ${TINYPROXY_SOURCES}
        "${TINYPROXY_HEADERS}"
        )

target_link_libraries(tinyproxy
        tinyproxy_base64
        tinyproxy_heap
        tinyproxy_list
        tinyproxy_hashmap
        tinyproxy_text
        tinyproxy_file_api
        ${PROXY_LIBRARIES}
        )
