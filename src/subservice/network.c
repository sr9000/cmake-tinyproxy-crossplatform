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

/* The functions found here are used for communicating across a
 * network.  They include both safe reading and writing (which are
 * the basic building blocks) along with two functions for
 * easily reading a line of text from the network, and a function
 * to write an arbitrary amount of data to the network.
 */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "misc/heap.h"
#include "subservice/network.h"

#ifdef HAVE_WSOCK32

int initialize_winsock()
{
  TRACE_CALL(initialize_winsock);
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
  {
    TRACE_RETURN_X(-1, "Initialising Winsock Failed. Error Code : %d", WSAGetLastError());
  }
  TRACE_RETURN_X(0);
}
#else
int initialize_winsock()
{
  return 0;
}
#endif

#ifdef MINGW
// Author: Paul Vixie, 1996.
// https://stackoverflow.com/questions/15370033/how-to-use-inet-pton-with-the-mingw-compiler
int mingw_inet_pton4(const char *src, struct in_addr *dst)
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
int mingw_inet_pton6(const char *src, struct in_addr *dst)
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
int mingw_inet_pton(int af, const char *src, struct in_addr *dst)
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
#endif // MINGW

/*
 * Write the buffer to the socket. If an EINTR occurs, pick up and try
 * again. Keep sending until the buffer has been sent.
 */
ssize_t safe_write(int fd, const void *buf, size_t count)
{
  ssize_t len;
  size_t bytestosend;
  const char *buffer = buf;

  assert(fd >= 0);
  assert(buffer != NULL);
  assert(count > 0);

  bytestosend = count;

  while (1)
  {
    len = send(fd, buffer, bytestosend, MSG_NOSIGNAL);

    if (len < 0)
    {
      if (errno == EINTR)
        continue;
      else
        return -errno;
    }

    if ((size_t)len == bytestosend)
      break;

    buffer += len;
    bytestosend -= len;
  }

  return count;
}

/*
 * Matched pair for safe_write(). If an EINTR occurs, pick up and try
 * again.
 */
ssize_t safe_read(int fd, void *buffer, size_t count)
{
  ssize_t len;

  do
  {
    len = read(fd, buffer, count);
  } while (len < 0 && errno == EINTR);

  return len;
}

/*
 * Send a "message" to the file descriptor provided. This handles the
 * differences between the various implementations of vsnprintf. This code
 * was basically stolen from the snprintf() man page of Debian Linux
 * (although I did fix a memory leak. :)
 */
int write_message(int fd, const char *fmt, ...)
{
  ssize_t n;
  size_t size = (1024 * 8); /* start with 8 KB and go from there */
  char *buf, *tmpbuf;
  va_list ap;

  if ((buf = (char *)safemalloc(size)) == NULL)
    return -1;

  while (1)
  {
    va_start(ap, fmt);
    n = vsnprintf(buf, size, fmt, ap);
    va_end(ap);

    /* If that worked, break out so we can send the buffer */
    if (n > -1 && (size_t)n < size)
      break;

    /* Else, try again with more space */
    if (n > -1)
      /* precisely what is needed (glibc2.1) */
      size = n + 1;
    else
      /* twice the old size (glibc2.0) */
      size *= 2;

    if ((tmpbuf = (char *)saferealloc(buf, size)) == NULL)
    {
      safefree(buf);
      return -1;
    }
    else
      buf = tmpbuf;
  }

  if (safe_write(fd, buf, n) < 0)
  {
    safefree(buf);
    return -1;
  }

  safefree(buf);
  return 0;
}

/*
 * Read in a "line" from the socket. It might take a few loops through
 * the read sequence. The full string is allocate off the heap and stored
 * at the whole_buffer pointer. The caller needs to free the memory when
 * it is no longer in use. The returned line is NULL terminated.
 *
 * Returns the length of the buffer on success (not including the NULL
 * termination), 0 if the socket was closed, and -1 on all other errors.
 */
