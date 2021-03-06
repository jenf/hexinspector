/* hi_ncurses_main: Hexinspector
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
 * Ncurses main loop
 */

#include <hi_ncurses.h>
#include <signal.h>
#include <macros.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <hi_ncurses_display.h>
#include <string.h>

/* 4 byte ruler currently needs 105 characters */
#define RULERCOLS_32BIT (106)
#define RULER_LINES ((COLS) > RULERCOLS_32BIT ? 5: 6)

#define PAGER_WIDTH_SOLO (COLS)
#define PAGER_WIDTH_PAIR ((COLS/2))
#define PAGER_HEIGHT (LINES-RULER_LINES)

void convert_to_bitstring(int value, char *str)
{
  int i;
  str[8]=0;
  for (i=0; i < 8; i++)
  {
    str[7-i] = 0x30 + ((value >> i) & 1);
  }
}

static void hi_ncurses_redraw_ruler(hi_ncurses *ncurses)
{
  unsigned char value8;
  uint16_t value16_be,value16_le;
  uint32_t value32_be,value32_le;
  char bitstring[9];
  hi_file *file;
  off_t offset;
  char format_str[256];
  char pos1[256],pos2[256];
  
  offset = ncurses->focused_pager->offset;
  file = ncurses->focused_pager->file;
  

  
  werase(ncurses->ruler);

  if (offset < file->size)
  {
    value8 = file->memory[offset];
    convert_to_bitstring(value8, bitstring);
    mvwprintw(ncurses->ruler,0,0,"1b: %s/% 3u/%+ 4i/%#03o/0x%02x/'%c'",
              bitstring, value8, (signed char) value8, value8, value8,
              isprint(file->memory[offset]) ? file->memory[offset] : ' ');
  }
  if (offset+1 < file->size)
  {
    value16_be = file->memory[offset] | (file->memory[offset+1] << 8);
    value16_le = file->memory[offset+1] | (file->memory[offset] << 8);
    mvwprintw(ncurses->ruler,1,0,"2b: BE: % 5u/%+ 6i/%#07o/0x%04x"
              " LE: % 5u/%+ 6i/%#07o/0x%04x",
              value16_be, (int16_t)value16_be, value16_be, value16_be,
              value16_le, (int16_t)value16_le, value16_le, value16_le);
  }
  if (offset+3 < file->size)
  {
    value32_be = file->memory[offset] | (file->memory[offset+1] << 8) |
                 (file->memory[offset+2] << 16) | (file->memory[offset+3] << 24);
    value32_le = file->memory[offset+3] | (file->memory[offset+2] << 8) |
                  (file->memory[offset+1] << 16) | (file->memory[offset] << 24);    
    mvwprintw(ncurses->ruler,2,0,"4b: BE: % 10u/%+ 11i/%#012o/0x%08x",
              value32_be, (int32_t)value32_be, value32_be, value32_be);
    mvwprintw(ncurses->ruler,RULER_LINES-3,(COLS) > RULERCOLS_32BIT ? 55 : 0,
              "4b: LE: % 10u/%+11i/%#012o/0x%08x",
              value32_le, (int32_t)value32_le, value32_le, value32_le);
  }
  
  snprintf(format_str,256,ncurses->focused_pager->location_mode->constructor_string, ncurses->focused_pager->bytes_in_location);

  snprintf(pos1, 256, format_str, (unsigned int) offset);
  snprintf(pos2, 256, format_str, (unsigned int) file->size);
  mvwprintw(ncurses->ruler,RULER_LINES-2,0,"Position(%s): %s/%s (%.2f%%)",
            ((ncurses->focused_pager->location_mode != NULL) && (ncurses->focused_pager->location_mode->name != NULL)) ? ncurses->focused_pager->location_mode->name : "",
            pos1, pos2,
            (((double) offset)/file->size)*100);  
  if (ncurses->error != NULL)
  {
    ncurses->activate_bell = TRUE;
    wattron(ncurses->ruler, A_REVERSE);
    mvwprintw(ncurses->ruler, RULER_LINES-1,0, "%s", ncurses->error);
    wattroff(ncurses->ruler, A_REVERSE);
    ncurses->error = NULL;
  }
  else
  {
    mvwprintw(ncurses->ruler,RULER_LINES-1,0,"%s \"%s\" %s %s: Press ? for help",
              ncurses->mode == MODE_NORMAL ? "Buffer:" : "RegEx:",
              ncurses->buffer,
              ((ncurses->focused_pager->highlighter != NULL) && (ncurses->focused_pager->highlighter->name != NULL)) ? ncurses->focused_pager->highlighter->name : "",
              ((ncurses->focused_pager->display_mode != NULL) && (ncurses->focused_pager->display_mode->name != NULL)) ? ncurses->focused_pager->display_mode->name : "");
  }
  wrefresh(ncurses->ruler);
}

