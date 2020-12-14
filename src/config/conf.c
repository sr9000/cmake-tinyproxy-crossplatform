/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 2004 Robert James Kaes <rjkaes@users.sourceforge.net>
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

/* Parses the configuration file and sets up the config_s structure for
 * use by the application.  This file replaces the old grammar.y and
 * scanner.l files.  It takes up less space and _I_ think is easier to
 * add new directives to.  Who knows if I'm right though.
 */

#include "config/conf.h"

#include "acl.h"
#include "anonymous.h"
#include "basicauth.h"
#include "child.h"
#include "connect-ports.h"
#include "filter.h"
#include "html-error.h"
#include "misc/heap.h"
#include "misc/list.h"
#include "reqs.h"
#include "reverse-proxy.h"
#include "self_contained/debugtrace.h"
#include "subservice/log.h"
#include "upstream.h"

/*
 * The configuration directives are defined in the structure below.  Each
 * directive requires a regular expression to match against, and a
 * function to call when the regex is matched.
 *
 * Below are defined certain constant regular expression strings that
 * can (and likely should) be used when building the regex for the
 * given directive.
 */
#define WS     "[[:space:]]+"
#define STR    "\"([^\"]+)\""
#define BOOL   "(yes|on|no|off)"
#define INT    "((0x)?[[:digit:]]+)"
#define ALNUM  "([-a-z0-9._]+)"
#define IP     "((([0-9]{1,3})\\.){3}[0-9]{1,3})"
#define IPMASK "(" IP "(/[[:digit:]]+)?)"
#define IPV6p1                                                                                     \
  "("                                                                                              \
  "(([0-9a-f]{1,4}:){1,1}(:[0-9a-f]{1,4}){1,6})|"                                                  \
  "(([0-9a-f]{1,4}:){1,2}(:[0-9a-f]{1,4}){1,5})|"                                                  \
  "(([0-9a-f]{1,4}:){1,3}(:[0-9a-f]{1,4}){1,4})"                                                   \
  ")"
#define IPV6p2                                                                                     \
  "("                                                                                              \
  "(([0-9a-f]{1,4}:){1,4}(:[0-9a-f]{1,4}){1,3})|"                                                  \
  "(([0-9a-f]{1,4}:){1,5}(:[0-9a-f]{1,4}){1,2})|"                                                  \
  "(([0-9a-f]{1,4}:){1,6}(:[0-9a-f]{1,4}){1,1})"                                                   \
  ")"
#define IPV6p3                                                                                     \
  "("                                                                                              \
  "((([0-9a-f]{1,4}:){1,7}|:):)|"                                                                  \
  "(:(:[0-9a-f]{1,4}){1,7})|"                                                                      \
  "([0-9a-f]{1,4}(:[0-9a-f]{1,4}){1,7})"                                                           \
  ")"
#define IPV6p4                                                                                     \
  "("                                                                                              \
  "(((([0-9a-f]{1,4}:){6})(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\.(25[0-5]|2[0-"                      \
  "4]\\d|[0-1]?\\d?\\d)){3}))|"                                                                    \
  "((([0-9a-f]{1,4}:){5}[0-9a-f]{1,4}:(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\.("                      \
  "25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3}))|"                                                        \
  "(([0-9a-f]{1,4}:){5}:[0-9a-f]{1,4}:(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\.("                      \
  "25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3})"                                                          \
  ")"
#define IPV6p5                                                                                     \
  "("                                                                                              \
  "(([0-9a-f]{1,4}:){1,1}(:[0-9a-f]{1,4}){1,4}:(25[0-5]|2[0-4]\\d|[0-1]?\\d?"                      \
  "\\d)(\\.(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3})|"                                                \
  "(([0-9a-f]{1,4}:){1,2}(:[0-9a-f]{1,4}){1,3}:(25[0-5]|2[0-4]\\d|[0-1]?\\d?"                      \
  "\\d)(\\.(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3})|"                                                \
  "(([0-9a-f]{1,4}:){1,3}(:[0-9a-f]{1,4}){1,2}:(25[0-5]|2[0-4]\\d|[0-1]?\\d?"                      \
  "\\d)(\\.(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3})"                                                 \
  ")"
#define IPV6p6                                                                                     \
  "("                                                                                              \
  "(([0-9a-f]{1,4}:){1,4}(:[0-9a-f]{1,4}){1,1}:(25[0-5]|2[0-4]\\d|[0-1]?\\d?"                      \
  "\\d)(\\.(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3})|"                                                \
  "((([0-9a-f]{1,4}:){1,5}|:):(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\.(25[0-5]|"                      \
  "2[0-4]\\d|[0-1]?\\d?\\d)){3})|"                                                                 \
  "(:(:[0-9a-f]{1,4}){1,5}:(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\.(25[0-5]|2[0-"                     \
  "4]\\d|[0-1]?\\d?\\d)){3})"                                                                      \
  ")"

