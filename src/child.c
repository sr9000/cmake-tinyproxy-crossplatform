/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 2000 Robert James Kaes <rjkaes@users.sourceforge.net>
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

/* Handles the creation/destruction of the various children required for
 * processing incoming connections.
 */

#include "main.h"

#include "child.h"
#include "config/conf.h"
#include "daemon.h"
#include "misc/heap.h"
#include "reqs.h"
#include "self_contained/debugtrace.h"
#include "sock.h"
#include "subservice/filter.h"
#include "subservice/log.h"
#include "subservice/network.h"
#include "utils.h"

static plist_t listen_fds;

/*
 * Stores the internal data needed for each child (connection)
 */
enum child_status_t
{
  T_EMPTY,
  T_WAITING,
  T_CONNECTED
};
struct child_s
{
  pid_t tid;
  unsigned int connects;
  enum child_status_t status;
  pproxy_t proxy;
};

/*
 * A pointer to an array of children. A certain number of children are
 * created when the program is started.
 */
static struct child_s *child_ptr;

static struct child_config_s
{
  unsigned int maxclients, maxrequestsperchild;
  unsigned int maxspareservers, minspareservers, startservers;
} child_config;

static unsigned int *servers_waiting; /* servers waiting for a connection */

/*
 * Lock/Unlock the "servers_waiting" variable so that two children cannot
 * modify it at the same time.
 */
#define SERVER_COUNT_LOCK()   _child_lock_wait()
#define SERVER_COUNT_UNLOCK() _child_lock_release()

/* START OF LOCKING SECTION */

#ifdef MINGW
/*
 * Using Mutex Objects
 */
HANDLE ghMutex;

static void _child_lock_init(void)
{
  ghMutex = CreateMutex(NULL,  // default security attributes
                        FALSE, // initially not owned
                        NULL); // unnamed mutex

  if (ghMutex == NULL)
  {
    fprintf(stderr, "CreateMutex error: %d\n", GetLastError());
    exit(EX_SOFTWARE);
  }
}
static void _child_lock_wait(void)
{
  DWORD dwWaitResult;
  dwWaitResult = WaitForSingleObject(ghMutex,   // handle to mutex
                                     INFINITE); // no time-out interval

  switch (dwWaitResult)
  {
  // The thread got ownership of the mutex
  case WAIT_OBJECT_0:
    return;

  // The thread got ownership of an abandoned mutex
  // The database is in an indeterminate state
  case WAIT_ABANDONED:
    fprintf(stderr, "WaitForSingleObject(ghMutex) error: WAIT_ABANDONED\n");
    exit(EX_SOFTWARE);
  }
}
static void _child_lock_release(void)
{
  if (!ReleaseMutex(ghMutex))
  {
    fprintf(stderr, "ReleaseMutex(ghMutex) error: cannot release mutex\n");
    exit(EX_SOFTWARE);
  }
}
#else
/*
 * These variables are required for the locking mechanism.  Also included
 * are the "private" functions for locking/unlocking.
 */
static struct flock lock_it, unlock_it;
static int lock_fd = -1;
static void _child_lock_init(void)
{
  char lock_file[] = "/tmp/tinyproxy.servers.lock.XXXXXX";

  /* Only allow u+rw bits. This may be required for some versions
   * of glibc so that mkstemp() doesn't make us vulnerable.
   */
  umask(0177);

  lock_fd = mkstemp(lock_file);
  unlink(lock_file);

  lock_it.l_type = F_WRLCK;
  lock_it.l_whence = SEEK_SET;
  lock_it.l_start = 0;
  lock_it.l_len = 0;

  unlock_it.l_type = F_UNLCK;
  unlock_it.l_whence = SEEK_SET;
  unlock_it.l_start = 0;
  unlock_it.l_len = 0;
}