static void redraw(hi_ncurses *ncurses, gboolean need_resize)
{
  if (need_resize)
  {
    if (ncurses->dst != NULL)
    {
      hi_ncurses_fpager_resize(ncurses->src, PAGER_HEIGHT, PAGER_WIDTH_PAIR, 0, 0);     
      hi_ncurses_fpager_resize(ncurses->dst, PAGER_HEIGHT, PAGER_WIDTH_PAIR, 0, PAGER_WIDTH_PAIR);  
    }
    else
    {
      hi_ncurses_fpager_resize(ncurses->src, PAGER_HEIGHT, PAGER_WIDTH_SOLO, 0, 0);           
    }
    
    hi_ncurses_help_resize(ncurses);
    
    wresize(ncurses->ruler, RULER_LINES, COLS);
    mvwin(ncurses->ruler, PAGER_HEIGHT, 0);
    erase();
    refresh();
  }
 
  hi_ncurses_fpager_redraw(ncurses->src);
  if (ncurses->dst != NULL)
    hi_ncurses_fpager_redraw(ncurses->dst);

  hi_ncurses_redraw_ruler(ncurses);

  hi_ncurses_help_redraw(ncurses);
  if (ncurses->activate_bell == TRUE)
  {
    beep();
  }
  refresh();
}


static void finish(unused(int sig))
{
  endwin();
  exit(0);
}