#define IPV6MASKp1 "(" IPV6p1 "(/[[:digit:]]+)?)"
#define IPV6MASKp2 "(" IPV6p2 "(/[[:digit:]]+)?)"
#define IPV6MASKp3 "(" IPV6p3 "(/[[:digit:]]+)?)"
#define IPV6MASKp4 "(" IPV6p4 "(/[[:digit:]]+)?)"
#define IPV6MASKp5 "(" IPV6p5 "(/[[:digit:]]+)?)"
#define IPV6MASKp6 "(" IPV6p6 "(/[[:digit:]]+)?)"
#define BEGIN      "^[[:space:]]*"
#define END        "[[:space:]]*$"

/*
 * Limit the maximum number of substring matches to a reasonably high
 * number.  Given the usual structure of the configuration file, sixteen
 * substring matches should be plenty.
 */
#define RE_MAX_MATCHES 16

/*
 * All configuration handling functions are REQUIRED to be defined
 * with the same function template as below.
 */
typedef int (*CONFFILE_HANDLER)(struct config_s *, const char *, regmatch_t[]);

/*
 * Define the pattern used by any directive handling function.  The
 * following arguments are defined:
 *
 *   struct config_s* conf   pointer to the current configuration structure
 *   const char* line          full line matched by the regular expression
 *   regmatch_t match[]        offsets to the substrings matched
 *
 * The handling function must return 0 if the directive was processed
 * properly.  Any errors are reported by returning a non-zero value.
 */
#define HANDLE_FUNC(func) int func(struct config_s *conf, const char *line, regmatch_t match[])

/*
 * List all the handling functions.  These are defined later, but they need
 * to be in-scope before the big structure below.
 */
static HANDLE_FUNC(handle_nop)
{
  return 0;
} /* do nothing function */

static HANDLE_FUNC(handle_allow);
static HANDLE_FUNC(handle_basicauth);
static HANDLE_FUNC(handle_anonymous);
static HANDLE_FUNC(handle_bind);
static HANDLE_FUNC(handle_bindsame);
static HANDLE_FUNC(handle_connectport);
static HANDLE_FUNC(handle_defaulterrorfile);
static HANDLE_FUNC(handle_deny);
static HANDLE_FUNC(handle_errorfile);
static HANDLE_FUNC(handle_addheader);
#ifdef FILTER_ENABLE
static HANDLE_FUNC(handle_filter);
static HANDLE_FUNC(handle_filtercasesensitive);
static HANDLE_FUNC(handle_filterdefaultdeny);
static HANDLE_FUNC(handle_filterextended);
static HANDLE_FUNC(handle_filterurls);
#endif
static HANDLE_FUNC(handle_group);
static HANDLE_FUNC(handle_listen);
static HANDLE_FUNC(handle_logfile);
static HANDLE_FUNC(handle_loglevel);
static HANDLE_FUNC(handle_maxclients);
static HANDLE_FUNC(handle_maxrequestsperchild);
static HANDLE_FUNC(handle_maxspareservers);
static HANDLE_FUNC(handle_minspareservers);
static HANDLE_FUNC(handle_pidfile);
static HANDLE_FUNC(handle_port);
#ifdef REVERSE_SUPPORT
static HANDLE_FUNC(handle_reversebaseurl);
static HANDLE_FUNC(handle_reversemagic);
static HANDLE_FUNC(handle_reverseonly);
static HANDLE_FUNC(handle_reversepath);
#endif
static HANDLE_FUNC(handle_startservers);
static HANDLE_FUNC(handle_statfile);
static HANDLE_FUNC(handle_stathost);
static HANDLE_FUNC(handle_timeout);

static HANDLE_FUNC(handle_user);
static HANDLE_FUNC(handle_viaproxyname);
static HANDLE_FUNC(handle_disableviaheader);
static HANDLE_FUNC(handle_xtinyproxy);

#ifdef UPSTREAM_SUPPORT
static HANDLE_FUNC(handle_upstream);
static HANDLE_FUNC(handle_upstream_no);
#endif

static void config_free_regex(void);

/*
 * This macro can be used to make standard directives in the form:
 *   directive arguments [arguments ...]
 *
 * The directive itself will be the first matched substring.
 *
 * Note that this macro is not required.  As you can see below, the
 * comment and blank line elements are defined explicitly since they
 * do not follow the pattern above.  This macro is for convenience
 * only.
 */
#define STDCONF(d, re, func)                                                                       \
  {                                                                                                \
    BEGIN "(" d ")" WS re END, func, NULL                                                          \
  }

/*
 * Holds the regular expression used to match the configuration directive,
 * the function pointer to the routine to handle the directive, and
 * for internal use, a pointer to the compiled regex so it only needs
 * to be compiled one.
 */
