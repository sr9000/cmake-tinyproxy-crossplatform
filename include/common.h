/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 2002 Robert James Kaes <rjkaes@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* This file groups all the headers required throughout the tinyproxy
 * system.  All this information use to be in the "main.h" header,
 * but various other "libraries" in the program need the same information,
 * without the tinyproxy specific defines.
 */

#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*
 * Include standard headers which are used through-out tinyproxy
 */

/* standard C headers - we can safely assume they exist. */
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* standard POSIX headers - they need to be there as well. */
#include <assert.h>    // mingw +
#include <errno.h>     // mingw +
#include <fcntl.h>     // mingw +
#include <inttypes.h>  // mingw +
#include <stdarg.h>    // mingw +
#include <strings.h>   // mingw +
#include <sys/stat.h>  // mingw +
#include <sys/types.h> // mingw +
#include <time.h>      // mingw +
#include <wchar.h>     // mingw +
#include <wctype.h>    // mingw +


#include <signal.h> // mingw +ifdefs +winapi

#ifdef MINGW
#define MAP_FAILED NULL // <sys/mman.h>
#else
#include <grp.h>      // skip group and user check on windows
#include <pwd.h>      // skip group and user check on windows
#include <sys/mman.h> // windows threads use common address space
#include <sys/wait.h> // do not use waitpid on windows
#endif                /* MINGW */

#include "custom_sysexits.h"



/*
 * SunOS doesn't have INADDR_NONE defined.
 */
#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif

/* Define boolean values */
#ifndef FALSE
#define FALSE 0
#define TRUE  (!FALSE)
#endif

/* Useful function macros */
#if !defined(min) || !defined(max)
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif



#endif
