/* tinyproxy - A fast light-weight HTTP proxy
 *
 * Copyright (C) 1998 Steven Young <sdyoung@miranda.org>
 * Copyright (C) 1998-2002 Robert James Kaes <rjkaes@users.sourceforge.net>
 * Copyright (C) 2000 Chris Lightfoot <chris@ex-parrot.com>
 * Copyright (C) 2009-2010 Mukund Sivaraman <muks@banu.com>
 * Copyright (C) 2009-2010 Michael Adam <obnox@samba.org>
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

/* The initialize routine. Basically sets up all the initial stuff (logfile,
 * listening socket, config options, etc.) and then sits there and loops
 * over the new connections until the daemon is closed. Also has additional
 * functions to handle the "user friendly" aspects of a program (usage,
 * stats, etc.) Like any good program, most of the work is actually done
 * elsewhere.
 */

#include "main.h"

#include "anonymous.h"
#include "buffer.h"
#include "child.h"
#include "config/conf.h"
#include "config/conf_log.h"
#include "daemon.h"
#include "filter.h"
#include "misc/file_api.h"
#include "misc/heap.h"
#include "reqs.h"
#include "self_contained/debugtrace.h"
#include "sock.h"
#include "stats.h"
#include "subservice/log.h"
#include "tinyproxy.h"
#include "utils.h"

/*
 * Global Structures
 */
struct config_s config;
struct config_s config_defaults;
unsigned int received_sighup = FALSE; /* boolean */

/*
 * Handle a signal
 */
static void takesig(int sig)
{
  int status;

  switch (sig)
  {
  case SIGTERM:
    config.quit = TRUE;
    break;
#ifndef MINGW
  case SIGHUP:
    received_sighup = TRUE;
    break;

  case SIGCHLD:
    while (waitpid(-1, &status, WNOHANG) > 0)
      ;
    break;
#endif
  }
}

/*
 * Display the version information for the user.
 */
static void display_version(void)
{
  printf("%s %s\n", PACKAGE, VERSION);
}

/*
 * Display usage to the user.
 */
static void display_usage(void)
{
  int features = 0;

  printf("Usage: %s [options]\n", PACKAGE);
  printf("\n"
         "Options are:\n"
         "  -d        Do not daemonize (run in foreground).\n"
         "  -c FILE   Use an alternate configuration file.\n"
         "  -h        Display this usage information.\n"
         "  -v        Display version information.\n");

  /* Display the modes compiled into tinyproxy */
  printf("\nFeatures compiled in:\n");

#ifdef XTINYPROXY_ENABLE
  printf("    XTinyproxy header\n");
  features++;
#endif /* XTINYPROXY */

#ifdef FILTER_ENABLE
  printf("    Filtering\n");
  features++;
#endif /* FILTER_ENABLE */

#ifndef NDEBUG
  printf("    Debugging code\n");
  features++;
#endif /* NDEBUG */

#ifdef TRANSPARENT_PROXY
  printf("    Transparent proxy support\n");
  features++;
#endif /* TRANSPARENT_PROXY */

#ifdef REVERSE_SUPPORT
  printf("    Reverse proxy support\n");
  features++;
#endif /* REVERSE_SUPPORT */

#ifdef UPSTREAM_SUPPORT
  printf("    Upstream proxy support\n");
  features++;
#endif /* UPSTREAM_SUPPORT */

  if (0 == features)
    printf("    None\n");

  printf("\n"
         "For support and bug reporting instructions, please visit\n"
         "<https://tinyproxy.github.io/>.\n");
}

static int get_id(char *str)
{
  char *tstr;

  if (str == NULL)
    return -1;

  tstr = str;
  while (*tstr != 0)
  {
    if (!isdigit(*tstr))
      return -1;
    tstr++;
  }

  return atoi(str);
}

/**
 * process_cmdline:
 * @argc: argc as passed to main()
 * @argv: argv as passed to main()
 *
 * This function parses command line arguments.
 *
 * RETURN: 0     - normal state, safe to continue execution
 *         1     - info showed, needs normal exit
 *         other - error occurred, needs exit with error
 **/
static int process_cmdline(int argc, char **argv, struct config_s *conf)
{
  TRACECALLEX(process_cmdline, "%d, %p, %p", argc, (void *)argv, (void *)conf);

  int opt;

  while ((opt = getopt(argc, argv, "c:vdh")) != EOF)
  {
    switch (opt)
    {
    case 'v':
      display_version();
      TRACERETURNEX(1, "%s", "-v: display_version");

    case 'c':
      if (conf->config_file != NULL)
      {
        safefree(conf->config_file);
      }
      conf->config_file = safestrdup(optarg);
      if (!conf->config_file)
      {
        TRACERETURNEX(-1, "%s: Could not allocate memory.\n", argv[0]);
      }
      break;

    case 'h':
      display_usage();
      TRACERETURNEX(1, "%s", "-h: display_usage");

    default:
      display_usage();
      TRACERETURNEX(1, "%s", "unknown argument: display_usage");
    }
  }

  TRACERETURN(0);
}