struct
{
  const char *re;
  CONFFILE_HANDLER handler;
  regex_t *cre;
} directives[] = {
    /* comments */
    {BEGIN "#", handle_nop, NULL},
    /* blank lines */
    {"^[[:space:]]+$", handle_nop, NULL},
    /* string arguments */
    STDCONF("logfile", STR, handle_logfile),
    STDCONF("pidfile", STR, handle_pidfile),
    STDCONF("anonymous", STR, handle_anonymous),
    STDCONF("viaproxyname", STR, handle_viaproxyname),
    STDCONF("defaulterrorfile", STR, handle_defaulterrorfile),
    STDCONF("statfile", STR, handle_statfile),
    STDCONF("stathost", STR, handle_stathost),
    STDCONF("xtinyproxy", BOOL, handle_xtinyproxy),
    /* boolean arguments */
    STDCONF("bindsame", BOOL, handle_bindsame),
    STDCONF("disableviaheader", BOOL, handle_disableviaheader),
    /* integer arguments */
    STDCONF("port", INT, handle_port),
    STDCONF("maxclients", INT, handle_maxclients),
    STDCONF("maxspareservers", INT, handle_maxspareservers),
    STDCONF("minspareservers", INT, handle_minspareservers),
    STDCONF("startservers", INT, handle_startservers),
    STDCONF("maxrequestsperchild", INT, handle_maxrequestsperchild),
    STDCONF("timeout", INT, handle_timeout),
    STDCONF("connectport", INT, handle_connectport),
    /* alphanumeric arguments */
    STDCONF("user", ALNUM, handle_user),
    STDCONF("group", ALNUM, handle_group),
    /* ip arguments */
    STDCONF("listen", IP, handle_listen),
    STDCONF("listen", IPV6p1, handle_listen),
    STDCONF("listen", IPV6p2, handle_listen),
    STDCONF("listen", IPV6p3, handle_listen),
    STDCONF("listen", IPV6p4, handle_listen),
    STDCONF("listen", IPV6p5, handle_listen),
    STDCONF("listen", IPV6p6, handle_listen),
    STDCONF("allow",
            "("
            "(" IPMASK ")"
            "|" ALNUM ")",
            handle_allow),
    STDCONF("allow", IPV6MASKp1, handle_allow),
    STDCONF("allow", IPV6MASKp2, handle_allow),
    STDCONF("allow", IPV6MASKp3, handle_allow),
    STDCONF("allow", IPV6MASKp4, handle_allow),
    STDCONF("allow", IPV6MASKp5, handle_allow),
    STDCONF("allow", IPV6MASKp6, handle_allow),
    STDCONF("deny",
            "("
            "(" IPMASK ")"
            "|" ALNUM ")",
            handle_deny),
    STDCONF("deny", IPV6MASKp1, handle_deny),
    STDCONF("deny", IPV6MASKp2, handle_deny),
    STDCONF("deny", IPV6MASKp3, handle_deny),
    STDCONF("deny", IPV6MASKp4, handle_deny),
    STDCONF("deny", IPV6MASKp5, handle_deny),
    STDCONF("deny", IPV6MASKp6, handle_deny),
    STDCONF("bind", IP, handle_bind),
    STDCONF("bind", IPV6p1, handle_bind),
    STDCONF("bind", IPV6p2, handle_bind),
    STDCONF("bind", IPV6p3, handle_bind),
    STDCONF("bind", IPV6p4, handle_bind),
    STDCONF("bind", IPV6p5, handle_bind),
    STDCONF("bind", IPV6p6, handle_bind),
    /* other */
    STDCONF("basicauth", ALNUM WS ALNUM, handle_basicauth),
    STDCONF("errorfile", INT WS STR, handle_errorfile),
    STDCONF("addheader", STR WS STR, handle_addheader),

#ifdef FILTER_ENABLE
    /* filtering */
    STDCONF("filter", STR, handle_filter),
    STDCONF("filterurls", BOOL, handle_filterurls),
    STDCONF("filterextended", BOOL, handle_filterextended),
    STDCONF("filterdefaultdeny", BOOL, handle_filterdefaultdeny),
    STDCONF("filtercasesensitive", BOOL, handle_filtercasesensitive),
#endif
#ifdef REVERSE_SUPPORT
    /* Reverse proxy arguments */
    STDCONF("reversebaseurl", STR, handle_reversebaseurl),
    STDCONF("reverseonly", BOOL, handle_reverseonly),
    STDCONF("reversemagic", BOOL, handle_reversemagic),
    STDCONF("reversepath", STR "(" WS STR ")?", handle_reversepath),
#endif
#ifdef UPSTREAM_SUPPORT
    {BEGIN "(upstream)" WS "(none)" WS STR END, handle_upstream_no, NULL},
    {BEGIN "(upstream)" WS "(http|socks4|socks5)" WS
           "(" ALNUM /*username*/ ":" ALNUM /*password*/ "@"
           ")?"
           "(" IP "|" ALNUM ")"
           ":" INT "(" WS STR ")?" END,
     handle_upstream, NULL},
#endif
    /* loglevel */
    STDCONF("loglevel", "(critical|error|warning|notice|connect|info)", handle_loglevel)};

