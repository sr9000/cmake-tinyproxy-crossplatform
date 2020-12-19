
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
add_subdirectory(config)
add_subdirectory(subservice)

set(TINYPROXY_SOURCES
        buffer.c
        child.c
        config/conf.c
        connect-ports.c
        conns.c
        daemon.c
        html-error.c
        http-message.c
        reqs.c
        sock.c
        stats.c
        upstream.c
        utils.c
        tinyproxy.c)

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
        websockets
        tinyproxy_base64
        tinyproxy_heap
        tinyproxy_list
        tinyproxy_hashmap
        tinyproxy_text
        tinyproxy_file_api
        tinyproxy_conf_help
        tinyproxy_log
        tinyproxy_anon
        tinyproxy_net
        tinyproxy_acl
        tinyproxy_auth
        ${PROXY_LIBRARIES}
        )

#add_library(tinyproxy_lib
#        main.c
#        ${TINYPROXY_SOURCES}
#        "${TINYPROXY_HEADERS}"
#        )
#
#target_link_libraries(tinyproxy_lib
#        websockets
#        tinyproxy_base64
#        tinyproxy_heap
#        tinyproxy_list
#        tinyproxy_hashmap
#        tinyproxy_text
#        tinyproxy_file_api
#        tinyproxy_conf_help
#        tinyproxy_log
#        tinyproxy_anon
#        tinyproxy_net
#        tinyproxy_acl
#        ${PROXY_LIBRARIES}
#        )
#
#set_target_properties(tinyproxy_lib PROPERTIES PUBLIC_HEADER ../include/tinyproxy_lib.h)
#install(TARGETS tinyproxy_lib EXPORT Tinyproxy PUBLIC_HEADER)
#
#include(CMakePackageConfigHelpers)
#write_basic_package_version_file(
#        "${CMAKE_CURRENT_BINARY_DIR}/Tinyproxy/TinyproxyConfigVersion.cmake"
#        VERSION 1.0.0
#        COMPATIBILITY AnyNewerVersion
#)
#
#export(EXPORT Tinyproxy
#        FILE "${CMAKE_CURRENT_BINARY_DIR}/Tinyproxy/TinyproxyTargets.cmake"
#        )
#
#configure_file(cmake/TinyproxyConfig.cmake
#        "${CMAKE_CURRENT_BINARY_DIR}/Tinyproxy/TinyproxyConfig.cmake"
#        COPYONLY
#        )
#
#set(ConfigPackageLocation lib/cmake/Tinyproxy)
#install(EXPORT Tinyproxy
#        FILE TinyproxyTargets.cmake
#        DESTINATION ${ConfigPackageLocation})
#install(FILES
#        cmake/TinyproxyConfig.cmake
#        "${CMAKE_CURRENT_BINARY_DIR}/Tinyproxy/TinyproxyConfigVersion.cmake"
#        DESTINATION ${ConfigPackageLocation}
#        COMPONENT Devel
#)
#
#add_custom_target(fake_install
#        ALL
#        "${CMAKE_COMMAND}"
#        -D CMAKE_INSTALL_PREFIX:string=${CMAKE_BINARY_DIR}/installed/tinyproxy-1
#        -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake"
#        DEPENDS tinyproxy_lib)