static int initialize_config_defaults(struct config_s *conf)
{
  TRACECALLEX(initialize_config_defaults, "config_s *conf = %p", (void *)conf);

  memset(conf, 0, sizeof(*conf));

  conf->config_file = safestrdup("tinyproxy.conf");
  if (!conf->config_file)
  {
    TRACERETURNEX(-1, "conf->config_file = %p", (void *)conf->config_file);
  }

  /*
   * Make sure the HTML error pages array is NULL to begin with.
   * (FIXME: Should have a better API for all this)
   */
  conf->stathost = safestrdup(TINYPROXY_STATHOST);
  if (!conf->stathost)
  {
    TRACERETURNEX(-1, "conf->stathost = %p", (void *)conf->stathost);
  }

  conf->errorpages = NULL;
  conf->idletimeout = MAX_IDLE_TIME;
  conf->pidpath = NULL;

  // setup log
  conf->log = create_pconf_log_t();

  TRACERETURN(0);
}

#ifdef HAVE_WSOCK32
WSADATA wsa;
int initialize_winsock()
{
  TRACECALL(initialize_winsock);
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
  {
    TRACERETURNEX(-1, "Initialising Winsock Failed. Error Code : %d", WSAGetLastError());
  }
  TRACERETURNEX(0);
}
#else
int initialize_winsock()
{
  return 0;
}
#endif

int init_proxy_log(pproxy_t proxy, struct config_s *config)
{
  assert(proxy != NULL);
  assert(config != NULL);
  proxy->log = create_log(config->log);

  if (proxy->log == NULL)
  {
    return -1;
  }

  return activate_logging(proxy->log);
}

int main(int argc, char **argv)
{
  TRACECALLEX(main, "%d, %p", argc, (void *)argv);

  if (initialize_winsock() != 0)
  {
    exit(EX_SOFTWARE);
  }

  /* Only allow u+rw bits. This may be required for some versions
   * of glibc so that mkstemp() doesn't make us vulnerable.
   */
  umask(0177);

  if (config_compile_regex())
  {
    exit(EX_SOFTWARE);
  }

  if (initialize_config_defaults(&config_defaults))
  {
    exit(EX_SOFTWARE);
  }

  {
    int r = process_cmdline(argc, argv, &config_defaults);
    switch (r)
    {
    case 0:
      break;
    case 1:
      exit(EX_OK);
      break;
    default:
      exit(EX_SOFTWARE);
      break;
    }
  }

  if (try_load_config_file(config_defaults.config_file, &config, &config_defaults))
  {
    exit(EX_SOFTWARE);
  }

  proxy_t proxy;
  if (init_proxy_log(&proxy, &config))
  {
    exit(EX_SOFTWARE);
  }

  init_stats();

  /* If ANONYMOUS is turned on, make sure that Content-Length is
   * in the list of allowed headers, since it is required in a
   * HTTP/1.0 request. Also add the Content-Type header since it
   * goes hand in hand with Content-Length. */
  if (is_anonymous_enabled())
  {
    anonymous_insert("Content-Length");
    anonymous_insert("Content-Type");
  }

#ifdef FILTER_ENABLE
  if (config.filter)
    filter_init();
#endif /* FILTER_ENABLE */

  /* Start listening on the selected port. */
  if (child_listening_sockets(&proxy, config.listen_addrs, config.port) < 0)
  {
    log_message(proxy.log, LOG_ERR, "%s: Could not create listening sockets.\n", argv[0]);
    exit(EX_OSERR);
  }

  /* Create pid file before we drop privileges */
  if (config.pidpath)
  {
    if (pidfile_create(config.pidpath) < 0)
    {
      log_message(proxy.log, LOG_ERR, "%s: Could not create PID file.\n", argv[0]);
      exit(EX_OSERR);
    }
  }

  if (child_pool_create(&proxy) < 0)
  {
    log_message(proxy.log, LOG_ERR, "%s: Could not create the pool of children.\n", argv[0]);
    exit(EX_SOFTWARE);
  }

  /* These signals are only for the parent process. */
  log_message(proxy.log, LOG_INFO, "Setting the various signals.");

  if (set_signal_handler(SIGTERM, takesig) == SIG_ERR)
  {
    log_message(proxy.log, LOG_ERR, "%s: Could not set the \"SIGTERM\" signal.\n", argv[0]);
    exit(EX_OSERR);
  }

#ifndef MINGW
  if (set_signal_handler(SIGCHLD, takesig) == SIG_ERR)
  {
    log_message(proxy.log, LOG_ERR, "%s: Could not set the \"SIGCHLD\" signal.\n", argv[0]);
    exit(EX_OSERR);
  }

  if (set_signal_handler(SIGHUP, takesig) == SIG_ERR)
  {
    log_message(proxy.log, LOG_ERR, "%s: Could not set the \"SIGHUP\" signal.\n", argv[0]);
    exit(EX_OSERR);
  }

  if (set_signal_handler(SIGPIPE, SIG_IGN) == SIG_ERR)
  {
    log_message(proxy.log, LOG_ERR, "%s: Could not set the \"SIGPIPE\" signal.\n", argv[0]);
    exit(EX_OSERR);
  }
#endif /* MINGW */

  /* Start the main loop */
  log_message(proxy.log, LOG_INFO, "Starting main loop. Accepting connections.");

  child_main_loop(&proxy);

  log_message(proxy.log, LOG_INFO, "Shutting down.");

  child_kill_children(&proxy, SIGTERM);

  child_close_sock();

  /* Remove the PID file */
  if (config.pidpath != NULL && unlink(config.pidpath) < 0)
  {
    log_message(proxy.log, LOG_WARNING, "Could not remove PID file \"%s\": %s.", config.pidpath,
                strerror(errno));
  }

#ifdef FILTER_ENABLE
  if (config.filter)
  {
    filter_destroy();
  }
#endif /* FILTER_ENABLE */

  shutdown_logging(&proxy.log);

  return EXIT_SUCCESS;
}
