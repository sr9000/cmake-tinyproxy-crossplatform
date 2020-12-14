/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 1998 Steven Young <sdyoung@miranda.org>
 * Copyright (C) 1999-2005 Robert James Kaes <rjkaes@users.sourceforge.net>
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

#include "connect-ports.h"

#include "self_contained/debugtrace.h"
#include "subservice/log.h"

/*
 * Now, this routine adds a "port" to the list.  It also creates the list if
 * it hasn't already by done.
 */
int add_connect_port_allowed(int port, plist_t *connect_ports)
{
  TRACECALLEX(add_connect_port_allowed, "port = %d, &list = %p", port, (void *)connect_ports);
  if (!*connect_ports)
  {
    *connect_ports = list_create();
    if (!*connect_ports)
    {
      TRACERETURNEX(-1, "%s", "Could not create a list of allowed CONNECT ports");
    }
  }

  list_append(*connect_ports, &port, sizeof(port));
  TRACERETURN(0);
}

/*
 * This routine checks to see if a port is allowed in the CONNECT method.
 *
 * Returns: 1 if allowed
 *          0 if denied
 */
int check_allowed_connect_ports(int port, plist_t connect_ports)
{
  size_t i;
  int *data;

  /*
   * The absence of ConnectPort options in the config file
   * meanas that all ports are allowed for CONNECT.
   */
  if (!connect_ports)
    return 1;

  for (i = 0; i != (size_t)list_length(connect_ports); ++i)
  {
    data = (int *)list_getentry(connect_ports, i, NULL);
    if (data && *data == port)
      return 1;
  }

  return 0;
}

/**
 * Free a connect_ports list.
 */
void free_connect_ports_list(plist_t connect_ports)
{
  list_delete(connect_ports);
}
