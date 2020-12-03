/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 1998 Steven Young <sdyoung@miranda.org>
 * Copyright (C) 1999 Robert James Kaes <rjkaes@users.sourceforge.net>
 * Copyright (C) 2009 Michael Adam <obnox@samba.org>
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

/* Logs the various messages which tinyproxy produces to either a log file
 * or the syslog daemon. Not much to it...
 */

#include "main.h"

#include "conf.h"
#include "log.h"
#include "misc/file_api.h"
#include "misc/heap.h"
#include "misc/list.h"

static const char *syslog_level[] = {NULL,     NULL,   "CRITICAL", "ERROR",  "WARNING",
                                     "NOTICE", "INFO", "DEBUG",    "CONNECT"};

#define TIME_LENGTH   16
#define STRING_LENGTH 800

/*
 * Global file descriptor for the log file
 */
int log_file_fd = -1;

/*
 * Store the log level setting.
 */
static int log_level = LOG_INFO;

/*
 * Hold a listing of log messages which need to be sent once the log
 * file has been established.
 * The key is the actual messages (already filled in full), and the value
 * is the log level.
 */
static plist_t log_message_storage;

static unsigned int logging_initialized = FALSE; /* boolean */

/*
 * Open the log file and store the file descriptor in a global location.
 */
int open_log_file(const char *log_file_name)
{
  if (log_file_name == NULL)
  {
    log_file_fd = fileno(stdout);
  }
  else
  {
    log_file_fd = create_file_safely(log_file_name, FALSE);
  }
  return log_file_fd;
}

/*
 * Close the log file
 */
void close_log_file(void)
{
  if (log_file_fd < 0 || log_file_fd == fileno(stdout))
  {
    return;
  }

  close(log_file_fd);
  log_file_fd = -1;
}

/*
 * Set the log level for writing to the log file.
 */
void set_log_level(int level)
{
  log_level = level;
}

/*
 * This routine logs messages to either the log file or the syslog function.
 */
void log_message(int level, const char *fmt, ...)
{
  va_list args;
  time_t nowtime;

  char time_string[TIME_LENGTH];
  char str[STRING_LENGTH];

  ssize_t ret;

#ifdef NDEBUG
  /*
   * Figure out if we should write the message or not.
   */
  if (log_level == LOG_CONN)
  {
    if (level == LOG_INFO)
      return;
  }
  else if (log_level == LOG_INFO)
  {
    if (level > LOG_INFO && level != LOG_CONN)
      return;
  }
  else if (level > log_level)
    return;
#endif

  va_start(args, fmt);

  /*
   * If the config file hasn't been processed, then we need to store
   * the messages for later processing.
   */
  if (!logging_initialized)
  {
    char *entry_buffer;

    if (!log_message_storage)
    {
      log_message_storage = list_create();
      if (!log_message_storage)
        goto out;
    }

    vsnprintf(str, STRING_LENGTH, fmt, args);

    entry_buffer = (char *)safemalloc(strlen(str) + 6);
    if (!entry_buffer)
      goto out;

    sprintf(entry_buffer, "%d %s", level, str);
    list_append(log_message_storage, entry_buffer, strlen(entry_buffer) + 1);

    safefree(entry_buffer);
    goto out;
  }

  if (log_file_fd == -1)
    goto out;

  {
    char *p;

    nowtime = time(NULL);
    /* Format is month day hour:minute:second (24 time) */
    strftime(time_string, TIME_LENGTH, "%b %d %H:%M:%S", localtime(&nowtime));

    snprintf(str, STRING_LENGTH, "%-9s %s [%ld]: ", syslog_level[level], time_string,
             (long int)getpid());

    /*
     * Overwrite the '\0' and leave room for a trailing '\n'
     * be added next.
     */
    p = str + strlen(str);
    vsnprintf(p, STRING_LENGTH - strlen(str) - 1, fmt, args);

    p = str + strlen(str);
    *p = '\n';
    *(p + 1) = '\0';

    assert(log_file_fd >= 0);

    ret = write(log_file_fd, str, strlen(str));
    if (ret == -1)
    {
      fprintf(stderr,
              "ERROR: Could not write to log "
              "file %s: %s.",
              config.logf_name, strerror(errno));
      exit(EX_SOFTWARE);
    }

    flush_file_buffer(log_file_fd);
  }

out:
  va_end(args);
}

/*
 * This needs to send any stored log messages.
 */
static void send_stored_logs(void)
{
  char *string;
  char *ptr;
  int level;
  size_t i;

  if (log_message_storage == NULL)
    return;

  log_message(LOG_DEBUG, "sending stored logs");

  for (i = 0; (ssize_t)i != list_length(log_message_storage); ++i)
  {
    string = (char *)list_getentry(log_message_storage, i, NULL);

    ptr = strchr(string, ' ') + 1;
    level = atoi(string);

#ifdef NDEBUG
    if (log_level == LOG_CONN && level == LOG_INFO)
      continue;
    else if (log_level == LOG_INFO)
    {
      if (level > LOG_INFO && level != LOG_CONN)
        continue;
    }
    else if (level > log_level)
      continue;
#endif

    log_message(level, "%s", ptr);
  }

  list_delete(log_message_storage);
  log_message_storage = NULL;

  log_message(LOG_DEBUG, "done sending stored logs");
}

/**
 * Initialize the logging subsystem, based on the configuration.
 * Returns 0 upon success, -1 upon failure.
 *
 * This function uses fprintf() instead of log_message(), since
 * the logging is not yet set up...
 */
int setup_logging(void)
{
  {
    if (open_log_file(config.logf_name) < 0)
    {
      /*
       * If opening the log file fails, we try
       * to fall back to syslog logging...
       */
      fprintf(stderr,
              "ERROR: Could not create log "
              "file %s: %s.",
              config.logf_name, strerror(errno));
      exit(EX_SOFTWARE);
    }
  }

  logging_initialized = TRUE;
  send_stored_logs();

  return 0;
}

/**
 * Stop the logging subsystem.
 */
void shutdown_logging(void)
{
  if (!logging_initialized)
  {
    return;
  }

  {
    close_log_file();
  }

  logging_initialized = FALSE;
}