void hi_ncurses_main(hi_file *file, hi_file *file2, hi_diff *diff)
{
  hi_ncurses *ncurses;
  int newch;
  gboolean quit = FALSE;
  gboolean need_resize = FALSE;
  gboolean key_claimed;
  long long buffer_val = 0;
  int len;
  
  ncurses = malloc(sizeof(hi_ncurses));
  ncurses->dst = NULL;
  ncurses->diff = diff;
  ncurses->buffer[0]=0;
  ncurses->mode = MODE_NORMAL;
  ncurses->error = NULL;
  
  (void) signal(SIGINT, finish);
  ncurses->window = initscr();
  start_color();
  init_pair(hi_ncurses_colour_normal,COLOR_WHITE  ,COLOR_BLACK);
  init_pair(hi_ncurses_colour_red ,COLOR_RED  ,COLOR_BLACK);
  init_pair(hi_ncurses_colour_blue,COLOR_CYAN ,COLOR_BLACK);
  init_pair(hi_ncurses_colour_green,COLOR_GREEN,COLOR_BLACK);
  init_pair(hi_ncurses_colour_yellow,COLOR_YELLOW,COLOR_BLACK);
  hi_ncurses_highlight_init();
  hi_ncurses_display_init();
  hi_ncurses_location_init();
  hi_ncurses_help_init(ncurses);
  
  keypad(stdscr, TRUE);
  nonl();
  cbreak();
  noecho();

  refresh();
  ncurses->big_hunk_size=64;

  
  ncurses->ruler = newwin(RULER_LINES, 0, PAGER_HEIGHT, 0);
  
  if (file2 != NULL)
  {
    ncurses->src = hi_ncurses_fpager_new(ncurses, file,  diff, PAGER_HEIGHT, PAGER_WIDTH_PAIR,  0, 0);
    ncurses->dst = hi_ncurses_fpager_new(ncurses, file2, diff, PAGER_HEIGHT, PAGER_WIDTH_PAIR,  0, PAGER_WIDTH_PAIR);    
    ncurses->src->linked_pager = ncurses->dst;
    ncurses->dst->linked_pager = ncurses->src;
  }
  else
  {
    ncurses->src = hi_ncurses_fpager_new(ncurses, file,  diff, PAGER_HEIGHT, PAGER_WIDTH_SOLO,  0, 0); 
  }
  ncurses->focused_pager = ncurses->src;


  while (FALSE == quit)
  {
    redraw(ncurses, need_resize);    
    need_resize = FALSE;
    ncurses->activate_bell = FALSE;
    buffer_val = strtoll(ncurses->buffer, NULL, 0);  
    
    /* Special case imply 1 if there is no buffer */
    if (ncurses->buffer[0]==0)
    {
      buffer_val = 1;
    }
    
    newch = getch();
    key_claimed = FALSE;
    
    if (ncurses->show_help)
    {
      key_claimed = hi_ncurses_help_key_event(ncurses, newch);
    }
    if (key_claimed == FALSE)
    { 
      switch (newch)
      {
        case 27: /* Escape key, clear the buffer */
          ncurses->buffer[0] = 0;
          key_claimed = TRUE;
          ncurses->mode = MODE_NORMAL;
          break;
          
        case 127:
        case '\b':
        case KEY_BACKSPACE:
          len = strlen(ncurses->buffer);
          if (len >= 1)
          {
            ncurses->buffer[len-1] = 0;
          }
          key_claimed = TRUE;
          break;
      }
    }
    
    /* Soak up all the key presses */
    if (key_claimed == FALSE)
    {
      if (ncurses->mode == MODE_REGEX)
      {
        if (newch == '\n' || newch == '\r')
        {
          hi_ncurses_fpager_search(ncurses->focused_pager, ncurses->buffer);
          ncurses->buffer[0]=0;
          ncurses->mode = MODE_NORMAL;
        }
        else
        {
          len = strlen(ncurses->buffer);
          if (len+1 < KEYBUFFER_LEN)
          {
            ncurses->buffer[len]=newch;
            ncurses->buffer[len+1]=0;          
          }
        }
        key_claimed = TRUE;
      }
    }
    
    if (key_claimed == FALSE)
    {
      if (ncurses->focused_pager->linked_pager != NULL)
      {
        key_claimed = hi_ncurses_fpager_slave_key_event(ncurses->focused_pager->linked_pager, newch);
      }
    }
    if (key_claimed == FALSE)
    {
      key_claimed = hi_ncurses_fpager_key_event(ncurses->focused_pager, newch, buffer_val);
    }
    
    if (FALSE == key_claimed)
    {
      if (newch < 255 && (isxdigit(newch) || newch=='x' || newch=='-'))
      {
        len = strlen(ncurses->buffer);
        if (len+1 < KEYBUFFER_LEN)
        {
          ncurses->buffer[len]=newch;
          ncurses->buffer[len+1]=0;          
        }
        key_claimed = TRUE;
      }
    }
    
    if (FALSE == key_claimed)
    {
      switch (newch)
      {
        case 'q':
        case 'Q':
          quit = TRUE;
          break;
        

          
        case 'P':
        case 'p':
          if (ncurses->dst)
            ncurses->focused_pager = (ncurses->src==ncurses->focused_pager ? ncurses->dst : ncurses->src);
          break;
          
        case 47 /* / */:
          ncurses->mode = MODE_REGEX;
          break;

        case KEY_RESIZE:
          /* Need to resize the pagers */
          need_resize = TRUE;
          break;


        case '?': /* Show help */
          ncurses->help_win_line=0; /* Force the help to line one when opened */
          ncurses->show_help = !ncurses->show_help;
          break;
          
        case ERR:
          break;
        default:
          /* Unknown key */
#if 0
          fprintf(stderr,"Unknown key %i\n",  newch);
#endif
          ncurses->activate_bell = TRUE;
      }
    }
  }
  
  finish(0);
}

