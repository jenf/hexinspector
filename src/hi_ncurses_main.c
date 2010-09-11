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


static void redraw(hi_ncurses *ncurses)
{
  hi_ncurses_fpager_redraw(ncurses->src);
  if (ncurses->dst)
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
  
  ncurses = malloc(sizeof(hi_ncurses));
  ncurses->diff = diff;
  
  (void) signal(SIGINT, finish);
  ncurses->window = initscr();
  keypad(stdscr, TRUE);
  nonl();
  cbreak();
  echo();
  refresh();
  wrefresh(mainwin);
  
  ncurses->src = hi_ncurses_fpager_new(ncurses, file, 75, 23, 0, 0);
  window = newwin(0,10,0,0);

  while (FALSE == quit)
  {
    redraw(ncurses);    
    newch = getch();
    switch (newch)
    {
      case 'q':
      case 'Q':
        quit = TRUE;
        break;
    }
  }
  
  finish(0);
}

