/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 2002, 2004 Robert James Kaes <rjkaes@users.sourceforge.net>
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

#ifndef TINYPROXY_NETWORK_H
#define TINYPROXY_NETWORK_H

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

static WSADATA wsa;
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#define closesocket close
#endif

#ifdef MINGW
// Author: Paul Vixie, 1996.
// https://stackoverflow.com/questions/15370033/how-to-use-inet-pton-with-the-mingw-compiler
#define MINGW_NS_INADDRSZ  4
#define MINGW_NS_IN6ADDRSZ 16
#define MINGW_NS_INT16SZ   2

extern int mingw_inet_pton4(const char *src, struct in_addr *dst);
extern int mingw_inet_pton6(const char *src, struct in_addr *dst);
extern int mingw_inet_pton(int af, const char *src, struct in_addr *dst);

#define proxy_inet_aton mingw_inet_pton4
#define proxy_inet_pton mingw_inet_pton

#else /* MINGW */

#define proxy_inet_aton inet_aton
#define proxy_inet_pton inet_pton

#endif /* MINGW */

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

#ifdef MINGW
int readsocket(SOCKET s, char *buf, int len)
{
  return recv(s, buf, len, 0);
}
int writesocket(SOCKET s, const char *buf, int len, int _ignore)
{
  return send(s, buf, len, 0);
}
#else /* MINGW */
// todo readsocket == recv?
#define readsocket  read
#define writesocket send
#endif /* MINGW */

#define MAXLISTEN 1024 /* Max number of connections */

extern int initialize_winsock();
extern ssize_t safe_write(int fd, const void *buf, size_t count);
extern ssize_t safe_read(int fd, void *buf, size_t count);

extern int write_message(int fd, const char *fmt, ...);
extern ssize_t readline(int fd, char **whole_buffer);

extern const char *get_ip_string(struct sockaddr *sa, char *buf, size_t len);
extern int full_inet_pton(const char *ip, void *dst);

#endif // TINYPROXY_NETWORK_H
