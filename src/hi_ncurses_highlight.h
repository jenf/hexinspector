/* hi_ncurses_highlight.h: Hexinspector
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

#ifndef HI_NCURSES_HIGHLIGHT_H
#define HI_NCURSES_HIGHLIGHT_H

/** Function pointers for highlighting */
typedef enum hi_ncurses_colour (*hi_ncurses_highlight_per_byte) (hi_file *file,
                                                                 off_t offset,
                                                                 unsigned char value,
                                                                 void *dataptr);

/** \brief Begin a redraw using a highlighter
 \return Memory to be used (which will be freed using hi_ncurses_highlight_end) */
typedef void *(*hi_ncurses_highlight_begin) (hi_file *file);
typedef void (*hi_ncurses_highlight_end) (void *dataptr);

typedef struct hi_ncurses_highlight
{
  hi_ncurses_highlight_per_byte highlight_func;
  hi_ncurses_highlight_begin    begin_func;
  hi_ncurses_highlight_end      end_func;
  char                          *name;
  char                          *help_string;
} hi_ncurses_highlight;

void hi_ncurses_highlight_init(void);
hi_ncurses_highlight *hi_ncurses_highlight_get(hi_ncurses_highlight *higlight,
                                               int relative);
#endif