static void _child_lock_wait(void)
{
  int rc;

  while ((rc = fcntl(lock_fd, F_SETLKW, &lock_it)) < 0)
  {
    if (errno == EINTR)
      continue;
    else
      return;
  }
}

static void _child_lock_release(void)
{
  if (fcntl(lock_fd, F_SETLKW, &unlock_it) < 0)
    return;
}
#endif
/* END OF LOCKING SECTION */

#define SERVER_INC(l)                                                                              \
  do                                                                                               \
  {                                                                                                \
    SERVER_COUNT_LOCK();                                                                           \
    ++(*servers_waiting);                                                                          \
    DEBUG_LOG_EX(l, "INC: servers_waiting: %d", *servers_waiting);                                 \
    SERVER_COUNT_UNLOCK();                                                                         \
  } while (0)

#define SERVER_DEC(l)                                                                              \
  do                                                                                               \
  {                                                                                                \
    SERVER_COUNT_LOCK();                                                                           \
    assert(*servers_waiting > 0);                                                                  \
    --(*servers_waiting);                                                                          \
    DEBUG_LOG_EX(l, "DEC: servers_waiting: %d", *servers_waiting);                                 \
    SERVER_COUNT_UNLOCK();                                                                         \
  } while (0)

/*
 * Set the configuration values for the various child related settings.
 */
short int child_configure(child_config_t type, unsigned int val)
{
  TRACE_CALL_X(child_configure, "policy = %d, val = %u", type, val);

  switch (type)
  {
  case CHILD_MAXCLIENTS:
    child_config.maxclients = val;
    break;
  case CHILD_MAXSPARESERVERS:
    child_config.maxspareservers = val;
    break;
  case CHILD_MINSPARESERVERS:
    child_config.minspareservers = val;
    break;
  case CHILD_STARTSERVERS:
    child_config.startservers = val;
    break;
  case CHILD_MAXREQUESTSPERCHILD:
    child_config.maxrequestsperchild = val;
    break;
  default:
    TRACE_RETURN_X(-1, "Invalid policy (%d)", type);
  }

  TRACE_RETURN(0);
}

/**
 * child signal handler for sighup
 */
#ifndef MINGW
static void child_sighup_handler(int sig)
{
  // todo: delete me
}
#endif /* MINGW */

/*
 * This is the main (per child) loop.
 */
static
#ifdef MINGW
    DWORD WINAPI
#else
    void
