/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 1999 George Talusan <gstalusan@uwaterloo.ca>
 * Copyright (C) 2002 James E. Flemer <jflemer@acm.jhu.edu>
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

/* A substring of the domain to be filtered goes into the file
 * pointed at by DEFAULT_FILTER.
 */

#include <ctype.h>
#include <regex.h> // mingw +libsystre
#include <stdbool.h>
#include <stdio.h>

#include "misc/heap.h"
#include "misc/list.h"
#include "self_contained/safecall.h"
#include "subservice/filter.h"
#include "subservice/log.h"

#define FILTER_MAX_REGEX_LEN (500)

typedef struct
{
  char *regex_str;
  regex_t *compiled;
} filter_rule_t;

void clean_filter_rule_t(filter_rule_t *rule)
{
  safefree(rule->regex_str);
  safefree(rule->compiled);
}

struct filter_s
{
  pconf_filt_t config;
  plist_t rules;
};

CREATE_IMPL(pfilter_t, {
  obj->config = NULL;
  obj->rules = NULL;
  TRACE_SAFE_FIN(NULL == (obj->rules = list_create()), NULL, { delete_pfilter_t(&obj); });
  TRACE_SAFE_FIN(NULL == (obj->config = create_pconf_filt_t()), NULL, { delete_pfilter_t(&obj); });
})

DELETE_IMPL(pfilter_t, {
  ssize_t l = list_length(obj->rules);
  TRACE_SAFE_X(l < 0, -1, "%s", "assert l >= 0");
  for (size_t i = 0; i < ((size_t)(l)); ++i)
  {
    filter_rule_t *prule = list_getentry(obj->rules, i, NULL);
    clean_filter_rule_t(prule);
  }
  TRACE_SAFE_X(list_delete(obj->rules), -1, "%s", "list_delete");
  TRACE_SAFE_X(delete_pconf_filt_t(&obj->config), -1, "%s", "delete_pconf_filt_t");
})

bool is_enabled(pfilter_t filter)
{
  return filter->config->enabled;
}

pfilter_t create_configured_filter(pconf_filt_t filt_config)
{
  TRACE_CALL(create_configured_filter);
  TRACE_SAFE_X(NULL == filt_config, NULL, "%s", "there is no filt config");

  pfilter_t obj = NULL;
  TRACE_SAFE_R(NULL == (obj = create_pfilter_t()), NULL);
  TRACE_SAFE_FIN(delete_pconf_filt_t(&obj->config), NULL, { delete_pfilter_t(&obj); });
  TRACE_SAFE_FIN(NULL == (obj->config = clone_pconf_filt_t(filt_config)), NULL,
                 { delete_pfilter_t(&obj); });

  TRACE_RETURN(obj);
}

int bad_filter_init(filter_rule_t **bad_pprule, FILE *bad_file)
{
  clean_filter_rule_t(*bad_pprule);
  safefree(*bad_pprule);
  fclose(bad_file);
  return -1;
}

// initializes a linked list of strings containing hosts/urls to be filtered
int activate_filtering(plog_t log, pfilter_t filter)
{
  if (NULL == filter)
  {
    log_message(log, LOG_ERR, "%s", "there is no filter to activate");
    return -1;
  }
  if (!filter->config->enabled)
  {
    log_message(log, LOG_INFO, "%s", "filtering is not enabled by config");
    return 0;
  }
  { // check if list was already initialized
    ssize_t l = list_length(filter->rules);
    if (l > 0)
    {
      log_message(log, LOG_WARNING,
                  "filter is already activated "
                  "{file: \"%s\", rules_count: %z}",
                  filter->config->file_path, l);
      return 0;
    }
  }

  FILE *file;
  char line[FILTER_MAX_REGEX_LEN];
  int regex_flags;

  file = fopen(filter->config->file_path, "r");
  if (NULL == file)
  {
    log_message(log, LOG_ERR, "cannot open file with filter rules \"%s\"",
                filter->config->file_path);
    return -1;
  }

  regex_flags = REG_NEWLINE | REG_NOSUB;
  if (filter->config->is_extended)
  {
    regex_flags |= REG_EXTENDED;
  }

  // ATTENTION: inverse condition
  if (!filter->config->is_case_sensitive)
  {
    regex_flags |= REG_ICASE;
  }

  while (fgets(line, FILTER_MAX_REGEX_LEN, file))
  {
    { // forget any trailing white space and comments
      char *s = line;
      while (*s && (!(isspace((unsigned char)*s))) && *s != '#')
      {
        // break at first whitespace or '#'
        // note: not url or host can have spaces (it is always encoded)
        // note: not url or host can have '#' (it is always encoded)
        ++s;
      }
      *s = '\0';
    }

    // skip blank or comment line
    if (*line == '\0')
    {
      continue;
    }

    filter_rule_t *prule = NULL;
    prule = (filter_rule_t *)safemalloc(sizeof(filter_rule_t));
    if (NULL == prule)
    {
      log_message(log, LOG_ERR, "cannot alloc memory for filter rule (%s)", line);
      fclose(file);
      return -1;
    }

    prule->regex_str = NULL;
    prule->compiled = NULL;

    prule->regex_str = safestrdup(line);
    if (NULL == prule->regex_str)
    {
      log_message(log, LOG_ERR, "cannot alloc memory for filter rule.regex_str (%s)", line);
      return bad_filter_init(&prule, file);
    }

    prule->compiled = (regex_t *)safemalloc(sizeof(regex_t));
    if (NULL == prule->compiled)
    {
      log_message(log, LOG_ERR, "cannot alloc memory for filter rule.compiled (%s)", line);
      return bad_filter_init(&prule, file);
    }

    if (regcomp(prule->compiled, prule->regex_str, regex_flags))
    {
      log_message(log, LOG_ERR, "bad regex in %s (%s)", filter->config->file_path, line);
      return bad_filter_init(&prule, file);
    }

    if (list_append(filter->rules, prule, sizeof(filter_rule_t)))
    {
      log_message(log, LOG_ERR, "cannot add rule %s (%s)", filter->config->file_path, line);
      return bad_filter_init(&prule, file);
    }
  }

  if (ferror(file))
  {
    log_message(log, LOG_ERR, "fgets error with file \"%s\"", filter->config->file_path);
    return -1;
  }

  log_message(log, LOG_INFO, "successfully activated %z filter rules from \"%s\"",
              list_length(filter->rules), filter->config->file_path);
  fclose(file);
  return 0;
}

// return true to allow, false to block
bool does_string_pass_filter(plog_t log, pfilter_t filter, const char *s)
{
  if (NULL == s)
  {
    log_message(log, LOG_ERR, "%s", "cannot applying rule to empty string");
    return false;
  }

  ssize_t l = list_length(filter->rules);
  if (l < 0)
  {
    log_message(log, LOG_ERR, "cannot get rule's list length (%z)", l);
    return false;
  }

  for (size_t i = 0; i < ((size_t)(l)); ++i)
  {
    filter_rule_t *prule = list_getentry(filter->rules, i, NULL);

    if (!regexec(prule->compiled, s, 0, NULL, 0))
    {
      return filter->config->policy == FILTER_WHITE_LIST;
    }
  }

  return filter->config->policy != FILTER_WHITE_LIST;
}

// return true to allow, false to block
bool does_pass_filter(plog_t log, pfilter_t filter, const char *host, const char *url)
{
  if (filter->config->does_full_url_filtering)
  {
    return does_string_pass_filter(log, filter, url);
  }
  else
  {
    return does_string_pass_filter(log, filter, host);
  }
}
