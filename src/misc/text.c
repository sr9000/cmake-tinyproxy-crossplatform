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

/* The functions included here are useful for text manipulation.  They
 * replace or augment the standard C string library.  These functions
 * are either safer replacements, or they provide services not included
 * with the standard C string library.
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include "misc/text.h"

// Function API taken from OpenBSD. Like strncpy(), but does not 0 fill the buffer, and always NULL
// terminates the buffer. size is the size of the destination buffer.
size_t safe_string_copy(char *dst, const char *src, size_t size)
{
  size_t len = strlen(src);
  size_t ret = len;

  if (len >= size)
    len = size - 1;

  memcpy(dst, src, len);
  dst[len] = '\0';

  return ret;
}

// Function API taken from OpenBSD. Like strncat(), but does not 0 fill the buffer, and always NULL
// terminates the buffer. size is the length of the buffer, which should be one more than the
// maximum resulting string length.
size_t safe_string_append(char *dst, const char *src, size_t size)
{
  size_t len1 = strlen(dst);
  size_t len2 = strlen(src);
  size_t ret = len1 + len2;

  if (len1 + len2 >= size)
    len2 = size - len1 - 1;
  if (len2 > 0)
  {
    memcpy(dst + len1, src, len2);
    dst[len1 + len2] = '\0';
  }

  return ret;
}

// Removes any new-line or carriage-return characters from the end of the string. This function is
// named after the same function in Perl. "length" should be the number of characters in the buffer,
// not including the trailing NULL.
//
// Returns the number of characters removed from the end of the string. A negative return value
// indicates an error.
ssize_t trim_ending_newlines(char *buffer, size_t length)
{
  size_t chars;

  assert(buffer != NULL);
  assert(length > 0);

  // Make sure the arguments are valid
  if (buffer == NULL)
    return -EFAULT;
  if (length < 1)
    return -ERANGE;

  chars = 0;

  --length;
  while (buffer[length] == '\r' || buffer[length] == '\n')
  {
    buffer[length] = '\0';
    chars++;

    // Stop once we get to zero to prevent wrap-around
    if (length-- == 0)
      break;
  }

  return chars;
}
