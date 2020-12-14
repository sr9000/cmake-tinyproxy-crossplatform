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

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "subservice/log.h"

#include "misc/file_api.h"
#include "misc/heap.h"
#include "self_contained/safecall.h"

static const char *syslog_level[] = {NULL,     NULL,   "CRITICAL", "ERROR",  "WARNING",
                                     "NOTICE", "INFO", "DEBUG",    "CONNECT"};

#define TIME_LENGTH   16
#define STRING_LENGTH 800

struct log_s
{
  pconf_log_t config;
  int fd;
};

CREATE_IMPL(plog_t, {
  TRACE_SAFE_R(NULL == (obj->config = create_pconf_log_t()), NULL);
  obj->fd = -1;
})

plog_t create_configured_log(pconf_log_t conf_log)
{
  TRACE_CALL(create_configured_log);

  plog_t log;
  TRACE_SAFE_X(NULL == (log = create_plog_t()), NULL, "%s",
               "Error when creating default struct log_s");

  TRACE_SAFE_R(delete_pconf_log_t(&log->config), NULL);
  TRACE_SAFE_R(NULL == (log->config = clone_pconf_log_t(conf_log)), NULL);

  TRACE_RETURN(log);
}

DELETE_IMPL(plog_t, {
  close_log_file(obj);
  TRACE_SAFE(delete_pconf_log_t(&obj->config));
})

/*
 * Open the log file and store the file descriptor in a global location.
 */
int open_log_file(plog_t log)
{
  if (log->config->logf_name == NULL)
  {
    log->fd = fileno(stdout);
  }
  else
  {
    log->fd = create_file_safely(log->config->logf_name, false);
  }
  return log->fd;
}

/*
 * Close the log file
 */
void close_log_file(plog_t log)
{
  if (log->fd < 0 || log->fd == fileno(stdout))
  {
    return;
  }

  close(log->fd);
  log->fd = -1;
}

/*
 * This routine logs messages to either the log file or the syslog function.
 */
void log_message(plog_t log, int level, const char *fmt, ...)
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
  if (log->config->log_level == LOG_CONN)
  {
    if (level == LOG_INFO)
      return;
  }
  else if (log->config->log_level == LOG_INFO)
  {
    if (level > LOG_INFO && level != LOG_CONN)
      return;
  }
  else if (level > log->config->log_level)
    return;
#endif

  va_start(args, fmt);

  if (log->fd != -1)
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

    assert(log->fd >= 0);

    ret = write(log->fd, str, strlen(str));
    if (ret == -1)
    {
      fprintf(stderr,
              "ERROR: Could not write to log "
              "file %s: %s.",
              log->config->logf_name, strerror(errno));
      exit(EXIT_FAILURE);
    }

    flush_file_buffer(log->fd);
  }

  va_end(args);
}

/**
 * Initialize the logging subsystem, based on the configuration.
 * Returns 0 upon success, -1 upon failure.
 *
 * This function uses fprintf() instead of log_message(), since
 * the logging is not yet set up...
 */
int activate_logging(plog_t log)
{
  TRACE_CALL(activate_logging);

  if (open_log_file(log) < 0)
  {
    TRACE_RETURN_X(-1, "ERROR: Could not create log file %s: %s.", log->config->logf_name,
                   strerror(errno));
  }

  TRACE_RETURN(0);
}