#endif
    child_main(struct child_s *ptr)
{
  int connfd;
  struct sockaddr *cliaddr;
  socklen_t clilen;
  fd_set rfds;
  int maxfd = 0;
  ssize_t i;
  int ret;

  cliaddr = (struct sockaddr *)safemalloc(sizeof(struct sockaddr_storage));
  if (!cliaddr)
  {
    log_message(ptr->proxy->log, LOG_CRIT, "Could not allocate memory for child address.");
    exit(0);
  }

  ptr->connects = 0;
  srand(time(NULL));

  /*
   * We have to wait for connections on multiple fds,
   * so use select.
   */

  FD_ZERO(&rfds);

  for (i = 0; i < list_length(listen_fds); i++)
  {
    int *fd = (int *)list_getentry(listen_fds, i, NULL);

    ret = socket_nonblocking(*fd);
    if (ret != 0)
    {
      log_message(ptr->proxy->log, LOG_ERR,
                  "Failed to set the listening "
                  "socket %d to non-blocking: %s",
                  fd, strerror(errno));
      exit(1);
    }

    FD_SET(*fd, &rfds);
    maxfd = max(maxfd, *fd);
  }

  while (!config.quit)
  {
    int listenfd = -1;

    ptr->status = T_WAITING;

    clilen = sizeof(struct sockaddr_storage);

    ret = select(maxfd + 1, &rfds, NULL, NULL, NULL);
    if (ret == -1)
    {
      if (errno == EINTR)
      {
        continue;
      }
      log_message(ptr->proxy->log, LOG_ERR, "error calling select: %s", strerror(errno));
      exit(1);
    }
    else if (ret == 0)
    {
      log_message(ptr->proxy->log, LOG_WARNING,
                  "Strange: select returned 0 "
                  "but we did not specify a timeout...");
      continue;
    }

    for (i = 0; i < list_length(listen_fds); i++)
    {
      int *fd = (int *)list_getentry(listen_fds, i, NULL);

      if (FD_ISSET(*fd, &rfds))
      {
        /*
         * only accept the connection on the first
         * fd that we find readable. - fair?
         */
        listenfd = *fd;
        break;
      }
    }

    if (listenfd == -1)
    {
      log_message(ptr->proxy->log, LOG_WARNING,
                  "Strange: None of our listen "
                  "fds was readable after select");
      continue;
    }

    ret = socket_blocking(listenfd);
    if (ret != 0)
    {
      log_message(ptr->proxy->log, LOG_ERR,
                  "Failed to set listening "
                  "socket %d to blocking for accept: %s",
                  listenfd, strerror(errno));
      exit(1);
    }

    /*
     * We have a socket that is readable.
     * Continue handling this connection.
     */

    connfd = accept(listenfd, cliaddr, &clilen);

#ifndef NDEBUG
    /*
     * Enable the TINYPROXY_DEBUG environment variable if you
     * want to use the GDB debugger.
     */
    if (getenv("TINYPROXY_DEBUG"))
    {
      /* Pause for 10 seconds to allow us to connect debugger */
      fprintf(stderr, "Process has accepted connection: %ld\n", (long int)ptr->tid);
      sleep(10);
      fprintf(stderr, "Continuing process: %ld\n", (long int)ptr->tid);
    }
#endif

    /*
     * Make sure no error occurred...
     */
    if (connfd < 0)
    {
      log_message(ptr->proxy->log, LOG_ERR, "Accept returned an error (%s) ... retrying.",
                  strerror(errno));
      continue;
    }

    ptr->status = T_CONNECTED;

    SERVER_DEC(ptr->proxy->log);

    //    handle_websocket_connection(ptr->proxy, connfd);
    handle_connection(ptr->proxy, connfd);
    ptr->connects++;

    if (child_config.maxrequestsperchild != 0)
    {
      DEBUG_LOG_EX(ptr->proxy->log, "%u connections so far...", ptr->connects);

      if (ptr->connects == child_config.maxrequestsperchild)
      {
        log_message(ptr->proxy->log, LOG_NOTICE,
                    "Child has reached MaxRequestsPerChild (%u). "
                    "Killing child.",
                    ptr->connects);
        break;
      }
    }

    SERVER_COUNT_LOCK();
    if (*servers_waiting > child_config.maxspareservers)
    {
      /*
       * There are too many spare children, kill ourself
       * off.
       */
      log_message(ptr->proxy->log, LOG_NOTICE,
                  "Waiting servers (%d) exceeds MaxSpareServers (%d). "
                  "Killing child.",
                  *servers_waiting, child_config.maxspareservers);
      SERVER_COUNT_UNLOCK();

      break;
    }
    else
    {
      SERVER_COUNT_UNLOCK();
    }

    SERVER_INC(ptr->proxy->log);
  }

  ptr->status = T_EMPTY;

  safefree(cliaddr);
  exit(0);
}

/*
 * Fork a child "child" (or in our case a process) and then start up the
 * child_main() function.
 */
#ifdef MINGW
static DWORD WINAPI mingw_child_main(void *pvoid)
{
  return child_main((struct child_s *)pvoid);
}

