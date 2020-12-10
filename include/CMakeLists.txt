include(HelpProxyMisc)

add_subdirectory(misc)

set(HEADERS
        config/conf.h
        config/conf_log.h
        tinyproxy.h
        reverse-proxy.h
        daemon.h
        main.h
        html-error.h
        http-message.h
        custom_sysexits.h
        transparent-proxy.h
        acl.h
        upstream.h
        sock.h
        common.h
        child.h
        anonymous.h
        stats.h
        conns.h
        buffer.h
        connect-ports.h
        filter.h
        reqs.h
        network.h
        utils.h
        basicauth.h)

proxy_global_header_absolute_paths(TINYPROXY_HEADERS "${HEADERS}")
