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

#include <regex.h>  // mingw +libsystre
#include <signal.h> // mingw +ifdefs +winapi

#ifdef MINGW
#define LOG_EMERG   0 // <syslog.h>
#define LOG_ALERT   1 // <syslog.h>
#define LOG_CRIT    2 // <syslog.h>
#define LOG_ERR     3 // <syslog.h>
#define LOG_WARNING 4 // <syslog.h>
#define LOG_NOTICE  5 // <syslog.h>
#define LOG_INFO    6 // <syslog.h>
#define LOG_DEBUG   7 // <syslog.h>

#define MAP_FAILED NULL // <sys/mman.h>
#else
#include <grp.h>      // skip group and user check on windows
#include <pwd.h>      // skip group and user check on windows
#include <sys/mman.h> // windows threads use common address space
#include <sys/wait.h> // do not use waitpid on windows
#include <syslog.h>   // only uses defines LOG_${LEVEL}
#endif                /* MINGW */

#ifdef HAVE_WSOCK32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0601 // Windows 7+

// winsock2 and ws2tcpip first
#include <winsock2.h> // order does matter
#include <ws2tcpip.h> // order does matter

// Ws2ipdef second
#include <Ws2ipdef.h> // order does matter

// windows last
#include <windows.h> // order does matter
typedef unsigned long in_addr_t;
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#define closesocket close
#endif

#include "custom_sysexits.h"

/*
 * If MSG_NOSIGNAL is not defined, define it to be zero so that it doesn't
 * cause any problems.
 */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL (0)
#endif

#ifndef SHUT_RD /* these three Posix.1g names are quite new */
#define SHUT_RD   0 /* shutdown for reading */
#define SHUT_WR   1 /* shutdown for writing */
#define SHUT_RDWR 2 /* shutdown for reading and writing */
#endif

#define MAXLISTEN 1024 /* Max number of connections */

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

#ifdef MINGW
// Author: Paul Vixie, 1996.
// https://stackoverflow.com/questions/15370033/how-to-use-inet-pton-with-the-mingw-compiler
#define MINGW_NS_INADDRSZ  4
#define MINGW_NS_IN6ADDRSZ 16
#define MINGW_NS_INT16SZ   2
static int mingw_inet_pton4(const char *src, struct in_addr *dst)
{
  uint8_t tmp[MINGW_NS_INADDRSZ], *tp;

  int saw_digit = 0;
  int octets = 0;
  *(tp = tmp) = 0;

  int ch;
  while ((ch = *src++) != '\0')
  {
    if (ch >= '0' && ch <= '9')
    {
      uint32_t n = *tp * 10 + (ch - '0');

      if (saw_digit && *tp == 0)
        return 0;

      if (n > 255)
        return 0;

      *tp = n;
      if (!saw_digit)
      {
        if (++octets > 4)
          return 0;
        saw_digit = 1;
      }
    }
    else if (ch == '.' && saw_digit)
    {
      if (octets == 4)
        return 0;
      *++tp = 0;
      saw_digit = 0;
    }
    else
      return 0;
  }
  if (octets < 4)
    return 0;

  memcpy(dst, tmp, MINGW_NS_INADDRSZ);

  return 1;
}
static int mingw_inet_pton6(const char *src, struct in_addr *dst)
{
  static const char xdigits[] = "0123456789abcdef";
  uint8_t tmp[MINGW_NS_IN6ADDRSZ];

  uint8_t *tp = (uint8_t *)memset(tmp, '\0', MINGW_NS_IN6ADDRSZ);
  uint8_t *endp = tp + MINGW_NS_IN6ADDRSZ;
  uint8_t *colonp = NULL;

  /* Leading :: requires some special handling. */
  if (*src == ':')
  {
    if (*++src != ':')
      return 0;
  }

  const char *curtok = src;
  int saw_xdigit = 0;
  uint32_t val = 0;
  int ch;
  while ((ch = tolower(*src++)) != '\0')
  {
    const char *pch = strchr(xdigits, ch);
    if (pch != NULL)
    {
      val <<= 4;
      val |= (pch - xdigits);
      if (val > 0xffff)
        return 0;
      saw_xdigit = 1;
      continue;
    }
    if (ch == ':')
    {
      curtok = src;
      if (!saw_xdigit)
      {
        if (colonp)
          return 0;
        colonp = tp;
        continue;
      }
      else if (*src == '\0')
      {
        return 0;
      }
      if (tp + MINGW_NS_INT16SZ > endp)
        return 0;
      *tp++ = (uint8_t)(val >> 8) & 0xff;
      *tp++ = (uint8_t)val & 0xff;
      saw_xdigit = 0;
      val = 0;
      continue;
    }
    if (ch == '.' && ((tp + MINGW_NS_INADDRSZ) <= endp) &&
        mingw_inet_pton4(curtok, (struct in_addr *)tp) > 0)
    {
      tp += MINGW_NS_INADDRSZ;
      saw_xdigit = 0;
      break; /* '\0' was seen by inet_pton4(). */
    }
    return 0;
  }
  if (saw_xdigit)
  {
    if (tp + MINGW_NS_INT16SZ > endp)
      return 0;
    *tp++ = (uint8_t)(val >> 8) & 0xff;
    *tp++ = (uint8_t)val & 0xff;
  }
  if (colonp != NULL)
  {
    /*
     * Since some memmove()'s erroneously fail to handle
     * overlapping regions, we'll do the shift by hand.
     */
    const int n = tp - colonp;

    if (tp == endp)
      return 0;

    for (int i = 1; i <= n; i++)
    {
      endp[-i] = colonp[n - i];
      colonp[n - i] = 0;
    }
    tp = endp;
  }
  if (tp != endp)
    return 0;

  memcpy(dst, tmp, MINGW_NS_IN6ADDRSZ);

  return 1;
}
static int mingw_inet_pton(int af, const char *src, struct in_addr *dst)
{
  switch (af)
  {
  case AF_INET:
    return mingw_inet_pton4(src, dst);
  case AF_INET6:
    return mingw_inet_pton6(src, dst);
  default:
    return -1;
  }
}
#define proxy_inet_aton mingw_inet_pton4
#define proxy_inet_pton mingw_inet_pton

#else /* MINGW */

#define proxy_inet_aton inet_aton
#define proxy_inet_pton inet_pton

#endif /* MINGW */

#endif
