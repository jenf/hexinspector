/* hi_ncurses_display: Hexinspector
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

#ifndef HI_NCURSES_DISPLAY_H
#define HI_NCURSES_DISPLAY_H

typedef int (*hi_display_bytes_per_line)(hi_ncurses_fpager *pager, int remaining_width);
typedef void (*hi_display_display_byte)(hi_ncurses_fpager *pager, int y, int start_x, int rowbyte, off_t offset, unsigned char value);

typedef struct hi_display_mode
{
  hi_display_bytes_per_line bytes_per_line_func;
  hi_display_display_byte   display_byte_func;
  char                     *name;
} hi_display_mode;

hi_display_mode *hi_ncurses_display_get(hi_display_mode *display,
                                        int relative);
void hi_ncurses_display_init(void);

#endif
