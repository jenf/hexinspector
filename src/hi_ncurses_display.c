/* hi_ncurses_display.c: Hexinspector
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
 * Display mode subroutines
 */

#include <hi_ncurses.h>
#include <stdlib.h>
#include <macros.h>
#include <hi_ncurses_display.h>
#include <glib.h>
#include <string.h>
#include <ctype.h>

static GList *display_list;

/* Canonical mode */
static int canmode_bytes_per_line(unused(hi_ncurses_fpager *pager), int remaining_width)
{
  return (remaining_width-1)/4;
}

static void canmode_display_byte(hi_ncurses_fpager *pager, int y, int start_x, int rowbyte, off_t offset, unsigned char value)
{
  mvwprintw(pager->window,y,start_x+(rowbyte*3),"%02x",value);  
  mvwprintw(pager->window,y,start_x+((pager->remaining_bytes_per_row-1)/4*3)+rowbyte,"%c",isprint(value) ? value : '.');
}

/* Hex8 mode */
static int hex8mode_bytes_per_line(unused(hi_ncurses_fpager *pager), int remaining_width)
{
  return remaining_width/3;
}

static void hex8mode_display_byte(hi_ncurses_fpager *pager, int y, int start_x, int rowbyte, off_t offset, unsigned char value)
{
  mvwprintw(pager->window,y,start_x+(rowbyte*3),"%02x",value);  
}

/* Int8 mode */
static int int8mode_bytes_per_line(unused(hi_ncurses_fpager *pager), int remaining_width)
{
  return remaining_width/4;
}

static void int8mode_display_byte(hi_ncurses_fpager *pager, int y, int start_x, int rowbyte, off_t offset, unsigned char value)
{
  mvwprintw(pager->window,y,start_x+(rowbyte*4),"%3i",value);  
}

/* Oct8 mode */
static int oct8mode_bytes_per_line(unused(hi_ncurses_fpager *pager), int remaining_width)
{
  return remaining_width/4;
}

static void oct8mode_display_byte(hi_ncurses_fpager *pager, int y, int start_x, int rowbyte, off_t offset, unsigned char value)
{
  mvwprintw(pager->window,y,start_x+(rowbyte*4),"%03o",value);  
}

/* ASCII mode */
static int asciimode_bytes_per_line(unused(hi_ncurses_fpager *pager), int remaining_width)
{
  return (remaining_width);
}

static void asciimode_display_byte(hi_ncurses_fpager *pager, int y, int start_x, int rowbyte, off_t offset, unsigned char value)
{
  mvwprintw(pager->window,y,start_x+rowbyte,"%c",isprint(value) ? value : '.'); 
}

/* Bit mode */
static int bitmode_bytes_per_line(unused(hi_ncurses_fpager *pager), int remaining_width)
{
  return remaining_width/9;
}

static void bitmode_display_byte(hi_ncurses_fpager *pager, int y, int start_x, int rowbyte, off_t offset, unsigned char value)
{
  char str[9];
  convert_to_bitstring(value,str);
  mvwprintw(pager->window,y,start_x+(rowbyte*9),"%8s",str);  
}


/* Utility functions */
static void hi_ncurses_display_define(const char *name,
                                      hi_display_bytes_per_line    bytes_per_line_func,
                                      hi_display_display_byte      display_byte_func)
{
  hi_display_mode *new;
  new = malloc(sizeof(hi_display_mode));

  new->name           = strdup(name);
  new->bytes_per_line_func = bytes_per_line_func;
  new->display_byte_func   = display_byte_func;
  
  display_list = g_list_append(display_list, new);
}
void hi_ncurses_display_init(void)
{
  hi_ncurses_display_define("Hex8+ASCII", canmode_bytes_per_line, canmode_display_byte); 
  hi_ncurses_display_define("Hex8", hex8mode_bytes_per_line, hex8mode_display_byte);
  hi_ncurses_display_define("Oct8", oct8mode_bytes_per_line, oct8mode_display_byte);  
  hi_ncurses_display_define("Int8", int8mode_bytes_per_line, int8mode_display_byte);  
  hi_ncurses_display_define("ASCII", asciimode_bytes_per_line, asciimode_display_byte);  
  hi_ncurses_display_define("Binary",bitmode_bytes_per_line, bitmode_display_byte);  
}

hi_display_mode *hi_ncurses_display_get(hi_display_mode *display,
                                        int relative)
{
  return hi_ncurses_common_get(display_list, display, relative);
}
