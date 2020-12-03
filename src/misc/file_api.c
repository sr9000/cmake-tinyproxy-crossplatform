//
// Created by sr9000 on 03/12/2020.
//

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "misc/file_api.h"
#include "misc/heap.h"

#define FATAL_FILE_ERROR(msg, filename)                                                            \
  fprintf(stderr,                                                                                  \
          "ERROR: %s: "                                                                            \
          "%s"                                                                                     \
          " file %s: %s\n",                                                                        \
          PACKAGE, (msg), filename, strerror(errno))

#define FILE_ERROR(msg, filename)                                                                  \
  fprintf(stderr,                                                                                  \
          "ERROR: %s: "                                                                            \
          "%s"                                                                                     \
          " file %s\n",                                                                            \
          PACKAGE, (msg), filename)

// Safely creates filename and returns the low-level file descriptor.
#ifdef MINGW
#include <io.h>

#define O_RDWR  _O_RDWR
#define O_CREAT _O_CREAT
#define O_EXCL  _O_EXCL
#define O_TRUNC _O_TRUNC

#endif // MINGW
int create_file_safely(const char *filename, bool truncate_file)
{
  struct stat lstatinfo;
  int fildes;

  // stat() the file
  // if file exists, open it for writing and perform the fstat() check
  // if not, create it with O_EXCL
  if (stat(filename, &lstatinfo) < 0)
  {
    // if stat() failed for any reason other than "file not existing", exit.
    if (errno != ENOENT)
    {
      FATAL_FILE_ERROR("Error checking.", filename);
      return -EACCES;
    }

    // the file doesn't exist, so create it with O_EXCL to make  sure an attacker can't slip in a
    // file between the lstat() and open()
    if ((fildes = open(filename, O_RDWR | O_CREAT | O_EXCL, 0600)) < 0)
    {
      FATAL_FILE_ERROR("Could not create.", filename);
      return fildes;
    }
  }
  else
  {
    int flags;

    flags = O_RDWR;
    if (!truncate_file)
      flags |= O_APPEND;

    // open an existing file
    if ((fildes = open(filename, flags)) < 0)
    {
      FATAL_FILE_ERROR("Could not open.", filename);
      return fildes;
    }

    // just return the file descriptor if we _don't_ want the file truncated.
    if (!truncate_file)
      return fildes;

#ifdef HAVE_FTRUNCATE
    if (ftruncate(fildes, 0) != 0)
    {
      log_message(LOG_WARNING, "Unable to truncate file '%s'", filename);
    }
#else // HAVE_FTRUNCATE
    // On systems which don't support ftruncate() the best we can do is to close the file and
    // reopen it in create mode, which unfortunately leads to a race condition, however "systems
    // which don't support ftruncate()" is pretty much SCO only,
    // and if you're using that you deserve what you get.
    // ("Little sympathy has been extended")
    close(fildes);
    if ((fildes = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0600)) < 0)
    {
      FATAL_FILE_ERROR("Could not open.", filename);
      return fildes;
    }
#endif // HAVE_FTRUNCATE
  }

  return fildes;
}

// Creates a file with the PID of the Tinyproxy process.
//
// Returns: 0 on success
//          non-zero values on errors
int pidfile_create(const char *filename)
{
  int fildes;
  FILE *fd;

  // create a new file
  if ((fildes = create_file_safely(filename, true)) < 0)
    return fildes;

  // open a stdio file over the low-level one
  if ((fd = fdopen(fildes, "w")) == NULL)
  {
    FATAL_FILE_ERROR("Could not write PID.", filename);
    close(fildes);
    unlink(filename);
    return -EIO;
  }

  fprintf(fd, "%d\n", getpid());
  fclose(fd);
  return 0;
}

#ifdef MINGW
#include <windows.h>

int flush_file_buffer(int fd)
{
  HANDLE h = (HANDLE)_get_osfhandle(fd);
  DWORD err;

  if (h == INVALID_HANDLE_VALUE)
  {
    errno = EBADF;
    return -1;
  }

  if (!FlushFileBuffers(h))
  {
    // Translate some Windows errors into rough approximations of Unix errors. MSDN is useless as
    // usual - in this case it doesn't document the full range of errors.
    err = GetLastError();
    switch (err)
    {
    case ERROR_ACCESS_DENIED:
      // For a read-only handle, fsync should succeed, even though we have no way to sync the
      // access-time changes.
      return 0;

    // eg. Trying to fsync a tty.
    case ERROR_INVALID_HANDLE:
      errno = EINVAL;
      break;

    default:
      errno = EIO;
    }
    return -1;
  }

  return 0;
}
#else // MINGW
int flush_file_buffer(int fd)
{
  return fsync(fd);
}
#endif // MINGW