#define SEGMENT_LEN           (512)
#define MAXIMUM_BUFFER_LENGTH (128 * 1024)
ssize_t readline(int fd, char **whole_buffer)
{
  ssize_t whole_buffer_len;
  char buffer[SEGMENT_LEN];
  char *ptr;

  ssize_t ret;
  ssize_t diff;

  struct read_lines_s
  {
    char *data;
    size_t len;
    struct read_lines_s *next;
  };
  struct read_lines_s *first_line, *line_ptr;

  first_line = (struct read_lines_s *)safecalloc(sizeof(struct read_lines_s), 1);
  if (!first_line)
    return -ENOMEM;

  line_ptr = first_line;

  whole_buffer_len = 0;
  for (;;)
  {
    ret = recv(fd, buffer, SEGMENT_LEN, MSG_PEEK);
    if (ret <= 0)
      goto CLEANUP;

    ptr = (char *)memchr(buffer, '\n', ret);
    if (ptr)
      diff = ptr - buffer + 1;
    else
      diff = ret;

    whole_buffer_len += diff;

    /*
     * Don't allow the buffer to grow without bound. If we
     * get to more than MAXIMUM_BUFFER_LENGTH close.
     */
    if (whole_buffer_len > MAXIMUM_BUFFER_LENGTH)
    {
      ret = -ERANGE;
      goto CLEANUP;
    }

    line_ptr->data = (char *)safemalloc(diff);
    if (!line_ptr->data)
    {
      ret = -ENOMEM;
      goto CLEANUP;
    }

    ret = recv(fd, line_ptr->data, diff, 0);
    if (ret == -1)
    {
      goto CLEANUP;
    }

    line_ptr->len = diff;

    if (ptr)
    {
      line_ptr->next = NULL;
      break;
    }

    line_ptr->next = (struct read_lines_s *)safecalloc(sizeof(struct read_lines_s), 1);
    if (!line_ptr->next)
    {
      ret = -ENOMEM;
      goto CLEANUP;
    }
    line_ptr = line_ptr->next;
  }

  *whole_buffer = (char *)safemalloc(whole_buffer_len + 1);
  if (!*whole_buffer)
  {
    ret = -ENOMEM;
    goto CLEANUP;
  }

  *(*whole_buffer + whole_buffer_len) = '\0';

  whole_buffer_len = 0;
  line_ptr = first_line;
  while (line_ptr)
  {
    memcpy(*whole_buffer + whole_buffer_len, line_ptr->data, line_ptr->len);
    whole_buffer_len += line_ptr->len;

    line_ptr = line_ptr->next;
  }

  ret = whole_buffer_len;

CLEANUP:
  do
  {
    line_ptr = first_line->next;
    if (first_line->data)
      safefree(first_line->data);
    safefree(first_line);
    first_line = line_ptr;
  } while (first_line);

  return ret;
}

/*
 * Convert the network address into either a dotted-decimal or an IPv6
 * hex string.
 */
const char *get_ip_string(struct sockaddr *sa, char *buf, size_t buflen)
{
  const char *result;

  assert(sa != NULL);
  assert(buf != NULL);
  assert(buflen != 0);
  buf[0] = '\0'; /* start with an empty string */

  switch (sa->sa_family)
  {
  case AF_INET: {
    struct sockaddr_in *sa_in = (struct sockaddr_in *)sa;

    result = inet_ntop(AF_INET, &sa_in->sin_addr, buf, buflen);
    break;
  }
  case AF_INET6: {
    struct sockaddr_in6 *sa_in6 = (struct sockaddr_in6 *)sa;

    result = inet_ntop(AF_INET6, &sa_in6->sin6_addr, buf, buflen);
    break;
  }
  default:
    /* no valid family */
    return NULL;
  }

  return result;
}

/*
 * Convert a numeric character string into an IPv6 network address
 * (in binary form.)  The function works just like inet_pton(), but it
 * will accept both IPv4 and IPv6 numeric addresses.
 *
 * Returns the same as inet_pton().
 */

int full_inet_pton(const char *ip, void *dst)
{
  char buf[24], tmp[24]; /* IPv4->IPv6 = ::FFFF:xxx.xxx.xxx.xxx\0 */
  int n;

  assert(ip != NULL && strlen(ip) != 0);
  assert(dst != NULL);

  /*
   * Check if the string is an IPv4 numeric address.  We use the
   * older inet_aton() call since it handles more IPv4 numeric
   * address formats.
   */
  n = proxy_inet_aton(ip, (struct in_addr *)dst);
  if (n == 0)
  {
    /*
     * Simple case: "ip" wasn't an IPv4 numeric address, so
     * try doing the conversion as an IPv6 address.  This
     * will either succeed or fail, but we can't do any
     * more processing anyway.
     */
    return proxy_inet_pton(AF_INET6, ip, dst);
  }

  /*
   * "ip" was an IPv4 address, so we need to convert it to
   * an IPv4-mapped IPv6 address and do the conversion
   * again to get the IPv6 network structure.
   *
   * We convert the IPv4 binary address back into the
   * standard dotted-decimal format using inet_ntop()
   * so we can be sure that inet_pton will accept the
   * full string.
   */
  snprintf(buf, sizeof(buf), "::ffff:%s", inet_ntop(AF_INET, dst, tmp, sizeof(tmp)));
  return proxy_inet_pton(AF_INET6, buf, dst);
}
