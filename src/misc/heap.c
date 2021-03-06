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

/* Debugging versions of various heap related functions are combined
 * here.  The debugging versions include assertions and also print
 * (to standard error) the function called along with the amount
 * of memory allocated, and where the memory is pointing.  The
 * format of the log message is standardized.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "misc/heap.h"

#ifdef NDEBUG
// DO NOTHING
#else

#include <stddef.h>
#include <stdio.h>

void *debugging_calloc(size_t nmemb, size_t size, const char *file, unsigned long line)
{
  void *ptr;

  assert(nmemb > 0);
  assert(size > 0);

  ptr = calloc(nmemb, size);
  fprintf(stderr,
          "MEMORY {calloc: %p:%lu x %lu} "
          "%s:%lu\n",
          ptr, (unsigned long)nmemb, (unsigned long)size, file, line);
  return ptr;
}

void *debugging_malloc(size_t size, const char *file, unsigned long line)
{
  void *ptr;

  assert(size > 0);

  ptr = malloc(size);
  fprintf(stderr,
          "MEMORY {malloc: %p:%lu} "
          "%s:%lu\n",
          ptr, (unsigned long)size, file, line);
  return ptr;
}

void *debugging_realloc(void *ptr, size_t size, const char *file, unsigned long line)
{
  void *newptr;

  assert(size > 0);

  newptr = realloc(ptr, size);
  fprintf(stderr,
          "MEMORY {realloc: %p -> %p:%lu} "
          "%s:%lu\n",
          ptr, newptr, (unsigned long)size, file, line);
  return newptr;
}

void debugging_free(void *ptr, const char *file, unsigned long line)
{
  fprintf(stderr, "MEMORY {free: %p} %s:%lu\n", ptr, file, line);
  free(ptr);
}

char *debugging_strdup(const char *s, const char *file, unsigned long line)
{
  assert(s != NULL);

  char *ptr;
  size_t len;

  len = strlen(s) + 1;
  ptr = (char *)malloc(len);
  if (!ptr)
  {
    return NULL;
  }
  memcpy(ptr, s, len);

  fprintf(stderr,
          "MEMORY {strdup: %p:%lu} "
          "%s:%lu\n",
          (void *)ptr, (unsigned long)len, file, line);
  return ptr;
}

#endif // NDEBUG

/*
 * Allocate a block of memory in the "shared" memory region.
 *
 * FIXME: This uses the most basic (and slowest) means of creating a
 * shared memory location.  It requires the use of a temporary file.  We might
 * want to look into something like MM (Shared Memory Library) for a better
 * solution.
 */
#ifdef MINGW
// On windows threads use commond address space so we dont need shared memory
// mechanism
void *malloc_shared_memory(size_t size)
{
  return malloc(size);
}
void *calloc_shared_memory(size_t nmemb, size_t size)
{
  return calloc(nmemb, size);
}
#else // MINGW
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

void *malloc_shared_memory(size_t size)
{
  int fd;
  void *ptr;
  char buffer[32];

  static const char *shared_file = "/tmp/tinyproxy.shared.XXXXXX";

  assert(size > 0);

  strcpy(buffer, shared_file);

  /* Only allow u+rw bits. This may be required for some versions
   * of glibc so that mkstemp() doesn't make us vulnerable.
   */
  umask(0177);

  if ((fd = mkstemp(buffer)) == -1)
  {
    return MAP_FAILED;
  }
  unlink(buffer);

  if (ftruncate(fd, size) == -1)
  {
    return MAP_FAILED;
  }
  ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  return ptr;
}

/*
 * Allocate a block of memory from the "shared" region an initialize it to
 * zero.
 */
void *calloc_shared_memory(size_t nmemb, size_t size)
{
  void *ptr;
  long length;

  assert(nmemb > 0);
  assert(size > 0);

  length = nmemb * size;

  ptr = malloc_shared_memory(length);
  if (ptr == MAP_FAILED)
  {
    return ptr;
  }

  memset(ptr, 0, length);

  return ptr;
}
#endif /* MINGW */
