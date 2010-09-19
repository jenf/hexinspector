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
#include <hi_ncurses_highlight.h>

#define KEYBUFFER_LEN (256)
enum hi_ncurses_colour
{
  hi_ncurses_colour_dummy,
  hi_ncurses_colour_normal,
  hi_ncurses_colour_red,
  hi_ncurses_colour_blue,
  hi_ncurses_colour_green,
  hi_ncurses_colour_yellow
};


typedef struct hi_location_mode
{
  char *constructor_string;
  char *name;
  int base;
} hi_location_mode;

typedef struct hi_ncurses_fpager
{
  hi_file *file;
  hi_diff *diff;
  struct hi_ncurses *curses;
  off_t    offset;
  WINDOW  *window;
  int      width;
  int      bytes_per_row;
  int      set_bytes_per_row;
  int      remaining_bytes_per_row;
  int      height;
  int      x;
  int      y;
  struct hi_ncurses_fpager *linked_pager;
  /* Declared later */
  struct hi_display_mode   *display_mode;
  hi_location_mode         *location_mode;
  hi_ncurses_highlight     *highlighter;
  int      bytes_in_location;
  int      byte_grouping;
} hi_ncurses_fpager;

typedef struct hi_ncurses
{
  hi_ncurses_fpager *src;
  hi_ncurses_fpager *dst;
  hi_ncurses_fpager *focused_pager;
  char                  buffer[KEYBUFFER_LEN];
  hi_diff               *diff;
  WINDOW                *window;
  WINDOW                *ruler;
  int      big_hunk_size;
  gboolean activate_bell;
} hi_ncurses;





void hi_ncurses_main(hi_file *file, hi_file *file2, hi_diff *diff);
hi_ncurses_fpager *hi_ncurses_fpager_new(hi_ncurses *curses,
                                         hi_file *file,
                                         hi_diff *diff,
                                         int height, int width,
                                         int y, int x);
void hi_ncurses_fpager_redraw(hi_ncurses_fpager *pager);
void hi_ncurses_fpager_resize(hi_ncurses_fpager *pager,
                              int height, int width,
                              int y, int x);
hi_ncurses_fpager_search(hi_ncurses_fpager *pager, char *search);
gboolean hi_ncurses_fpager_key_event(hi_ncurses_fpager *pager,
                                     int key,
                                     long long buffer_val);
gboolean hi_ncurses_fpager_slave_key_event(hi_ncurses_fpager *pager,
                                           int key);
void convert_to_bitstring(int value, char *str);
void hi_ncurses_location_init(void);

void *hi_ncurses_common_get(GList *list,
                            void *to_find,
                            int relative);
hi_location_mode *hi_ncurses_location_get(hi_location_mode *location,
                                          int relative);

#endif