const unsigned int ndirectives = sizeof(directives) / sizeof(directives[0]);

static void free_added_headers(plist_t add_headers)
{
  TRACE_CALL_X(free_added_headers, "&add_headers = %p", add_headers);

  ssize_t i;

  for (i = 0; i < list_length(add_headers); i++)
  {
    http_header_t *header = (http_header_t *)list_getentry(add_headers, i, NULL);

    safefree(header->name);
    safefree(header->value);
  }

  list_delete(add_headers);
  TRACE_RET_VOID;
}

static void free_config(struct config_s *conf)
{
  TRACE_CALL_X(free_config, "%p", (void *)conf);

  safefree(conf->config_file);
  delete_pconf_log_t(&conf->log);
  safefree(conf->stathost);
  safefree(conf->user);
  safefree(conf->group);
  list_delete(conf->listen_addrs);
  list_delete(conf->basicauth_list);
#ifdef FILTER_ENABLE
  safefree(conf->filter);
#endif /* FILTER_ENABLE */
#ifdef REVERSE_SUPPORT
  free_reversepath_list(conf->reversepath_list);
  safefree(conf->reversebaseurl);
#endif
#ifdef UPSTREAM_SUPPORT
  free_upstream_list(conf->upstream_list);
#endif /* UPSTREAM_SUPPORT */
  safefree(conf->pidpath);
  safefree(conf->bind_address);
  safefree(conf->via_proxy_name);
  hashmap_delete(conf->errorpages);
  free_added_headers(conf->add_headers);
  safefree(conf->errorpage_undef);
  safefree(conf->statpage);
  flush_access_list(conf->access_list);
  free_connect_ports_list(conf->connect_ports);
  hashmap_delete(conf->anonymous_map);

  memset(conf, 0, sizeof(*conf));

  TRACE_RET_VOID;
}

/*
 * Compiles the regular expressions used by the configuration file.  This
 * routine MUST be called before trying to parse the configuration file.
 *
 * Returns 0 on success; negative upon failure.
 */