static DWORD child_make(struct child_s *ptr)
{
  HANDLE hThread;
  DWORD dwThreadId;
  hThread = CreateThread(NULL,             // default security attributes
                         0,                // use default stack size
                         mingw_child_main, // thread function name
                         ptr,              // argument to thread function
                         0,                // use default creation flags
                         &dwThreadId);     // returns the thread identifier

  if (hThread == NULL)
  {
    fprintf(stderr, "Could not create child process.\n");
    exit(EX_SOFTWARE);
  }
  CloseHandle(hThread);

  return dwThreadId;
}
#else
static pid_t child_make(struct child_s *ptr)
{
  pid_t pid;

  if ((pid = fork()) > 0)
    return pid; /* parent */

  /*
   * Reset the SIGNALS so that the child can be reaped.
   */
  set_signal_handler(SIGCHLD, SIG_DFL);
  set_signal_handler(SIGTERM, SIG_DFL);
  set_signal_handler(SIGHUP, child_sighup_handler);

  child_main(ptr); /* never returns */
  return -1;
}
#endif /* MINGW */

/*
 * Create a pool of children to handle incoming connections
 */
short int child_pool_create(pproxy_t proxy)
{
  unsigned int i;

  /*
   * Make sure the number of MaxClients is not zero, since this
   * variable determines the size of the array created for children
   * later on.
   */
  if (child_config.maxclients == 0)
  {
    log_message(proxy->log, LOG_ERR,
                "child_pool_create: \"MaxClients\" must be "
                "greater than zero.");
    return -1;
  }

  if (child_config.startservers == 0)
  {
    log_message(proxy->log, LOG_ERR,
                "child_pool_create: \"StartServers\" must be "
                "greater than zero.");
    return -1;
  }

  child_ptr =
      (struct child_s *)calloc_shared_memory(child_config.maxclients, sizeof(struct child_s));
  if (!child_ptr)
  {
    log_message(proxy->log, LOG_ERR, "Could not allocate memory for children.");
    return -1;
  }

  servers_waiting = (unsigned int *)malloc_shared_memory(sizeof(unsigned int));
  if (servers_waiting == MAP_FAILED)
  {
    log_message(proxy->log, LOG_ERR, "Could not allocate memory for child counting.");
    return -1;
  }
  *servers_waiting = 0;

  /*
   * Create a "locking" file for use around the servers_waiting
   * variable.
   */
  _child_lock_init();

  if (child_config.startservers > child_config.maxclients)
  {
    log_message(proxy->log, LOG_WARNING,
                "Can not start more than \"MaxClients\" servers. "
                "Starting %u servers instead.",
                child_config.maxclients);
    child_config.startservers = child_config.maxclients;
  }

  for (i = 0; i != child_config.maxclients; i++)
  {
    child_ptr[i].status = T_EMPTY;
    child_ptr[i].connects = 0;
  }

  for (i = 0; i != child_config.startservers; i++)
  {
    DEBUG_LOG_EX(proxy->log, "Trying to create child %d of %d", i + 1, child_config.startservers);
    child_ptr[i].status = T_WAITING;
    child_ptr[i].proxy = proxy;
    child_ptr[i].tid = child_make(&child_ptr[i]);

    if (child_ptr[i].tid < 0)
    {
      log_message(proxy->log, LOG_WARNING, "Could not create child number %d of %d", i,
                  child_config.startservers);
      return -1;
    }
    else
    {
      log_message(proxy->log, LOG_INFO, "Creating child number %d of %d ...", i + 1,
                  child_config.startservers);
      SERVER_INC(proxy->log);
    }
  }

  log_message(proxy->log, LOG_INFO, "Finished creating all children.");

  return 0;
}

/*
 * Keep the proper number of servers running. This is the birth of the
 * servers. It monitors this at least once a second.
 */
