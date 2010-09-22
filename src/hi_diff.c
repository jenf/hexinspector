/* hi_diff.c: Hexinspector
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
 * Calculate diff on two files
 */
//#define VERBOSE_DEBUG
#include <hi_file.h>
#include <hi_diff.h>
#include "hi_priv.h"
#include <stdint.h>
#include <buzhash.h>
#include <stdlib.h>
#include <string.h>
#ifdef USE_RABINKARP
#include <rabinkarp.h>

#endif
/* Useful for development */
//#define START_WRONG
//#define DISABLE_NEAR_MATCH

enum diff_mode
{
  DIFF_MODE_SYNC,
  DIFF_MODE_UNSYNCED_NEAR,
  DIFF_MODE_UNSYNCED_FAR,
};

/* Debug function to dump hunks */
static void dump_hunk(hi_diff_hunk *hunk)
{
  switch (hunk->type)
  {
    case HI_DIFF_TYPE_DIFF:
      DPRINTF("DIFF");break;
    case HI_DIFF_TYPE_SAME:
      DPRINTF("SAME");break;
    case HI_DIFF_TYPE_SAME_BACKTRACKED:
      DPRINTF("SAME_BT");break;
    case HI_DIFF_TYPE_SAME_INDEXED:
      DPRINTF("SAME_IDX");break;
  }     
  DPRINTF(" %lx %lx %lx %lx\n",
          (unsigned long) hunk->src_start,
          (unsigned long) hunk->dst_start,
          (unsigned long) hunk->src_end,
          (unsigned long) hunk->dst_end); 
}

/** Compare two diff hunks */
static gint compare_diff_hunks(hi_diff_hunk *hunk1, hi_diff_hunk *hunk2)
{

  if (hunk1 == NULL)
  {
    return 1;
  }
  if (hunk2 == NULL)
  {
    return -1;
  }
  
  /* This is based on the assumption in glib that the lookup hunk1 is the one passed */
  if (hunk1->type == HI_DIFF_FIND_SRC)
  {
    if ((hunk1->src_start >= hunk2->src_start) &&
        (hunk1->src_start <= hunk2->src_end))
    {
      return 0;
    }
  }
  if (hunk1->type == HI_DIFF_FIND_DST)
  {
    if ((hunk1->dst_start >= hunk2->dst_start) &&
        (hunk1->dst_start <= hunk2->dst_end))
    {
      return 0;
    }
    
    if ((hunk1->dst_start-hunk2->dst_start) == 0)
    {
      return hunk1->dst_end - hunk2->dst_end;
    }
    return hunk1->dst_start-hunk2->dst_start;
  }
  
  if ((hunk1->src_start-hunk2->src_start) == 0)
  {
    
    return hunk1->src_end - hunk2->src_end;
  }
  
  return hunk1->src_start-hunk2->src_start;
}

struct diff_userdata
{
  hi_diff *diff;
  hi_diff_hunk *last;
  GSList *list;
};

/** Go through the indexed hunks converting them to backtracked versions */
void backtrack_hunks(hi_diff *diff)
{
  GList *list, *backlist, *tmp;
  hi_diff_hunk *hunk, *otherhunk;
  
  off_t srcptr, dstptr;
  
  list = diff->working_hunks;
  while (list != NULL)
  {
    hunk = list->data;
    
    if (hunk->type==HI_DIFF_TYPE_SAME_INDEXED)
    {
      hunk->type=HI_DIFF_TYPE_SAME_BACKTRACKED;
      srcptr = hunk->src_start;
      dstptr = hunk->dst_start;
      while ((srcptr >= 0) && (dstptr >= 0))
      {
        if (diff->src->memory[srcptr]!=diff->dst->memory[dstptr])
        {
          break;
        }

        srcptr--;
        dstptr--;
      }
      srcptr+=1;
      dstptr+=1;
      
      DPRINTF("Original %lx %lx New %lx %lx\n",
              (unsigned long) hunk->src_start, (unsigned long) hunk->dst_start, (unsigned long) srcptr, (unsigned long) dstptr);
      hunk->src_start = srcptr;
      hunk->dst_start = dstptr;
      
      backlist = g_list_previous(list);
      while (backlist != NULL)
      {
        tmp = g_list_previous(backlist);
        
        otherhunk = backlist->data;

        if ((otherhunk->src_start <= srcptr) && (otherhunk->dst_start <= dstptr))
        {
          DPRINTF("Moving back ");
          dump_hunk(otherhunk);
          otherhunk->src_end = srcptr==0 ? 0 : srcptr-1;
          otherhunk->dst_end = dstptr==0 ? 0 : dstptr-1;
          DPRINTF("To ");
          dump_hunk(otherhunk);
          break;
        }
        else
        {
          DPRINTF("Remove ");
          dump_hunk(otherhunk);
          diff->working_hunks = g_list_delete_link(diff->working_hunks, backlist);
        }
        
        backlist = tmp;
      }

    }
    list = g_list_next(list);
  }
  
  /* Move the data into the binary tree */
  list = diff->working_hunks;
  while (list != NULL)
  {
    g_tree_insert(diff->hunks, list->data, list->data);
    list = g_list_next(list);
    
  }
  g_list_free(diff->working_hunks);
  diff->working_hunks = NULL;
}