int config_compile_regex(void)
{
  TRACE_CALL(config_compile_regex);

  unsigned int i, r;

  for (i = 0; i != ndirectives; ++i)
  {
    assert(directives[i].handler);
    assert(!directives[i].cre);

    directives[i].cre = (regex_t *)safemalloc(sizeof(regex_t));
    if (!directives[i].cre)
    {
      TRACE_RETURN_X(-1, "directives[%d]: malloc failed", i);
    }

    r = regcomp(directives[i].cre, directives[i].re, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
    if (r)
    {
      TRACE_RETURN_X(r, "directives[%d]: regcomp failed", i);
    }
  }

  atexit(config_free_regex);

  TRACE_RETURN(0);
}

/*
 * Frees pre-compiled regular expressions used by the configuration
 * file. This function is registered to be automatically called at exit.
 */
static void config_free_regex(void)
{
  TRACE_CALL(config_free_regex);

  unsigned int i;

  for (i = 0; i < ndirectives; i++)
  {
    if (directives[i].cre)
    {
      regfree(directives[i].cre);
      safefree(directives[i].cre);
      directives[i].cre = NULL;
    }
  }

  TRACE_RET_VOID;
}

/*
 * Attempt to match the supplied line with any of the configuration
 * regexes defined above.  If a match is found, call the handler
 * function to process the directive.
 *
 * Returns 0 if a match was found and successfully processed; otherwise,
 * a negative number is returned.
 */
static int check_match(struct config_s *conf, const char *line)
{
  TRACE_CALL_X(check_match, "&conf = %p, line = ...", (void *)conf);

  regmatch_t match[RE_MAX_MATCHES];
  unsigned int i;

  assert(ndirectives > 0);

  for (i = 0; i != ndirectives; ++i)
  {
    assert(directives[i].cre);
    if (!regexec(directives[i].cre, line, RE_MAX_MATCHES, match, 0))
    {
      TRACE_MSG("handle directive[%d]", i);
      int r = (*directives[i].handler)(conf, line, match);
      TRACE_RETURN_X(r, "return code = %d for directives[%d].handler", r, i);
    }
  }

  TRACE_RETURN(-1);
}

/*
 * Parse the previously opened configuration stream.
 */
static int config_parse(struct config_s *conf, FILE *f)
{
  TRACE_CALL_X(config_parse, "&conf = %p, &file = %p", (void *)conf, (void *)f);

  char buffer[1024]; // 1KB lines should be plenty

  for (unsigned long lineno = 1; fgets(buffer, sizeof(buffer), f); ++lineno)
  {
    if (check_match(conf, buffer))
    {
      TRACE_RETURN_X(1, "Syntax error on line %ld: %s", lineno, buffer);
    }
  }

  TRACE_RETURN(0);
}

/**
 * Read the settings from a config file.
 */
static int load_config_file(const char *config_fname, struct config_s *conf)
{
  TRACE_CALL_X(load_config_file, "%s, %p", config_fname, (void *)conf);

  FILE *fconfig;

  fconfig = fopen(config_fname, "r");
  if (!fconfig)
  {
    TRACE_RETURN_X(-1, "Could not open config file \"%s\"", config_fname);
  }

  if (config_parse(conf, fconfig))
  {
    fclose(fconfig);
    TRACE_RETURN_X(-1, "Unable to parse config file \"%s\"", config_fname);
  }

  fclose(fconfig);
  TRACE_RETURN(0);
}

#define INIT_STRFLD_WITH_DEFAULT(fieldname)                                                        \
  if (defaults->fieldname)                                                                         \
  {                                                                                                \
    conf->fieldname = safestrdup(defaults->fieldname);                                             \
    if (!conf->fieldname)                                                                          \
    {                                                                                              \
      TRACE_CALL_X(-1, "conf->%s = %p", #fieldname, (void *)conf->fieldname);                       \
    }                                                                                              \
  }                                                                                                \
  do                                                                                               \
  {                                                                                                \
  } while (0)

static int initialize_with_defaults(struct config_s *conf, struct config_s *defaults)
{
  TRACE_CALL_X(initialize_with_defaults, "&conf = %p, &defaults = %p", (void *)conf,
              (void *)defaults);

  conf->log = clone_pconf_log_t(defaults->log);
  INIT_STRFLD_WITH_DEFAULT(config_file);
  INIT_STRFLD_WITH_DEFAULT(stathost);
  INIT_STRFLD_WITH_DEFAULT(user);
  INIT_STRFLD_WITH_DEFAULT(group);
  INIT_STRFLD_WITH_DEFAULT(pidpath);
  INIT_STRFLD_WITH_DEFAULT(bind_address);
  INIT_STRFLD_WITH_DEFAULT(via_proxy_name);
  INIT_STRFLD_WITH_DEFAULT(errorpage_undef);
  INIT_STRFLD_WITH_DEFAULT(statpage);

  conf->port = defaults->port;
  conf->quit = defaults->quit;
  conf->idletimeout = defaults->idletimeout;
  conf->bindsame = defaults->bindsame;
  conf->disable_viaheader = defaults->disable_viaheader;

  if (defaults->listen_addrs)
  {
    ssize_t i;

    conf->listen_addrs = list_create();
    for (i = 0; i < list_length(defaults->listen_addrs); i++)
    {
      char *addr;
      size_t size;
      addr = (char *)list_getentry(defaults->listen_addrs, i, &size);
      list_append(conf->listen_addrs, addr, size);
    }
  }

#ifdef FILTER_ENABLE
  INIT_STRFLD_WITH_DEFAULT(filter);

  conf->filter_url = defaults->filter_url;
  conf->filter_extended = defaults->filter_extended;
  conf->filter_casesensitive = defaults->filter_casesensitive;
#endif // FILTER_ENABLE

#ifdef XTINYPROXY_ENABLE
  conf->add_xtinyproxy = defaults->add_xtinyproxy;
#endif // XTINYPROXY_ENABLE

#ifdef REVERSE_SUPPORT
  INIT_STRFLD_WITH_DEFAULT(reversebaseurl);

  conf->reverseonly = defaults->reverseonly;
  conf->reversemagic = defaults->reversemagic;
#endif // REVERSE_SUPPORT

  TRACE_RETURN(0);
}

/**
 * Load the configuration.
 */
int try_load_config_file(const char *config_fname, struct config_s *conf, struct config_s *defaults)
{
  TRACE_CALL_X(try_load_config_file, "%s, &conf = %p, &defaults = %p", config_fname, (void *)conf,
              (void *)defaults);

  int ret;

  free_config(conf);
  if (initialize_with_defaults(conf, defaults))
  {
    TRACE_RETURN(-1);
  }

  ret = load_config_file(config_fname, conf);
  if (ret == 0)
  {
    TRACE_RETURN(0);
  }

  if (conf->port == 0)
  {
    TRACE_RETURN_X(-1, "conf->port = %d: You MUST set a Port in the config file.", conf->port);
  }

  // set the default values if they were not set in the config file
  conf->idletimeout = conf->idletimeout ? conf->idletimeout : MAX_IDLE_TIME;

  TRACE_RETURN(ret);
}

/***********************************************************************
 *
 * The following are basic data extraction building blocks that can
 * be used to simplify the parsing of a directive.
 *
 ***********************************************************************/

static char *get_string_arg(const char *line, regmatch_t *match)
{
  char *p;
  const unsigned int len = match->rm_eo - match->rm_so;

  assert(line);
  assert(len > 0);

  p = (char *)safemalloc(len + 1);
  if (!p)
    return NULL;

  memcpy(p, line + match->rm_so, len);
  p[len] = '\0';
  return p;
}

static int set_string_arg(char **var, const char *line, regmatch_t *match)
{
  TRACE_CALL_X(set_string_arg, "**var = %p, *line = %p, ...", (void*)var, (void *)line);
  char *arg = get_string_arg(line, match);

  if (!arg)
  {
    TRACE_RETURN_X(-1, "get_string_arg = %p", (void *)arg);
  }

  if (*var != NULL)
  {
    safefree(*var);
  }

  *var = arg;

  TRACE_RETURN(0);
}

static int get_bool_arg(const char *line, regmatch_t *match)
{
  const char *p = line + match->rm_so;

  assert(line);
  assert(match && match->rm_so != -1);

  /* "y"es or o"n" map as true, otherwise it's false. */
  if (tolower(p[0]) == 'y' || tolower(p[1]) == 'n')
    return 1;
  else
    return 0;
}

static int set_bool_arg(unsigned int *var, const char *line, regmatch_t *match)
{
  assert(var);
  assert(line);
  assert(match && match->rm_so != -1);

  *var = get_bool_arg(line, match);
  return 0;
}

static unsigned long get_long_arg(const char *line, regmatch_t *match)
{
  assert(line);
  assert(match && match->rm_so != -1);

  return strtoul(line + match->rm_so, NULL, 0);
}

static int set_int_arg(unsigned int *var, const char *line, regmatch_t *match)
{
  assert(var);
  assert(line);
  assert(match);

  *var = (unsigned int)get_long_arg(line, match);
  return 0;
}

/***********************************************************************
 *
 * Below are all the directive handling functions.  You will notice
 * that most of the directives delegate to one of the basic data
 * extraction routines.  This is deliberate.  To add a new directive
 * to tinyproxy only requires you to define the regular expression
 * above and then figure out what data extract routine to use.
 *
 * However, you will also notice that more complicated directives are
 * possible.  You can make your directive as complicated as you require
 * to express a solution to the problem you're tackling.
 *
 * See the definition/comment about the HANDLE_FUNC() macro to learn
 * what arguments are supplied to the handler, and to determine what
 * values to return.
 *
 ***********************************************************************/

static HANDLE_FUNC(handle_logfile)
{
  TRACE_MSG("conf->log = %p", (void*)conf->log);
  return set_string_arg(&conf->log->logf_name, line, &match[2]);
}

static HANDLE_FUNC(handle_pidfile)
{
  return set_string_arg(&conf->pidpath, line, &match[2]);
}

static HANDLE_FUNC(handle_anonymous)
{
  char *arg = get_string_arg(line, &match[2]);

  if (!arg)
  {
    return -1;
  }

  anonymous_insert(arg);
  safefree(arg);
  return 0;
}

static HANDLE_FUNC(handle_viaproxyname)
{
  TRACE_CALL(handle_viaproxyname);

  int r = set_string_arg(&conf->via_proxy_name, line, &match[2]);
  if (r)
  {
    TRACE_RETURN_X(r, "ret code: %d", r);
  }

  TRACE_RETURN_X(0, "Setting \"Via\" header to '%s'", conf->via_proxy_name);
}

static HANDLE_FUNC(handle_disableviaheader)
{
  TRACE_CALL(handle_disableviaheader);

  int r = set_bool_arg(&conf->disable_viaheader, line, &match[2]);
  if (r)
  {
    TRACE_RETURN_X(r, "ret code: %d", r);
  }

  TRACE_RETURN_X(0, "%s", "Disabling transmission of the \"Via\" header.");
}

static HANDLE_FUNC(handle_defaulterrorfile)
{
  return set_string_arg(&conf->errorpage_undef, line, &match[2]);
}

static HANDLE_FUNC(handle_statfile)
{
  return set_string_arg(&conf->statpage, line, &match[2]);
}

static HANDLE_FUNC(handle_stathost)
{
  TRACE_CALL(handle_stathost);

  int r = set_string_arg(&conf->stathost, line, &match[2]);
  if (r)
  {
    TRACE_RETURN_X(r, "ret code: %d", r);
  }

  TRACE_RETURN_X(0, "Stathost set to \"%s\"", conf->stathost);
}

static HANDLE_FUNC(handle_xtinyproxy)
{
  TRACE_CALL(handle_xtinyproxy);

#ifdef XTINYPROXY_ENABLE
  int r = set_bool_arg(&conf->add_xtinyproxy, line, &match[2]);
  TRACE_RETURN_X(r, "ret code: %d", r);
#else
  TRACE_RETURN_X(1, "%s", "XTinyproxy NOT Enabled! Recompile with --enable-xtinyproxy");
#endif
}

static HANDLE_FUNC(handle_bindsame)
{
  TRACE_CALL(handle_bindsame);

  int r = set_bool_arg(&conf->bindsame, line, &match[2]);
  if (r)
  {
    TRACE_RETURN_X(r, "ret code: %d", r);
  }

  TRACE_RETURN_X(0, "%s", "Binding outgoing connection to incoming IP");
}

static HANDLE_FUNC(handle_port)
{
  TRACE_CALL(handle_port);
  set_int_arg(&conf->port, line, &match[2]);

  if (conf->port > 65535)
  {
    TRACE_RETURN_X(1, "Bad port number (%d) supplied for Port.", conf->port);
  }

  TRACE_RETURN(0);
}

static HANDLE_FUNC(handle_maxclients)
{
  child_configure(CHILD_MAXCLIENTS, get_long_arg(line, &match[2]));
  return 0;
}

static HANDLE_FUNC(handle_maxspareservers)
{
  child_configure(CHILD_MAXSPARESERVERS, get_long_arg(line, &match[2]));
  return 0;
}

static HANDLE_FUNC(handle_minspareservers)
{
  child_configure(CHILD_MINSPARESERVERS, get_long_arg(line, &match[2]));
  return 0;
}

static HANDLE_FUNC(handle_startservers)
{
  child_configure(CHILD_STARTSERVERS, get_long_arg(line, &match[2]));
  return 0;
}

static HANDLE_FUNC(handle_maxrequestsperchild)
{
  child_configure(CHILD_MAXREQUESTSPERCHILD, get_long_arg(line, &match[2]));
  return 0;
}

static HANDLE_FUNC(handle_timeout)
{
  return set_int_arg(&conf->idletimeout, line, &match[2]);
}

static HANDLE_FUNC(handle_connectport)
{
  add_connect_port_allowed(get_long_arg(line, &match[2]), &conf->connect_ports);
  return 0;
}

static HANDLE_FUNC(handle_user)
{
  return set_string_arg(&conf->user, line, &match[2]);
}

static HANDLE_FUNC(handle_group)
{
  return set_string_arg(&conf->group, line, &match[2]);
}

static HANDLE_FUNC(handle_allow)
{
  char *arg = get_string_arg(line, &match[2]);

  insert_acl(arg, ACL_ALLOW, &conf->access_list);
  safefree(arg);
  return 0;
}

static HANDLE_FUNC(handle_deny)
{
  char *arg = get_string_arg(line, &match[2]);

  insert_acl(arg, ACL_DENY, &conf->access_list);
  safefree(arg);
  return 0;
}

static HANDLE_FUNC(handle_bind)
{
  TRACE_CALL(handle_bind);

  int r = set_string_arg(&conf->bind_address, line, &match[2]);
  if (r)
  {
    TRACE_RETURN_X(r, "ret code: %d", r);
  }

  TRACE_RETURN_X(0, "Outgoing connections bound to IP %s", conf->bind_address);
}

static HANDLE_FUNC(handle_listen)
{
  TRACE_CALL(handle_listen);

  char *arg = get_string_arg(line, &match[2]);
  if (arg == NULL)
  {
    TRACE_RETURN_X(-1, "%s", "get_string_arg == NULL");
  }

  if (conf->listen_addrs == NULL)
  {
    conf->listen_addrs = list_create();
    if (conf->listen_addrs == NULL)
    {
      safefree(arg);
      TRACE_RETURN_X(-1, "%s", "Could not create a list of listen addresses.");
    }
  }

  list_append(conf->listen_addrs, arg, strlen(arg) + 1);
  safefree(arg);

  TRACE_RETURN_X(0, "Added address [%s] to listen addresses.",
                (char *)list_getentry(conf->listen_addrs, list_length(conf->listen_addrs), NULL));
}

static HANDLE_FUNC(handle_errorfile)
{
  /*
   * Because an integer is defined as ((0x)?[[:digit:]]+) _two_
   * match places are used.  match[2] matches the full digit
   * string, while match[3] matches only the "0x" part if
   * present.  This is why the "string" is located at
   * match[4] (rather than the more intuitive match[3].
   */
  unsigned long int err = get_long_arg(line, &match[2]);
  char *page = get_string_arg(line, &match[4]);

  add_new_errorpage(page, err);
  safefree(page);
  return 0;
}

static HANDLE_FUNC(handle_addheader)
{
  char *name = get_string_arg(line, &match[2]);
  char *value = get_string_arg(line, &match[3]);
  http_header_t *header;

  if (!conf->add_headers)
  {
    conf->add_headers = list_create();
  }

  header = (http_header_t *)safemalloc(sizeof(http_header_t));
  header->name = name;
  header->value = value;

  list_prepend(conf->add_headers, header, sizeof *header);

  // don't free name or value here, as they are referenced in the struct inserted into the vector
  safefree(header);

  return 0;
}

// log level's strings
struct log_levels_s
{
  const char *string;
  int level;
};
static struct log_levels_s log_levels[] = {{"critical", LOG_CRIT},   {"error", LOG_ERR},
                                           {"warning", LOG_WARNING}, {"notice", LOG_NOTICE},
                                           {"connect", LOG_CONN},    {"info", LOG_INFO}};

static HANDLE_FUNC(handle_loglevel)
{
  static const unsigned int nlevels = sizeof(log_levels) / sizeof(log_levels[0]);
  unsigned int i;

  char *arg = get_string_arg(line, &match[2]);

  for (i = 0; i != nlevels; ++i)
  {
    if (!strcasecmp(arg, log_levels[i].string))
    {
      conf->log->log_level = log_levels[i].level;
      safefree(arg);
      return 0;
    }
  }

  safefree(arg);
  return -1;
}

static HANDLE_FUNC(handle_basicauth)
{
  char *user, *pass;
  user = get_string_arg(line, &match[2]);
  if (!user)
    return -1;
  pass = get_string_arg(line, &match[3]);
  if (!pass)
  {
    safefree(user);
    return -1;
  }
  if (!conf->basicauth_list)
  {
    conf->basicauth_list = list_create();
  }

  basicauth_add(conf->basicauth_list, user, pass);
  safefree(user);
  safefree(pass);
  return 0;
}

#ifdef FILTER_ENABLE
static HANDLE_FUNC(handle_filter)
{
  return set_string_arg(&conf->filter, line, &match[2]);
}

static HANDLE_FUNC(handle_filterurls)
{
  return set_bool_arg(&conf->filter_url, line, &match[2]);
}

static HANDLE_FUNC(handle_filterextended)
{
  return set_bool_arg(&conf->filter_extended, line, &match[2]);
}

static HANDLE_FUNC(handle_filterdefaultdeny)
{
  assert(match[2].rm_so != -1);

  if (get_bool_arg(line, &match[2]))
    filter_set_default_policy(FILTER_DEFAULT_DENY);
  return 0;
}

static HANDLE_FUNC(handle_filtercasesensitive)
{
  return set_bool_arg(&conf->filter_casesensitive, line, &match[2]);
}
#endif

#ifdef REVERSE_SUPPORT
static HANDLE_FUNC(handle_reverseonly)
{
  return set_bool_arg(&conf->reverseonly, line, &match[2]);
}

static HANDLE_FUNC(handle_reversemagic)
{
  return set_bool_arg(&conf->reversemagic, line, &match[2]);
}

static HANDLE_FUNC(handle_reversebaseurl)
{
  return set_string_arg(&conf->reversebaseurl, line, &match[2]);
}

static HANDLE_FUNC(handle_reversepath)
{
  // The second string argument is optional.
  char *arg1, *arg2;

  arg1 = get_string_arg(line, &match[2]);
  if (!arg1)
    return -1;

  if (match[4].rm_so != -1)
  {
    arg2 = get_string_arg(line, &match[4]);
    if (!arg2)
    {
      safefree(arg1);
      return -1;
    }
    reversepath_add(arg1, arg2, &conf->reversepath_list);
    safefree(arg1);
    safefree(arg2);
  }
  else
  {
    reversepath_add(NULL, arg1, &conf->reversepath_list);
    safefree(arg1);
  }
  return 0;
}
#endif

#ifdef UPSTREAM_SUPPORT

static enum proxy_type pt_from_string(const char *s)
{
  static const char pt_map[][7] = {
      [PT_NONE] = "none",
      [PT_HTTP] = "http",
      [PT_SOCKS4] = "socks4",
      [PT_SOCKS5] = "socks5",
  };
  unsigned i;
  for (i = 0; i < sizeof(pt_map) / sizeof(pt_map[0]); i++)
    if (!strcmp(pt_map[i], s))
      return i;
  return PT_NONE;
}

static HANDLE_FUNC(handle_upstream)
{
  char *ip;
  int port, mi = 2;
  char *domain = 0, *user = 0, *pass = 0, *tmp;
  enum proxy_type pt;

  tmp = get_string_arg(line, &match[mi]);
  pt = pt_from_string(tmp);
  safefree(tmp);
  mi += 2;

  if (match[mi].rm_so != -1)
    user = get_string_arg(line, &match[mi]);
  mi++;

  if (match[mi].rm_so != -1)
    pass = get_string_arg(line, &match[mi]);
  mi++;

  ip = get_string_arg(line, &match[mi]);
  if (!ip)
    return -1;
  mi += 5;

  port = (int)get_long_arg(line, &match[mi]);
  mi += 3;

  if (match[mi].rm_so != -1)
    domain = get_string_arg(line, &match[mi]);

  upstream_add(ip, port, domain, user, pass, pt, &conf->upstream_list);

  safefree(user);
  safefree(pass);
  safefree(domain);
  safefree(ip);

  return 0;
}

static HANDLE_FUNC(handle_upstream_no)
{
  char *domain;

  domain = get_string_arg(line, &match[3]);
  if (!domain)
    return -1;

  upstream_add(NULL, 0, domain, 0, 0, PT_NONE, &conf->upstream_list);
  safefree(domain);

  return 0;
}
#endif
