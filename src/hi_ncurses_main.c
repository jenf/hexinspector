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

#define PAGER_WIDTH_SOLO (COLS)
#define PAGER_WIDTH_PAIR ((COLS/2)-2)
#define PAGER_HEIGHT (LINES-RULER_LINES)
#define RULER_LINES (5)
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

    erase();
    refresh();
  }
  
  hi_ncurses_fpager_redraw(ncurses->src);
  if (ncurses->dst != NULL)
    hi_ncurses_fpager_redraw(ncurses->dst);
  
  refresh();
}


static void finish(int sig)
{
  endwin();
  exit(0);
}


void hi_ncurses_main(hi_file *file, hi_file *file2, hi_diff *diff)
{
  WINDOW *window;
  WINDOW *mainwin;
  hi_ncurses *ncurses;
  int newch;
  gboolean quit = FALSE;
  gboolean need_resize = FALSE;
  gboolean key_claimed;
  char buffer[256];
  
  ncurses = malloc(sizeof(hi_ncurses));
  ncurses->dst = NULL;
  ncurses->diff = diff;
  
  (void) signal(SIGINT, finish);
  ncurses->window = initscr();
  start_color();
  init_pair(hi_ncurses_colour_diff,COLOR_BLACK,COLOR_RED);
  init_pair(hi_ncurses_colour_red ,COLOR_RED  ,COLOR_BLACK);
  init_pair(hi_ncurses_colour_blue,COLOR_BLUE ,COLOR_BLACK);
  init_pair(hi_ncurses_colour_green,COLOR_GREEN,COLOR_BLACK);
  
  keypad(stdscr, TRUE);
  nonl();
  cbreak();
  echo();

  
  refresh();
  


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
    
    newch = getch();
    key_claimed = hi_ncurses_fpager_key_event(ncurses->focused_pager, newch, buffer, 256);
    
    if (FALSE == key_claimed)
    {
      switch (newch)
      {
        case 'q':
        case 'Q':
          quit = TRUE;
          break;
          
        case 'F':
        case 'f':
          if (ncurses->dst)
            ncurses->focused_pager = (ncurses->src==ncurses->focused_pager ? ncurses->dst : ncurses->src);
          break;
          
          /* Just temporary */

        case KEY_RESIZE:
          /* Need to resize the pagers */
          need_resize = TRUE;
          break;
      }
    }
  }
  
  finish(0);
}

