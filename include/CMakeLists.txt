include(HelpProxyMisc)

proxy_require_header("sys/ioctl.h" HAVE_SYS_IOCTL_H PROXY_DEFINITIONS)
proxy_require_header("alloca.h" HAVE_ALLOCA_H PROXY_DEFINITIONS)
proxy_require_header("memory.h" HAVE_MEMORY_H PROXY_DEFINITIONS)
proxy_require_header("malloc.h" HAVE_MALLOC_H PROXY_DEFINITIONS)
proxy_require_header("malloc.h" HAVE_MALLOC PROXY_DEFINITIONS)
proxy_require_header("sysexits.h" HAVE_SYSEXITS_H PROXY_DEFINITIONS)
proxy_require_header("values.h" HAVE_VALUES_H PROXY_DEFINITIONS)
