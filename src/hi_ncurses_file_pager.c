/* hi_ncurses_file_pager.c: Hexinspector
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
 * Ncurses file pager
 */
#include <hi_ncurses.h>
#include <stdlib.h>
#include <macros.h>
#include <ctype.h>

#define BYTES_FOR_BORDER (4)
#define WIDTH_PER_BYTE (3)
#define OFFSET_SIZE (8)
static void update_bytes_per_line(hi_ncurses_fpager *pager)
{
  /* TODO: add other output format mechanisms */
  pager->bytes_per_row = (pager->width-(OFFSET_SIZE+BYTES_FOR_BORDER))/WIDTH_PER_BYTE;
  
}

hi_ncurses_fpager *hi_ncurses_fpager_new(hi_ncurses *curses,
                                         hi_file *file,
                                         hi_diff *diff,
                                         int height, int width,
                                         int y, int x)
{
  hi_ncurses_fpager *pager;
  pager = malloc(sizeof(hi_ncurses_fpager));
  if (NULL == pager)
  {
    DPRINTF("Could not allocate pager\n");
    return NULL;
  }
  
  pager->file = file;
  pager->diff = diff;
  pager->curses = curses;
  pager->width = width;
  pager->height = height;
  pager->x = x;
  pager->y = y;
  pager->offset = 0;
  pager->window = newwin(height , width,y, x);
  pager->linked_pager = NULL;
  update_bytes_per_line(pager);
  
  return pager;
}

void hi_ncurses_fpager_resize(hi_ncurses_fpager *pager,
                              int height, int width,
                              int y, int x)
{
  pager->x = x;
  pager->y = y;
  pager->width = width;
  pager->height = height;
  update_bytes_per_line(pager);
  wresize(pager->window,height, width);
  mvwin(pager->window, y, x);
  box(pager->window, 0, 0);
  werase(pager->window);
}

void hi_ncurses_fpager_redraw(hi_ncurses_fpager *pager)
{
  int x, y;
  unsigned char val;
  off_t offset;
  char buffer[256];
  hi_diff_hunk *hunk;
  werase(pager->window);
  gboolean diff;
  void *highlighter_data = NULL;
  hi_ncurses_highlight *highlighter;
  enum hi_ncurses_colour colour;
  
  highlighter = pager->curses->highlighter;
  
  if ((highlighter != NULL) && (highlighter->begin_func != NULL))
  {
    highlighter_data = highlighter->begin_func(pager->file);
  }
  
  /* Make the pager we're selected highlighted */
  if (pager == pager->curses->focused_pager)
    wattron(pager->window, A_REVERSE);
  
  box(pager->window, ACS_VLINE, ACS_HLINE);
  
  if (pager == pager->curses->focused_pager)
    wattroff(pager->window, A_REVERSE);
  
  for (y=0; y< pager->height-2; y++)
  {
    for (x=0; x<pager->bytes_per_row; x++)
    {
      offset = pager->offset+x+(pager->bytes_per_row*y);

      if (offset < pager->file->size)
      {
          if (x == 0)
          {

            
            snprintf(buffer, 256, "%08x", (unsigned int) offset);
            wmove(pager->window, y+1,1);
            waddstr(pager->window, buffer);
            

          }
          val = pager->file->memory[offset];
        
          diff = FALSE;
          if (pager->diff)
          {
            hunk = hi_diff_get_hunk(pager->diff, pager->file, offset);
            if (hunk != NULL && hunk->type == HI_DIFF_TYPE_DIFF)
            {
              /* TODO: Fix this */
              if ((pager->diff->dst==pager->file) && (hunk->dst_start==hunk->dst_end))
              {
                diff = FALSE;
              }
              diff = TRUE;
            }
            
          }
        
          if (TRUE == diff)
            wattron(pager->window, A_REVERSE);
          
          colour = hi_ncurses_colour_normal;
          if ((highlighter != NULL) && (highlighter->highlight_func != NULL))
          {
            colour = highlighter->highlight_func(pager->file, offset, val, highlighter_data);
            
          }
          wcolor_set(pager->window, colour, NULL);
        
          mvwprintw(pager->window,y+1,2+OFFSET_SIZE+(x*3),"%02x",val);  
          wcolor_set(pager->window, hi_ncurses_colour_normal,NULL);
          if (TRUE == diff)
            wattroff(pager->window, A_REVERSE);
            
      }
    }
  
  }
  if ((highlighter != NULL) && (highlighter->end_func != NULL))
  {
       highlighter->end_func(highlighter_data);
  }
      
  wrefresh(pager->window);
}

