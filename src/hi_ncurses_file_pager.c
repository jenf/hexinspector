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

#define BYTES_FOR_BORDER (4)
#define WIDTH_PER_BYTE (3)
#define OFFSET_SIZE (8)
static void update_bytes_per_line(hi_ncurses_fpager *pager)
{
  /* TODO: add other output format mechanisms */
  pager->bytes_per_row = (pager->width-(OFFSET_SIZE+BYTES_FOR_BORDER))/WIDTH_PER_BYTE;
  
}

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
  pager->offset = 0;
  pager->window = newwin(height , width,y, x);
  update_bytes_per_line(pager);
  
  return pager;
}

void hi_ncurses_fpager_resize(hi_ncurses_fpager *pager,
                              int width, int height,
                              int y, int x)
{
  pager->x = x;
  pager->y = y;
  pager->width = width;
  pager->height = height;
  update_bytes_per_line(pager);
  wresize(pager->window,height, width);
  mvwin(pager->window, y, x);
  box(pager->window, 0, 0);
  werase(pager->window);
}

void hi_ncurses_fpager_redraw(hi_ncurses_fpager *pager)
{
  int x, y;
  unsigned char val;
  off_t offset;
  box(pager->window, ACS_VLINE, ACS_HLINE);
  
  for (y=0; y< pager->height-2; y++)
  {
    for (x=0; x<pager->bytes_per_row; x++)
    {
      offset = pager->offset+x+(pager->bytes_per_row*y);

      if (offset < pager->file->size)
      {
          if (x == 0)
          {
            mvwprintw(pager->window, y+1, 1, "%08x", offset);
          }
          val = pager->file->memory[offset];
          mvwprintw(pager->window,y+1,1+OFFSET_SIZE+(x*3),"%02x",val);         
      }
    }
  
  }

  wrefresh(pager->window);
}
