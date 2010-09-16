/* hi_ncurses_common.c: Hexinspector
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

#include <glib.h>

/**
 * Common utility functions
 */

void *hi_ncurses_common_get(GList *list,
                            void *to_find,
                            int relative)
{
  GList *item = NULL;
  if (to_find == NULL)
  {
    item = g_list_first(list);
  }
  else
  {
    item = g_list_find(list, to_find);
    if (relative >= 1)
    {
      item = g_list_next(item);
    }
    else
    {
      item = g_list_previous(item);
    }
    if (item == NULL)
    {
      if (relative >= 1)
      {
        item = g_list_first(list);
      }
      else
      {
        item = g_list_last(list);
      }
    }
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
