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
                                        hi_ncurses_highlight_block    block_func,
                                        const char *help_string);
static void *mpegts_highlight_begin(hi_file *file);
static void mpegts_highlight_end(void *mem);
static enum hi_ncurses_colour mpeg_highlight(unused(hi_file *file),
                                             unused(off_t offset),
                                             unsigned char val,
                                             void *dataptr);

/* C Type style */
static void highlight_ctype_block(hi_file *file,
                                  off_t offset,
                                  size_t len,
                                  enum hi_ncurses_colour *window)
{
	int i=0;
	int val;
	for (i=0; i<len; i++) {
		val = file->memory[offset+i];
		if (isalpha(val))
			window[i]=hi_ncurses_colour_blue;
		else if (!isprint(val))
			window[i]=hi_ncurses_colour_red;
		else if (isdigit(val))
			window[i]=hi_ncurses_colour_green;
		else if (isspace(val))
			window[i]=hi_ncurses_colour_yellow;
	}
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

static void highlight_mpeg2_video_block(hi_file *file,
										off_t offset,
									    size_t len,
									    enum hi_ncurses_colour *window)
{
	int i=0;
	int val;
	enum hi_ncurses_colour colour = hi_ncurses_colour_normal;
	unsigned char match[3]={0,0,1};
	for (i=0; i<len; i++) {
		if (i+4 < len) {
			if (file->memory[offset+i]==0 && file->memory[offset+i+1]==0 && file->memory[offset+i+2]==1)
			{
				int start_code = file->memory[offset+i+3];
				switch(start_code) {
				case 0x00:
					colour = hi_ncurses_colour_blue;
					break;
				case 0xB3:
					colour = hi_ncurses_colour_red;
					break;
				default:
					if (start_code >= 0x01 && start_code <= 0xAF)
					{
						colour = hi_ncurses_colour_green;
					}
					else
					{
						colour = hi_ncurses_colour_normal;
					}
				}

				/* Highligh start code */
				window[i++] = hi_ncurses_colour_yellow;
				window[i++] = hi_ncurses_colour_yellow;
				window[i++] = hi_ncurses_colour_yellow;
			}
		}
		window[i] = colour;
	}
}

static void hi_ncurses_highlight_define(hi_ncurses_highlight_per_byte highlighter,
                                        const char *name,
                                        hi_ncurses_highlight_begin    begin_func,
                                        hi_ncurses_highlight_end      end_func,
                                        hi_ncurses_highlight_block    block_func,
                                        const char *help_string)
{
  hi_ncurses_highlight *new;
  new = malloc(sizeof(hi_ncurses_highlight));
  new->highlight_func = highlighter;
  new->name           = strdup(name);
  new->begin_func     = begin_func;
  new->end_func       = end_func;
  new->block_func     = block_func;
  new->help_string    = strdup(help_string);
  
  highlight_list = g_list_append(highlight_list, new);
}

void hi_ncurses_highlight_init(void)
{

  hi_ncurses_highlight_define(NULL,"ctype",NULL,NULL,highlight_ctype_block,"Blue = Numbers, Red = Non-printable, Green = Decimal numbers, Yellow = Space");
  hi_ncurses_highlight_define(NULL,"none", NULL,NULL,NULL, "No highlighting");
  hi_ncurses_highlight_define(mpeg_highlight, "mpeg", mpegts_highlight_begin,mpegts_highlight_end,NULL, "Green = Tables, Yellow = AV/Private, Red = NULL");
  hi_ncurses_highlight_define(NULL, "mpeg2-video", NULL, NULL, highlight_mpeg2_video_block, "Blue = Packet");
}

hi_ncurses_highlight *hi_ncurses_highlight_get(hi_ncurses_highlight *highlight,
                                              int relative)
{
  return hi_ncurses_common_get(highlight_list, highlight, relative);
}
