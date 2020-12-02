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
int create_file_safely(const char *filename, bool truncate_file)
{
  // todo: create_file_safely for mingw
}
#else // MINGW
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
    // file exists, so open it for writing and perform the fstat() check
    struct stat fstatinfo;
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

    // fstat() the opened file and check that the file mode bits, inode, and device match
    if (fstat(fildes, &fstatinfo) < 0 || lstatinfo.st_mode != fstatinfo.st_mode ||
        lstatinfo.st_ino != fstatinfo.st_ino || lstatinfo.st_dev != fstatinfo.st_dev)
    {
      FILE_ERROR("The file has been changed before it could be opened.", filename);
      close(fildes);
      return -EIO;
    }

    // If the above check was passed, we know that the stat() and fstat() were done on the same
    // file. Now we check that there's only one link, and that it's a normal file (this isn't
    // strictly necessary because the fstat() vs stat() st_mode check would also find this)
    if (fstatinfo.st_nlink > 1 || !S_ISREG(lstatinfo.st_mode))
    {
      FATAL_FILE_ERROR("The file has too many links or is not a regular file.", filename);
      close(fildes);
      return -EMLINK;
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
#endif // MINGW

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
