/* <filename>: Hexinspector
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
 * Structure overview
 */

#ifndef HI_NCURSES_H
#define HI_NCURSES_H

#include <ncurses.h>
#include <hi_file.h>
#include <hi_diff.h>

enum hi_ncurses_colour
{
  hi_ncurses_colour_normal =0,
  hi_ncurses_colour_diff   =1
};

typedef struct hi_ncurses_fpager
{
  hi_file *file;
  off_t    offset;
  WINDOW  *window;
  int      width;
  int      bytes_per_row;
  int      height;
  int      x;
  int      y;
} hi_ncurses_fpager;

typedef struct hi_ncurses
{
  hi_ncurses_fpager *src;
  hi_ncurses_fpager *dst;
  hi_diff               *diff;
  WINDOW                *window;
} hi_ncurses;

void hi_ncurses_main(hi_file *file, hi_file *file2, hi_diff *diff);
hi_ncurses_fpager *hi_ncurses_fpager_new(hi_ncurses *curses,
                                         hi_file *file,
                                         int height, int width,
                                         int y, int x);
void hi_ncurses_fpager_redraw(hi_ncurses_fpager *pager);
void hi_ncurses_fpager_resize(hi_ncurses_fpager *pager,
                              int height, int width,
                              int y, int x);
#endif