/** Set the offset and move the linked pager to the correct location */
static void set_offset(hi_ncurses_fpager *pager, off_t offset)
{
  hi_diff_hunk *hunk;
  
  pager->offset = offset;
  if (pager->offset < 0)
  {
    pager->offset = 0;
  }
  if (pager->offset >= pager->file->size)
  {
    pager->offset = pager->file->size;
  }
  
  /* Make the other pager move to the right position */
  if (NULL != pager->linked_pager)
  {
    hunk = hi_diff_get_hunk(pager->diff, pager->file, pager->offset);
    if (hunk != NULL)
    {
      if (pager->diff->src == pager->file)
      {
        
        if (hunk->type == HI_DIFF_TYPE_DIFF)
        {
          pager->linked_pager->offset = hunk->dst_start;
        }
        else
        {
          pager->linked_pager->offset = hunk->dst_start+(pager->offset-hunk->src_start);
        }
      }
      else
      {
        if (hunk->type == HI_DIFF_TYPE_DIFF)
        {
          pager->linked_pager->offset = hunk->src_start;
        }
        else
        {
          pager->linked_pager->offset = hunk->src_start+(pager->offset-hunk->dst_start);
        }
      }     
      
      /* Ensure the values are within bounds */
      if (pager->linked_pager->offset < 0)
      {
        pager->linked_pager->offset = 0;
      }
      if (pager->linked_pager->offset >= pager->linked_pager->file->size)
      {
        pager->linked_pager->offset = pager->linked_pager->file->size;
      } 
    }
    
  }  
}

static relative_move_pager(hi_ncurses_fpager *pager, off_t move)
{
  set_offset(pager, pager->offset + move);
}

static void move_to_next_diff(hi_ncurses_fpager *pager, gboolean forwards)
{
  hi_diff_hunk *hunk;
  
  hunk = hi_diff_get_hunk(pager->diff, pager->file, pager->offset);
  if (NULL != hunk)
  {
    if (forwards)
    {
      if (pager->file == pager->diff->src)
      {
        set_offset(pager, hunk->src_end);        
      }
      if (pager->file == pager->diff->dst)
      {
        set_offset(pager, hunk->dst_end);        
      }
    }
    else
    {
      if (pager->file == pager->diff->src)
      {
        set_offset(pager, hunk->src_start-1);        
      }
      if (pager->file == pager->diff->dst)
      {
        set_offset(pager, hunk->dst_start-1);        
      }      
    }
  }
}

/** Act on a key, returns TRUE if key was claimed */
gboolean hi_ncurses_fpager_key_event(hi_ncurses_fpager *pager,
                                     int key,
                                     long long buffer_val)
{
  signed long long requested_offset;
  
  gboolean claimed = FALSE;
  switch (key)
  {
    case KEY_NPAGE:
      /* Keep the last line on the screen */
      relative_move_pager(pager, (pager->bytes_per_row * pager->height-3) *  buffer_val);
      pager->curses->buffer[0]=0;
      claimed = TRUE;
      break;
    case KEY_PPAGE:
      /* Keep the last line on the screen */
      relative_move_pager(pager, -(pager->bytes_per_row * pager->height-3) * buffer_val);
      pager->curses->buffer[0]=0;
      claimed = TRUE;
      break;     
    case KEY_LEFT:
      relative_move_pager(pager, -1*buffer_val);
      pager->curses->buffer[0]=0;
      claimed = TRUE;
      break;
    case KEY_RIGHT:
      relative_move_pager(pager, 1*buffer_val);
      pager->curses->buffer[0]=0;
      claimed = TRUE;
      break;
    

    case KEY_UP:
      relative_move_pager(pager, -pager->bytes_per_row*buffer_val);
      pager->curses->buffer[0]=0;
      claimed = TRUE;
      break;
    case KEY_DOWN:
      relative_move_pager(pager, pager->bytes_per_row*buffer_val);
      pager->curses->buffer[0]=0;
      claimed = TRUE;
      break;   
      
    case '[':
      move_to_next_diff(pager, FALSE);
      claimed = TRUE;
      break;
    case ']':
      move_to_next_diff(pager, TRUE);
      claimed = TRUE;
      break;
    case 'G':
    case 'g':
      requested_offset = strtoll(pager->curses->buffer, NULL, 0);
      if (buffer_val < 0)
      {
        set_offset(pager, pager->file->size+requested_offset);
      }
      else
      {
        set_offset(pager, (off_t) buffer_val);
      }
      pager->curses->buffer[0]=0; 
      break;
      
  }

  return claimed;
}