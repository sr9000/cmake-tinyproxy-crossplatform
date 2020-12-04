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

#include "config/conf.h"
#include "debugtrace.h"
#include "misc/file_api.h"
#include "misc/heap.h"
#include "misc/list.h"
#include "subservice/log.h"

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

// Hold a listing of log messages which need to be sent once the log file has been established.
// The key is the actual messages (already filled in full), and the value is the log level.
static plist_t log_message_storage;

struct log_s
{
  conf_log_t config;
  int fd;
};

void assign_default_values(plog_t log)
{
  initialize_conf_log_defaults(&log->config);
  log->fd = -1;
}

plog_t create_default_log()
{
  TRACECALL(create_default_log);

  plog_t log = safemalloc(sizeof(struct log_s));
  if (log == NULL)
  {
    TRACERETURNEX(NULL, "%s", "Error when allocating memory for struct log_s");
  }

  assign_default_values(log);

  TRACERETURN(log);
}

plog_t create_log(pconf_log_t conf_log)
{
  TRACECALL(create_log);

  plog_t log = create_default_log();
  if (log == NULL)
  {
    TRACERETURNEX(NULL, "%s", "Error when creating default struct log_s");
  }

  assign_conf_log(&log->config, conf_log);

  TRACERETURN(log);
}

void delete_log(plog_t *pplog)
{
  assert(pplog != NULL);
  safefree(*pplog);
  *pplog = NULL;
}

static unsigned int logging_initialized = FALSE; /* boolean */

/*
 * Open the log file and store the file descriptor in a global location.
 */
int open_log_file(plog_t log)
{
  if (log->config.logf_name == NULL)
  {
    log->fd = fileno(stdout);
  }
  else
  {
    log->fd = create_file_safely(log->config.logf_name, FALSE);
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
  if (log->config.log_level == LOG_CONN)
  {
    if (level == LOG_INFO)
      return;
  }
  else if (log->config.log_level == LOG_INFO)
  {
    if (level > LOG_INFO && level != LOG_CONN)
      return;
  }
  else if (level > log->config.log_level)
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
              config.log.logf_name, strerror(errno));
      exit(EX_SOFTWARE);
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
  TRACECALL(activate_logging);

  if (open_log_file(log) < 0)
  {
    TRACERETURNEX(-1, "ERROR: Could not create log file %s: %s.", config.log.logf_name,
                  strerror(errno));
  }

  TRACERETURN(0);
}

/**
 * Stop the logging subsystem.
 */
void shutdown_logging(plog_t *pplog)
{
  close_log_file(*pplog);
  delete_log(pplog);
}
