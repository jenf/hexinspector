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
#include <hi_ncurses_display.h>

#define BYTES_FOR_BORDER (4)
static void update_bytes_per_line(hi_ncurses_fpager *pager)
{
  int x;
  off_t offset;
  
  for (x=0, offset = pager->file->size; offset > 0; offset /=pager->location_mode->base)
  {
    x++;
  }
  pager->bytes_in_location = x;
  pager->remaining_bytes_per_row = pager->width-(pager->bytes_in_location+BYTES_FOR_BORDER);
  pager->bytes_per_row = pager->display_mode->bytes_per_line_func(pager, pager->remaining_bytes_per_row);
  
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
  pager->byte_grouping = 4;
  pager->set_bytes_per_row = 0;
  pager->window = newwin(height , width,y, x);
  pager->linked_pager = NULL;
  pager->display_mode = hi_ncurses_display_get(NULL, 0);
  pager->location_mode = hi_ncurses_location_get(NULL, 0);
  pager->highlighter   = hi_ncurses_highlight_get(NULL,0);
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
  char format_str[256];
  int bytes;
  
  highlighter = pager->highlighter;
  
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
  
  mvwprintw(pager->window, pager->height-1, 2," %s ", pager->file->filename);  
  
  snprintf(format_str,256,pager->location_mode->constructor_string, pager->bytes_in_location);

  bytes = (pager->set_bytes_per_row == 0 ? pager->bytes_per_row : pager->set_bytes_per_row);
  if (bytes > pager->bytes_per_row)
  {
    bytes = pager->bytes_per_row;
  }
  
  for (y=0; y< pager->height-2; y++)
  {
    for (x=0; x<bytes; x++)
    {
      offset = pager->offset+x+(pager->set_bytes_per_row == 0 ? pager->bytes_per_row*y : pager->set_bytes_per_row*y);

      if (offset < pager->file->size)
      {
          if (x == 0)
          {

            
            snprintf(buffer, 256, format_str, (unsigned int) offset);
            wmove(pager->window, y+1,2);
            waddstr(pager->window, buffer);
            

          }
          val = pager->file->memory[offset];
        
          diff = FALSE;
          if (pager->diff)
          {
            hunk = hi_diff_get_hunk(pager->diff, pager->file, offset);
            if (hunk != NULL && hunk->type == HI_DIFF_TYPE_DIFF)
            {
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
        
          if (colour != hi_ncurses_colour_normal)
          {
            wcolor_set(pager->window, colour, NULL);
          }
        
          /* Display byte */
          pager->display_mode->display_byte_func(pager, y+1, 3+pager->bytes_in_location, x, offset, val);
 
          if (colour != hi_ncurses_colour_normal)
          {
            wcolor_set(pager->window, hi_ncurses_colour_normal,NULL);
          }
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
  double ratio;
  
  pager->offset = offset;
  if (pager->offset < 0)
  {
    pager->curses->activate_bell = TRUE;
    pager->offset = 0;
  }
  if (pager->offset >= pager->file->size)
  {
    pager->offset = pager->file->size;
    pager->curses->activate_bell = TRUE;
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
          if (hunk->src_end - hunk->src_start == 0)
          {
            ratio = 0.0;
          }
          else
          {
            ratio = (pager->offset - hunk->src_start) / ((double)(hunk->src_end - hunk->src_start));
          }
          pager->linked_pager->offset = hunk->dst_start + (ratio * (hunk->dst_end - hunk->dst_start));
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
          if (hunk->dst_end - hunk->dst_start == 0)
          {
            ratio = 0.0;
          }
          else
          {
            ratio = (pager->offset - hunk->dst_start) / ((double)(hunk->dst_end - hunk->dst_start));
          }
          pager->linked_pager->offset = hunk->src_start + (ratio * (hunk->src_end - hunk->src_start));
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

static void relative_move_pager(hi_ncurses_fpager *pager, off_t move)
{
  set_offset(pager, pager->offset + move);
}

static void move_to_next_diff(hi_ncurses_fpager *pager, int times, gboolean bigdiff)
{
  hi_diff_hunk *hunk = NULL, *hunk2 = NULL;
  int i=0;
  int times_abs = abs(times);
  gboolean forwards= FALSE;
 
  if (pager->diff == NULL)
  {
    return;
  } 

  if (times_abs == times)
    forwards = TRUE;
  
  while (i < times_abs)
  {
    hunk = hi_diff_get_hunk(pager->diff, pager->file, pager->offset);
    if (NULL != hunk)
    {
      if (forwards)
      {
        if (pager->file == pager->diff->src)
        {
          set_offset(pager, hunk->src_end+1);        
        }
        if (pager->file == pager->diff->dst)
        {
          set_offset(pager, hunk->dst_end+1);        
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
      if (bigdiff == TRUE)
      {
        hunk2 = hi_diff_get_hunk(pager->diff, pager->file, pager->offset);
        if (NULL != hunk2)
        {
          if (hunk2 == hunk)
          {
            break;
          }
          if ((hunk2->type != HI_DIFF_TYPE_DIFF) ||
              ((hunk2->src_end - hunk2->src_start < pager->curses->big_hunk_size) ||
              (hunk2->dst_end - hunk2->dst_start < pager->curses->big_hunk_size)))
          {
            continue;
          }
        }

      }
    }
    else
    {
      break;
    }
    i++;
  }
  
  /* move to the start of a bigdiff, as it's probably what the user wants */
  if ((bigdiff == TRUE) && (hunk != hunk2))
  {
    hunk = hi_diff_get_hunk(pager->diff, pager->file, pager->offset);
    if (pager->file == pager->diff->src)
    {
      set_offset(pager, hunk->src_start);        
    }
    if (pager->file == pager->diff->dst)
    {
      set_offset(pager, hunk->dst_start);        
    }      
  }
}

/** Act on a key, but only if your the slave pager */
gboolean hi_ncurses_fpager_slave_key_event(hi_ncurses_fpager *pager,
                                           int key)
{
  gboolean claimed = FALSE;
  switch (key)
  {
    case '=':
      pager->location_mode = pager->linked_pager->location_mode;
      pager->display_mode  = pager->linked_pager->display_mode;
      pager->highlighter   = pager->linked_pager->highlighter;
      pager->byte_grouping = pager->linked_pager->byte_grouping;
      update_bytes_per_line(pager);
      claimed = TRUE;
      break;
  }
  return claimed;
}

/** Act on a key, returns TRUE if key was claimed */
gboolean hi_ncurses_fpager_key_event(hi_ncurses_fpager *pager,
                                     int key,
                                     long long buffer_val)
{
  signed long long requested_offset;
  
  gboolean claimed = TRUE;
  switch (key)
  {
    case KEY_NPAGE:
      /* Keep the last line on the screen */
      relative_move_pager(pager, (pager->bytes_per_row * pager->height-3) *  buffer_val);
      pager->curses->buffer[0]=0;
      break;
    case KEY_PPAGE:
      /* Keep the last line on the screen */
      relative_move_pager(pager, -(pager->bytes_per_row * pager->height-3) * buffer_val);
      pager->curses->buffer[0]=0;
      break;     
    case KEY_LEFT:
      relative_move_pager(pager, -1*buffer_val);
      pager->curses->buffer[0]=0;
      break;
    case KEY_RIGHT:
      relative_move_pager(pager, 1*buffer_val);
      pager->curses->buffer[0]=0;
      break;
    

    case KEY_UP:
      relative_move_pager(pager, -pager->bytes_per_row*buffer_val);
      pager->curses->buffer[0]=0;
      break;
    case KEY_DOWN:
      relative_move_pager(pager, pager->bytes_per_row*buffer_val);
      pager->curses->buffer[0]=0;
      break;   
      
    /* Display modes */
    case 'V':
      pager->display_mode = hi_ncurses_display_get(pager->display_mode,-1);
      update_bytes_per_line(pager);
      break;          
    case 'v':
      pager->display_mode = hi_ncurses_display_get(pager->display_mode,1);
      update_bytes_per_line(pager);
      break;      
    case 'L':
      pager->location_mode = hi_ncurses_location_get(pager->location_mode,-1); 
      update_bytes_per_line(pager);
      break;
    case 'l':
      pager->location_mode = hi_ncurses_location_get(pager->location_mode,1); 
      update_bytes_per_line(pager);
      break;
    case 'H':
      pager->highlighter = hi_ncurses_highlight_get(pager->highlighter,-1);
      break;          
    case 'h':
      pager->highlighter = hi_ncurses_highlight_get(pager->highlighter,1);
      break;
    case ',':
      pager->byte_grouping = buffer_val;
      pager->curses->buffer[0]=0; 
      update_bytes_per_line(pager);
      break;
    case '.':
      pager->set_bytes_per_row = buffer_val >= 0 ? buffer_val : 0;
      pager->curses->buffer[0]=0;
      break;
      
    case '[':
      move_to_next_diff(pager,-buffer_val, FALSE);
      pager->curses->buffer[0]=0; 
      break;
    case ']':
      move_to_next_diff(pager, buffer_val, FALSE);
      pager->curses->buffer[0]=0; 
      break;
    case '{':
      move_to_next_diff(pager,-buffer_val, TRUE);
      pager->curses->buffer[0]=0; 
      break;
    case '}':
      move_to_next_diff(pager, buffer_val, TRUE);
      pager->curses->buffer[0]=0; 
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
      
    default:
      claimed = FALSE;
  }

  return claimed;
}

void hi_ncurses_fpager_search(hi_ncurses_fpager *pager, char *search)
{
  char *error;
  off_t offset;
  gboolean found;
  
  found = hi_search_compile_and_exec(pager->file, search, pager->offset, &offset, &pager->curses->error);
  if (found == TRUE)
  {
    set_offset(pager, offset);
  }
}
