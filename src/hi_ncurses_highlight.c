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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <macros.h>

static GList *highlight_list;

static void hi_ncurses_highlight_define(hi_ncurses_highlight_per_byte highlighter,
                                        const char *name,
                                        hi_ncurses_highlight_begin    begin_func,
                                        hi_ncurses_highlight_end      end_func,
                                        const char *help_string);
static void *mpegts_highlight_begin(hi_file *file);
static void mpegts_highlight_end(void *mem);
static enum hi_ncurses_colour mpeg_highlight(unused(hi_file *file),
                                             unused(off_t offset),
                                             unsigned char val,
                                             void *dataptr);
static enum hi_ncurses_colour highlight_ctype(unused(hi_file *file),
                                              unused(off_t offset),
                                              unsigned char val,
                                              unused(void *dataptr));

/* C Type style */
static enum hi_ncurses_colour highlight_ctype(unused(hi_file *file),
                                              unused(off_t offset),
                                              unsigned char val,
                                              unused(void *dataptr))
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

/* MPEG-TS style, this isn't perfect */
static void *mpegts_highlight_begin(unused(hi_file *file))
{
  enum hi_ncurses_colour *pos;
  pos = malloc(sizeof(enum hi_ncurses_colour));
  *pos=hi_ncurses_colour_normal;
  return pos;
}

static void mpegts_highlight_end(void *mem)
{
  free(mem);
}

static enum hi_ncurses_colour mpeg_highlight(unused(hi_file *file),
                                             unused(off_t offset),
                                             unsigned char val,
                                             void *dataptr)
{
  int pid;
  enum hi_ncurses_colour *col = dataptr;
  
  if (0x47==val)
  {
    if (offset+2 < file->size)
    {
      pid = ((file->memory[offset+1] << 16) | (file->memory[offset+2])) & 0x1fff;
      if (pid <0x1F)
      {
        *col = hi_ncurses_colour_green;
      }
      else if (pid < 0x1fff)
      {
        *col = hi_ncurses_colour_yellow;
      }
      else
      {
        *col = hi_ncurses_colour_red;
      }
    }
    return hi_ncurses_colour_blue;
  }
  else
  {
    return *col;
  }
  return hi_ncurses_colour_normal;
}

static void hi_ncurses_highlight_define(hi_ncurses_highlight_per_byte highlighter,
                                        const char *name,
                                        hi_ncurses_highlight_begin    begin_func,
                                        hi_ncurses_highlight_end      end_func,
                                        const char *help_string)
{
  hi_ncurses_highlight *new;
  new = malloc(sizeof(hi_ncurses_highlight));
  new->highlight_func = highlighter;
  new->name           = strdup(name);
  new->begin_func     = begin_func;
  new->end_func       = end_func;
  new->help_string    = strdup(help_string);
  
  highlight_list = g_list_append(highlight_list, new);
}

void hi_ncurses_highlight_init(void)
{

  hi_ncurses_highlight_define(highlight_ctype,"ctype",NULL,NULL,"Blue = Numbers, Red = Non-printable, Green = Decimal numbers, Yellow = Space");
  hi_ncurses_highlight_define(NULL,           "none", NULL,NULL,"No highlighting");
  hi_ncurses_highlight_define(mpeg_highlight, "mpeg", mpegts_highlight_begin,mpegts_highlight_end,"Green = Tables, Yellow = AV/Private, Red = NULL");
}

hi_ncurses_highlight *hi_ncurses_highlight_get(hi_ncurses_highlight *highlight,
                                              int relative)
{
  return hi_ncurses_common_get(highlight_list, highlight, relative);
}
