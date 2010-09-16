/* hi_ncurses_location.c: Hexinspector
   Copyright (c) 2010 Jen Freeman

   $Id$

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

   Author contact information:

   Web: http://github.com/jenf/hexinspector

*/

#include <hi_ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * Methods for showing the current location
 */
static GList *location_list;

static void hi_ncurses_location_define(const char *constructor_string,
                                       const char *name,
                                       int base)
{
  hi_location_mode *new;
  new = malloc(sizeof(hi_location_mode));
  new->constructor_string = strdup(constructor_string);
  new->name           = strdup(name);
  new->base           = base;
  
  location_list = g_list_append(location_list, new);
}

void hi_ncurses_location_init(void)
{
  hi_ncurses_location_define("%%0%ix", "Hex", 16);
  hi_ncurses_location_define("%%0%ii", "Dec", 10);
  hi_ncurses_location_define("%%0%io", "Oct", 8);
}

hi_location_mode *hi_ncurses_location_get(hi_location_mode *location,
                                          int relative)
{
  return hi_ncurses_common_get(location_list, location, relative);
}
