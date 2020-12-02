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

#ifndef TINYPROXY_TEXT_H
#define TINYPROXY_TEXT_H

#include <stddef.h>
#include <sys/types.h>

// Function API taken from OpenBSD. Like strncpy(), but does not 0 fill the buffer, and always NULL
// terminates the buffer. size is the size of the destination buffer.
extern size_t safe_string_append(char *dst, const char *src, size_t size);

// Function API taken from OpenBSD. Like strncat(), but does not 0 fill the buffer, and always NULL
// terminates the buffer. size is the length of the buffer, which should be one more than the
// maximum resulting string length.
extern size_t safe_string_copy(char *dst, const char *src, size_t size);

// Removes any new-line or carriage-return characters from the end of the string. This function is
// named after the same function in Perl. "length" should be the number of characters in the buffer,
// not including the trailing NULL.
//
// Returns the number of characters removed from the end of the string. A negative return value
// indicates an error.
extern ssize_t trim_ending_newlines(char *buffer, size_t length);

#endif // TINYPROXY_TEXT_H
