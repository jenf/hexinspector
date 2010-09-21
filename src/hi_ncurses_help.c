/* hi_ncurses_help.c Hexinspector
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
 * NCurses help display
 */
#include <hi_ncurses.h>
#include <string.h>
const char *help_data [] = {
"*Buffer",
"Certain commands can be prefixed with a 'buffer' argument, this takes an",
"integer in decimal, octal (preceeded by 0), or hex (preceeded by 0x) formats",
"",
"Escape                       | Clear the buffer",
"Backspace                    | Remove the last character from buffer",
"",
"*All mode keys",
"q/Q                          | Quit",
"?                            | This help (Press to remove)",
"[buffer] Page Up/Down/Arrows | Move around the file",
"[buffer] g/G                 | Goto (Negative value in buffer)",
"                               act as bytes from EOF",
"/                            | Search using a regular expression",
"",
"*Display modes",
"Note that display modes are not applied to both pagers",
"=                            | Set the unfocused pager to the same settings",
"h/H                          | Next/Prev Highlight mode",
"v/V                          | Next/Prev Display mode (Dec, Hex, Oct etc)",
"l/L                          | Next/Prev base for address location",
"[buffer] ,                   | Set byte grouping (default of 4)",
"[buffer] .                   | Limit/Set number of bytes per line",
"                               (0 = auto-adjust to the terminal size)"
"",
"*Diff mode only",
"The following only operate when two files are open",
"p/P                          | Switch between pagers/files",
"[buffer] [ or ]              | Previous or next hunk",
"[buffer] { or }              | Previous or next big diff hunk",
"",
"&highlight1",
"&highlight2",
NULL,
};

static int help_lines = 14;

void hi_ncurses_help_init(hi_ncurses *ncurses)
{
  
  ncurses->help_win = newwin(LINES-10, COLS-2, 5, 1);
  
  /* Calculate the number of help lines */
  help_lines = 0;
  while (help_data[help_lines]!=NULL)
  {
    help_lines++;
  }
}

void hi_ncurses_help_resize(hi_ncurses *ncurses)
{
  wresize(ncurses->help_win, LINES-10, COLS-2);
  mvwin(ncurses->help_win, 5, 1);
  box(ncurses->help_win, 0, 0);
  werase(ncurses->help_win);
}

void hi_ncurses_help_redraw(hi_ncurses *ncurses)
{
  int x;
  werase(ncurses->help_win);
  

  if (ncurses->show_help)
  { 
    if (COLS < 80)
    {
      ncurses->activate_bell = TRUE;
      mvwprintw(ncurses->help_win,0,0, "Please make the window bigger than 80 columns");
    }
    else
    {
      box(ncurses->help_win, ACS_VLINE, ACS_HLINE);
      for (x = ncurses->help_win_line; x< help_lines; x++)
      {
        if (x-ncurses->help_win_line < LINES-12)
        {
          if (help_data[x][0] == '&')
          {
            if (strcmp(help_data[x],"&highlight1")==0)
            {
              wattron(ncurses->help_win, A_BOLD);
              mvwprintw(ncurses->help_win,1+x-ncurses->help_win_line,1, "'%s' highlight mode", ncurses->focused_pager->highlighter->name);
              wattroff(ncurses->help_win, A_BOLD);
            }
            if (strcmp(help_data[x],"&highlight2")==0)
            {
              mvwprintw(ncurses->help_win,1+x-ncurses->help_win_line,1, ncurses->focused_pager->highlighter->help_string);
            }
          }
          else
          {
            if (help_data[x][0] == '*')
            {
              wattron(ncurses->help_win, A_BOLD);
            }
            mvwprintw(ncurses->help_win,1+x-ncurses->help_win_line,1, help_data[x][0]=='*' ? help_data[x]+1 : help_data[x]);
            if (help_data[x][0] == '*')
            {
              wattroff(ncurses->help_win, A_BOLD);
            }
          }
        }
      }
    }
    wrefresh(ncurses->help_win);
  }
}



gboolean hi_ncurses_help_key_event(hi_ncurses *ncurses, int newch)
{
  switch (newch)
  {
  case KEY_UP:
      ncurses->help_win_line -=1;
      if (ncurses->help_win_line < 0)
      {
        ncurses->help_win_line = 0;
      }
      break;
  case KEY_DOWN:
      ncurses->help_win_line +=1;
      if (ncurses->help_win_line >= (help_lines-1))
      {
        ncurses->help_win_line = (help_lines-1);
      }      
      break;
  case 27:
  case '?':
      ncurses->show_help = !ncurses->show_help;  
      break;
      
  default:
      ncurses->activate_bell = TRUE;
      break;
  }
  return TRUE;
}