/* insert a hunk into the tree */
hi_diff_hunk *insert_hunk(hi_diff *diff, hi_diff_hunk *hunk)
{
  hi_diff_hunk *new;
  DPRINTF("Add hunk ");

  if (hunk->dst_end < hunk->dst_start)
  {
    hunk->dst_end = hunk->dst_start;
  }
  if (hunk->src_end < hunk->src_start)
  {
    hunk->src_end = hunk->src_start;
  }
  
  new = malloc(sizeof(hi_diff_hunk));
  memcpy(new, hunk, sizeof(hi_diff_hunk));
  if (new != NULL)
  {
      diff->working_hunks = g_list_prepend(diff->working_hunks, new); 
  }
  dump_hunk(hunk);

  return new;
}

/** Retrieve a hunk by position */
hi_diff_hunk *hi_diff_get_hunk(hi_diff *diff,
                               hi_file *file,
                               off_t pos)
{
  hi_diff_hunk *found;
  hi_diff_hunk search_hunk = {
    .src_start = pos,
    .dst_start = pos,
    .src_end = 0,
    .dst_end = 0
  };
  if (diff->src == file)
  {
    search_hunk.type = HI_DIFF_FIND_SRC;
  }
  else
  {
    search_hunk.type = HI_DIFF_FIND_DST;
  }
  
  /* Check to see if it's the same hunk as last time */
  if (compare_diff_hunks(&search_hunk, diff->last_hunk) == 0)
  {

    return diff->last_hunk;
  } 
  found = g_tree_lookup(diff->hunks, &search_hunk);

  if (NULL != found)
  {
    diff->last_hunk = found;
#if 0
    DPRINTF("Found %lu", pos);
    dump_hunk(found);
#endif
  }

  return found;
}


