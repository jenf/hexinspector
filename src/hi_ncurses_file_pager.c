/* hi_ncurses_file_pager.c: Hexinspector
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
 * Ncurses file pager
 */
#include <hi_ncurses.h>
#include <stdlib.h>
#include <macros.h>

hi_ncurses_fpager *hi_ncurses_fpager_new(hi_ncurses *curses,
                                         hi_file *file,
                                         int width, int height,
                                         int y, int x)
{
  hi_ncurses_fpager *pager;
  pager = malloc(sizeof(hi_ncurses_fpager));
  if (NULL == pager)
  {
    DPRINTF("Could not allocate pager\n");
    return NULL;
  }
  
  pager->file = file;
  pager->width = width;
  pager->height = height;
  pager->x = x;
  pager->y = y;
  pager->window = newwin(width, height, y, x);

  return pager;
}

void hi_ncurses_fpager_redraw(hi_ncurses_fpager *pager)
{
  mvwprintw(pager->window,1,1,"Hi there");
  box(pager->window, ACS_VLINE, ACS_HLINE);
  
  wrefresh(pager->window);
}
