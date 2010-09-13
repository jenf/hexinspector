/* hi_ncurses_highlight.c: Hexinspector
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

/**
 * Highlighting functions
 */

#include <hi_file.h>
#include <hi_ncurses.h>
#include <hi_ncurses_highlight.h>
#include <glib.h>

static GList *highlight_list;

/* C Type style */
enum hi_ncurses_colour highlight_ctype(hi_file *file,
                                       off_t offset,
                                       char val)
{
  if (isalpha(val))
    return hi_ncurses_colour_blue;
  if (!isprint(val))
    return hi_ncurses_colour_red;
  if (isdigit(val))
    return hi_ncurses_colour_green;
  if (isspace(val))
    return hi_ncurses_colour_yellow;
  return hi_ncurses_colour_normal;
}

void hi_ncurses_highlight_define(hi_ncurses_highlight_per_byte *highlighter,
                                 char *name,
                                 hi_ncurses_highlight_begin    *begin_func,
                                 hi_ncurses_highlight_end      *end_func)
{
  hi_ncurses_highlight *new;
  new = malloc(sizeof(hi_ncurses_highlight));
  new->highlight_func = highlighter;
  new->name           = strdup(name);
  new->begin_func     = begin_func;
  new->end_func       = end_func;
  
  g_list_append(highlight_list, new);
}

void hi_ncurses_highlight_init()
{

  hi_ncurses_highlight_define(highlight_ctype,"ctype",NULL,NULL);
  hi_ncurses_highlight_define(NULL,           "none", NULL,NULL);
}

hi_ncurses_highlight *hi_ncurses_highlight_get(hi_ncurses_highlight *highlight,
                                              int relative)
{
  GList *item;
  if (highlight == NULL)
  {
    item = g_list_first(highlight_list);
  }
  if (item == NULL)
  {
    return NULL;    
  }
  else
  {
    return item->data;    
  }
}