void child_main_loop(pproxy_t proxy)
{
  unsigned int i;

  while (1)
  {
    if (config.quit)
      return;

    /* If there are not enough spare servers, create more */
    SERVER_COUNT_LOCK();
    if (*servers_waiting < child_config.minspareservers)
    {
      log_message(proxy->log, LOG_NOTICE,
                  "Waiting servers (%d) is less than MinSpareServers (%d). "
                  "Creating new child.",
                  *servers_waiting, child_config.minspareservers);

      SERVER_COUNT_UNLOCK();

      for (i = 0; i != child_config.maxclients; i++)
      {
        if (child_ptr[i].status == T_EMPTY)
        {
          child_ptr[i].status = T_WAITING;
          child_ptr[i].proxy = proxy;
          child_ptr[i].tid = child_make(&child_ptr[i]);
          if (child_ptr[i].tid < 0)
          {
            log_message(proxy->log, LOG_NOTICE, "Could not create child");

            child_ptr[i].status = T_EMPTY;
            break;
          }

          SERVER_INC(proxy->log);

          break;
        }
      }
    }
    else
    {
      SERVER_COUNT_UNLOCK();
    }

    sleep(5);

#ifndef MINGW
    /* Handle log rotation if it was requested */
    if (received_sighup)
    {
      /*
       * Ignore the return value of reload_config for now.
       * This should actually be handled somehow...
       */
      // todo: delete this branch

      /* propagate filter reload to all children */
      child_kill_children(proxy, SIGHUP);

      received_sighup = FALSE;
    }
#endif /* MINGW */
  }
}

/*
 * Go through all the non-empty children and cancel them.
 */
void child_kill_children(pproxy_t proxy, int sig)
{
  unsigned int i;
#ifdef MINGW
  HANDLE hchl;
  LPVOID lpMsgBuf;
  LPVOID lpDisplayBuf;
#endif /* MINGW */

  for (i = 0; i != child_config.maxclients; i++)
  {
    if (child_ptr[i].status != T_EMPTY)
#ifdef MINGW
      hchl = OpenThread(THREAD_TERMINATE, FALSE, child_ptr[i].tid);
    if (hchl == NULL)
    {
      FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                         FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                     (LPSTR)&lpMsgBuf, 0, NULL);
      log_message(proxy->log, LOG_ERR, "Failed to close child thread: %d", lpMsgBuf);
      exit(EX_SOFTWARE);
    }
    if (!TerminateThread(hchl, sig))
    {
      FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                         FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                     (LPSTR)&lpMsgBuf, 0, NULL);
      log_message(proxy->log, LOG_ERR, "Failed to close child thread: %d", lpMsgBuf);
      exit(EX_SOFTWARE);
    }

    CloseHandle(hchl);
#else /* MINGW */
      kill(child_ptr[i].tid, sig);
#endif /* MINGW */
  }
}

/**
 * Listen on the various configured interfaces
 */
int child_listening_sockets(pproxy_t proxy, plist_t listen_addrs, uint16_t port)
{
  int ret;
  ssize_t i;

  assert(port > 0);

  if (listen_fds == NULL)
  {
    listen_fds = list_create();
    if (listen_fds == NULL)
    {
      log_message(proxy->log, LOG_ERR,
                  "Could not create the list "
                  "of listening fds");
      return -1;
    }
  }

  if ((listen_addrs == NULL) || (list_length(listen_addrs) == 0))
  {
    /*
     * no Listen directive:
     * listen on the wildcard address(es)
     */
    ret = listen_sock(proxy, NULL, port, listen_fds);
    return ret;
  }

  for (i = 0; i < list_length(listen_addrs); i++)
  {
    const char *addr;

    addr = (char *)list_getentry(listen_addrs, i, NULL);
    if (addr == NULL)
    {
      log_message(proxy->log, LOG_WARNING, "got NULL from listen_addrs - skipping");
      continue;
    }

    ret = listen_sock(proxy, addr, port, listen_fds);
    if (ret != 0)
    {
      return ret;
    }
  }

  return 0;
}

void child_close_sock(void)
{
  ssize_t i;

  for (i = 0; i < list_length(listen_fds); i++)
  {
    int *fd = (int *)list_getentry(listen_fds, i, NULL);
    closesocket(*fd);
  }

  list_delete(listen_fds);

  listen_fds = NULL;
}