/** Create the diff lists */
hi_diff *hi_diff_calculate(hi_file *src, hi_file *dst)
{
  off_t srcptr=0, dstptr=0, srcptr_new=0, dstptr_new=0;
  off_t srcptr_search, dstptr_search;
  off_t search_size;
  enum diff_mode mode = DIFF_MODE_SYNC;
  uint32_t hash = 0;
  off_t *value;  
  int idx;
  gboolean moved_since_hash_match = TRUE;
  gboolean diffed = FALSE;
  int bytes_jump;
  hi_diff *diff;
  
  hi_diff_hunk working_hunk = {
    .type = HI_DIFF_TYPE_SAME,
    .src_start = 0,
    .dst_start = 0,
    .src_end = 0,
    .dst_end = 0
  };
  hi_diff_hunk *last_hunk = NULL;
  
#ifdef START_WRONG
  mode = DIFF_MODE_UNSYNCED_NEAR;
#endif
  
  bytes_jump = (dst->file_options.diff_jump_percent*dst->size)/100;
  if (bytes_jump < dst->file_options.hashbytes)
  {
    bytes_jump = dst->file_options.hashbytes * 2;
  }
  
  /* Create difference structure */
  diff = malloc(sizeof(hi_diff));
  if (NULL == diff)
  {
    DPRINTF("Failure to allocate memory\n");
    return NULL;
  }
  diff->working_hunks = NULL;
  diff->last_hunk = NULL;
  
  diff->hunks = g_tree_new((GCompareFunc)compare_diff_hunks); 
  if (NULL == diff->hunks)
  {
    DPRINTF("Failure to create tree\n");
    return NULL;
  }
  diff->src = src;
  diff->dst = dst;

  
  DPRINTF("Calculating diffs size %lu %lu Jumplimit %i\n", (unsigned long) src->size, (unsigned long)  dst->size, bytes_jump);
  
  for (srcptr_new = 0; srcptr_new < dst->file_options.hashbytes; srcptr_new++)
  {
    if (srcptr_new < src->size)
    {
#ifdef USE_RABINKARP
      hash = rabinkarp_add(hash, src->memory[srcptr_new], PRIME, BASE);
#else
      hash = buzhash_roll(hash, src->memory[srcptr_new], 0, srcptr_new, dst->file_options.hashbytes);
#endif
      VDPRINTF("Rolling %lu 0x%x\n", (unsigned long) srcptr_new, hash);
    }
    else
    {
      /* This may or may not work */
      DPRINTF("Attempting to roll a very small file, untested behaviour\n");
      break;
    }
  }
  
  while ((srcptr < src->size) && (dstptr < dst->size))
  {
    srcptr_new = srcptr;
    dstptr_new = dstptr;
    
    switch (mode)
    {
        /* Files are currently exact */
        case DIFF_MODE_SYNC:
        if (src->memory[srcptr] == dst->memory[dstptr])
        {
          srcptr_new ++;
          dstptr_new ++;
          last_hunk = NULL;
          moved_since_hash_match = TRUE;
        }
        else
        {
          DPRINTF("Lost sync at %lx %lx\n",  (unsigned long) srcptr, (unsigned long) dstptr);
          if ((srcptr != working_hunk.src_start) &&
             (dstptr != working_hunk.dst_start))
          {
              working_hunk.src_end = srcptr-1;
              working_hunk.dst_end = dstptr-1;

              insert_hunk(diff, &working_hunk);  
            
              working_hunk.src_start = srcptr;
              working_hunk.dst_start = dstptr;
            
          }


          
          mode = DIFF_MODE_UNSYNCED_NEAR;
        }
        break;
      
        case DIFF_MODE_UNSYNCED_NEAR:

        /* Try all combinations of bytes up to the dst hash size
           I'm not convinced that you have to do it all */
#ifndef DISABLE_NEAR_MATCH        
        DPRINTF("Search start at %lu %lu\n", (unsigned long) srcptr, (unsigned long) dstptr);
        
        /* Currently make it 16 */
        for (search_size=0;search_size<dst->file_options.hashbytes;search_size++)
        {
          if (DIFF_MODE_UNSYNCED_NEAR != mode) {break;}
#if 0          
          VDPRINTF("Search %lu\n", search_size);
#endif
          for (srcptr_search = 0; srcptr_search <= search_size; srcptr_search++)
          {
            dstptr_search = search_size - (srcptr_search);
            if ((srcptr+srcptr_search+dst->file_options.minimum_same-1) > src->size) {continue;}
            if ((dstptr+dstptr_search+dst->file_options.minimum_same-1) > dst->size) {continue;}      
#if 0            
            VDPRINTF("Search %lu %lu %lu\n", (unsigned long) search_size, (unsigned long)srcptr_search, (unsigned long)dstptr_search);
#endif
            if (src->memory[srcptr+srcptr_search] == dst->memory[dstptr+dstptr_search])
            {
              diffed = FALSE;
              for (idx = 1; idx < dst->file_options.minimum_same; idx++)
              {
                if ((src->memory[srcptr+srcptr_search+idx]) != (dst->memory[dstptr+dstptr_search+idx]))
                {
                  diffed = TRUE;
                  break;
                }
                                                                         
              }
              
              if (diffed == FALSE)
              {
                srcptr_new = srcptr+srcptr_search;
                dstptr_new = dstptr+dstptr_search;
                mode = DIFF_MODE_SYNC;
                
                working_hunk.src_end = srcptr_new-1;
                working_hunk.dst_end = dstptr_new-1;
                working_hunk.type = HI_DIFF_TYPE_DIFF;
                insert_hunk(diff, &working_hunk); 
                
                working_hunk.src_start = srcptr_new;
                working_hunk.dst_start = dstptr_new;
                working_hunk.type = HI_DIFF_TYPE_SAME;
                
                DPRINTF("Near Sync at %lx %lx\n", (unsigned long) srcptr_new, (unsigned long) dstptr_new);
                break;
              }

            }
          }
        }
#endif        
        /* Couldn't find a match */
        if (DIFF_MODE_UNSYNCED_NEAR == mode)
        {
          mode = DIFF_MODE_UNSYNCED_FAR;   
        }

        break;
        
        case DIFF_MODE_UNSYNCED_FAR:
        
        if (FALSE == moved_since_hash_match)
        {
          srcptr_new++;
          break;
        }
        
        /* Cannot run in the far mode within the hashbyte length */
        if (srcptr+dst->file_options.hashbytes >= src->size)
        {
          mode = DIFF_MODE_UNSYNCED_NEAR;
        }
        else
        {
          VDPRINTF("Unsynced far mode %lu %x\n",(unsigned long) srcptr_new, hash);
          /* Lookup the current hash value in the list */
          value = g_hash_table_lookup(dst->buzhashes, (gpointer) hash);
          if (NULL != value)
          {
#if 1
            VDPRINTF("Found hash %lu %x\n", (unsigned long)srcptr, hash);
#endif
            /* Check for one after the current dstptr */
            for (idx = 0; idx < value[0]; idx++)
            {
#if 0
              VDPRINTF("Value at %lu %lu %lu %i\n",(unsigned long)srcptr, (unsigned long)dstptr, (unsigned long)value[idx+1], idx);
#endif
              if (value[idx+1] >= dstptr)
              {
                if (value[idx+1] < dstptr+bytes_jump)
                {
                  
                  /* This should memcmp the memory to ensure it is correct, however to many collisions will cause it to grind to a halt. */
                  if (TRUE)
                  {
                    dstptr_new = value[idx+1];
                    DPRINTF("Far Sync at %lx %lx %i %x %x\n",(unsigned long)srcptr, (unsigned long)dstptr_new, idx, src->memory[srcptr],dst->memory[dstptr_new]);
                    mode = DIFF_MODE_SYNC;
                    if (last_hunk == NULL)
                    {
                      working_hunk.src_end = srcptr-1;
                      working_hunk.dst_end = dstptr_new-1;

                      working_hunk.type = HI_DIFF_TYPE_DIFF;
                      last_hunk = insert_hunk(diff, &working_hunk); 
                    }
                    else
                    {
                      /* This relies on the basis that it will not move position in the balanced binary tree */
                      last_hunk->src_end = srcptr_new-1;
                      last_hunk->dst_end = dstptr_new-1;
                      DPRINTF("Edit last hunk to %lx %lx\n", (unsigned long)last_hunk->src_end, (unsigned long)last_hunk->dst_end);
                    }
                    working_hunk.src_start = srcptr;
                    working_hunk.dst_start = dstptr_new;
                    working_hunk.type = HI_DIFF_TYPE_SAME_INDEXED;
                    
                    moved_since_hash_match = FALSE;
                    break;
                  }
                  else
                  {
                    VDPRINTF("Hash collision at %lx %lx\n", (unsigned long)srcptr, (unsigned long)dstptr_new);
                  }
                }
                else
                {
                  DPRINTF("Would have jumped to far  %lu %lu %lu %i\n",(unsigned long)srcptr, (unsigned long)dstptr, (unsigned long)value[idx+1], idx);
                  break;
                }
                
              }
            }

          }
        }
        
        if (DIFF_MODE_SYNC != mode)
        {
          srcptr_new++;          
        }

        

        break;
        
        default:
        DPRINTF("Unhandled case\n");
        srcptr_new++;
    }
    
    for (/**/ ; srcptr < srcptr_new; srcptr++)
    {
      if (srcptr+dst->file_options.hashbytes < src->size)
      {
#ifdef USE_RABINKARP
        hash = rabinkarp_roll(hash, src->memory[srcptr+dst->file_options.hashbytes], src->memory[srcptr ==0 ? 0 : srcptr],
                              srcptr+dst->file_options.hashbytes, dst->file_options.hashbytes,
                              PRIME, BASE, dst->file_options.popvalue);
#else
        hash = buzhash_roll(hash, src->memory[srcptr+dst->file_options.hashbytes], src->memory[srcptr ==0 ? 0 : srcptr], srcptr+dst->file_options.hashbytes, dst->file_options.hashbytes);
#endif
#if 1
        VDPRINTF("%lu %lu %lx\n", (unsigned long) srcptr, (unsigned long) srcptr_new, hash);
#endif
      }
    }
    dstptr = dstptr_new;
    
  }
  
  /* Add the last working hunk */
  if (mode == DIFF_MODE_SYNC)
  {
    working_hunk.src_end = srcptr-1;
    working_hunk.dst_end = dstptr-1;
    DPRINTF("%lx\n", (unsigned long) srcptr_new);
    DPRINTF("EOF SAME\n");
    insert_hunk(diff, &working_hunk);
    if (dstptr != dst->size)
    {
      working_hunk.src_start = srcptr;
      working_hunk.src_end = src->size;
      working_hunk.dst_start = dstptr;
      working_hunk.dst_end = dst->size;
      working_hunk.type = HI_DIFF_TYPE_DIFF;
      DPRINTF("DST EOF DIFF\n");
      insert_hunk(diff, &working_hunk);
    }
    else if (srcptr != src->size)
    {
      working_hunk.src_start = srcptr;
      working_hunk.src_end = src->size;
      working_hunk.dst_start = dstptr;
      working_hunk.dst_end = dst->size;
      working_hunk.type = HI_DIFF_TYPE_DIFF;
      DPRINTF("SRC EOF DIFF\n");
      insert_hunk(diff, &working_hunk);
    }
  }
  else
  {
    if (last_hunk == NULL)
    {
      working_hunk.src_end = src->size;
      working_hunk.dst_end = dst->size;
      working_hunk.type = HI_DIFF_TYPE_DIFF;
      insert_hunk(diff, &working_hunk);
    }
    else
    {
      last_hunk->src_end = src->size;
      last_hunk->dst_end = dst->size;
    }
    
  }
  
  struct diff_userdata userdata={
    .diff = diff,
    .last = NULL,
    .list = NULL
  }; 
  /* Reverse the list */
  diff->working_hunks = g_list_reverse(diff->working_hunks);
  
  /* Backtrack the indexed ones */
  backtrack_hunks(diff);
  
  return diff;
}
