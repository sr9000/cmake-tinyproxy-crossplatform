include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFile)

if (MINGW)
    get_filename_component(PROXY_MINGW_PREFIX "$ENV{MINGW_PREFIX}/$ENV{MINGW_CHOST}" REALPATH BASE_DIR "")
endif()

function(global_proxy_list LISTNAME)
    set(${LISTNAME} "" CACHE INTERNAL "${LISTNAME}")
endfunction(global_proxy_list)

function(global_proxy_list_append LISTNAME NEWELEMENT)
    set(_NEW_LIST)

    # double referencing
    # LISTNAME contains `name` of global list
    list(APPEND _NEW_LIST "${${LISTNAME}}")
    list(APPEND _NEW_LIST "${NEWELEMENT}")

    set(${LISTNAME} "${_NEW_LIST}" CACHE INTERNAL "${LISTNAME}")
endfunction(global_proxy_list_append)

function(proxy_require_lib_with_func FUNC_NAME LIB_NAME VAR_NAME LIBS_LISTNAME DEFS_LISTNAME)
    set(CMAKE_REQUIRED_LIBRARIES "${LIB_NAME}")
    check_function_exists("${FUNC_NAME}" "_FUNC_IN_LIB_GLOBAL_${FUNC_NAME}")
    if ("${_FUNC_IN_LIB_GLOBAL_${FUNC_NAME}}")
        set(IS_EXISTS TRUE)
    else ()
        check_library_exists("${LIB_NAME}" "${FUNC_NAME}" "" IS_EXISTS)
    endif()
    set(CMAKE_REQUIRED_LIBRARIES)

    set(${VAR_NAME} ${IS_EXISTS} CACHE INTERNAL "${VAR_NAME}")
    if (${IS_EXISTS})
        global_proxy_list_append(${LIBS_LISTNAME} ${LIB_NAME})
        global_proxy_list_append(${DEFS_LISTNAME} ${VAR_NAME})
    else ()
        message(SEND_ERROR "Cannot find library \"${LIB_NAME}\" with function \"${FUNC_NAME}\"")
    endif ()
endfunction(proxy_require_lib_with_func)

function(proxy_option_to_definition _OPTION _DEFINITION DEFS_LISTNAME)
    if (${${_OPTION}})
        global_proxy_list_append(${DEFS_LISTNAME} ${_DEFINITION})
    endif()
endfunction(proxy_option_to_definition)

function(proxy_require_header HFILE VAR_NAME DEFS_LISTNAME)
    set(IS_EXISTS FALSE)    
    if (CYGWIN)
        if (EXISTS "/usr/include/${HFILE}")
            set(IS_EXISTS TRUE)
        endif()
    elseif(MINGW)
        if (EXISTS "${PROXY_MINGW_PREFIX}/include/${HFILE}")
            set(IS_EXISTS TRUE)
        endif()
    else ()
        check_include_file(${HFILE} IS_EXISTS)
    endif()

    set(${VAR_NAME} ${IS_EXISTS} CACHE INTERNAL "${VAR_NAME}")
    if (${IS_EXISTS})
        global_proxy_list_append(${DEFS_LISTNAME} ${VAR_NAME})
    else ()
        message(SEND_ERROR "Cannot find header \"${HFILE}\"")
    endif ()
endfunction(proxy_require_header)


