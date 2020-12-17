include(HelpProxyMisc)

add_subdirectory(misc)

set(HEADERS
        config/conf.h
        tinyproxy.h
        tinyproxy_lib.h
        reverse-proxy.h
        daemon.h
        main.h
        html-error.h
        http-message.h
        custom_sysexits.h
        transparent-proxy.h
        upstream.h
        sock.h
        common.h
        child.h
        stats.h
        conns.h
        buffer.h
        connect-ports.h
        filter.h
        reqs.h
        utils.h
        basicauth.h)

proxy_global_header_absolute_paths(TINYPROXY_HEADERS "${HEADERS}")
