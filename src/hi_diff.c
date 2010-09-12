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

#include <hi_file.h>
#include <hi_diff.h>
#include "hi_priv.h"
#include <stdint.h>
#include <buzhash.h>
#include <stdlib.h>

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
  DPRINTF(" %lu %lu %lu %lu\n",
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
        (hunk1->src_start <  hunk2->src_end))
    {
        return 0;
    }
  }
  if (hunk1->type == HI_DIFF_FIND_DST)
  {
    if ((hunk1->dst_start >= hunk2->dst_start) &&
        (hunk1->dst_start <  hunk2->dst_end))
    {
      return 0;
    }
    return hunk1->dst_start-hunk2->dst_start;
  }
  
  return hunk1->src_start-hunk2->src_start;
}

/** Go through the indexed hunks converting them to backtracked versions */
gboolean backtrack_hunks(hi_diff_hunk *hunk,void *value, hi_diff *diff)
{
  off_t srcptr, dstptr;
  if (hunk->type==HI_DIFF_TYPE_SAME_INDEXED)
  {
    hunk->type=HI_DIFF_TYPE_SAME_BACKTRACKED;
    srcptr = hunk->src_start;
    dstptr = hunk->dst_start;
    while ((srcptr >0) || (dstptr > 0))
    {
      if (diff->src->memory[srcptr]!=diff->dst->memory[dstptr])
      {
        break;
      }
      srcptr--;
      dstptr--;
    }
    DPRINTF("Original %lu %lu New %lu %lu\n", hunk->src_start, hunk->dst_start, srcptr, dstptr);
    hunk->src_start = srcptr;
    hunk->dst_start = dstptr;
  }
  
  return FALSE;
}

struct missing_diff_userdata
{
  hi_diff *diff;
  hi_diff_hunk *last;
  GSList *list;
};


                               
/* For each item add the diffs */
gboolean insert_missing_diffs_each(hi_diff_hunk *hunk, void *value, struct missing_diff_userdata *userdata)
{
  hi_diff_hunk *new_hunk;

  if ((userdata->last != NULL) && (hunk->type != HI_DIFF_TYPE_DIFF))
  {
    new_hunk=malloc(sizeof(hi_diff_hunk));
    new_hunk->type      = HI_DIFF_TYPE_DIFF;
    new_hunk->src_start = userdata->last->src_end;
    new_hunk->src_end   = hunk->src_start;
    new_hunk->dst_start = userdata->last->dst_end;
    new_hunk->dst_end   = userdata->last->dst_start;
    DPRINTF("New Diff ");
    dump_hunk(new_hunk);
    userdata->list = g_slist_prepend(userdata->list, new_hunk);
  }
  userdata->last = hunk;
  return FALSE;
}

void insert_missing_diffs_to_tree(hi_diff_hunk *hunk, hi_diff *diff)
{
  DPRINTF("Insert ");
  dump_hunk(hunk);
  g_tree_insert(diff->hunks, hunk, hunk);
}

/** Add the missing diffs */
void insert_missing_diffs(hi_diff *diff)
{
  GSList *list;
  struct missing_diff_userdata userdata={
    .diff = diff,
    .last = NULL,
    .list = NULL
  };
  
  g_tree_foreach(diff->hunks, (GTraverseFunc) insert_missing_diffs_each, &userdata);
  
  list = userdata.list;
  
  if (list != NULL)
  {
     /* Add the added items */
    g_slist_foreach(userdata.list,(GFunc) insert_missing_diffs_to_tree, diff);
    g_slist_free(userdata.list);   
  }

}



/* insert a hunk into the tree */
void insert_hunk(hi_diff *diff, hi_diff_hunk *hunk)
{
  hi_diff_hunk *new;
  DPRINTF("Add hunk ");

  
  new = malloc(sizeof(hi_diff_hunk));
  memcpy(new, hunk, sizeof(hi_diff_hunk));
  if (new != NULL)
  {
      g_tree_insert(diff->hunks, new, new); 
  }
  dump_hunk(hunk);

  
}

/** Retrieve a hunk by position */
hi_diff_hunk *hi_diff_get_hunk(hi_diff *diff,
                               int pos,
                               enum hi_diff_type type)
{
  hi_diff_hunk *found;
  hi_diff_hunk search_hunk = {
    .type = type,
    .src_start = pos,
    .dst_start = pos,
    .src_end = 0,
    .dst_end = 0
  };
  
  found = g_tree_lookup(diff->hunks, &search_hunk);
  
  if (NULL != found)
  {
    DPRINTF("Found ");
    dump_hunk(found);
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
  int bytes_jump;
  hi_diff *diff;
  
  hi_diff_hunk working_hunk = {
    .type = HI_DIFF_TYPE_SAME,
    .src_start = 0,
    .dst_start = 0,
    .src_end = 0,
    .dst_end = 0
  };
  
  
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
      hash = buzhash_roll(hash, src->memory[srcptr_new], 0, srcptr_new, dst->file_options.hashbytes);
      VDPRINTF("Rolling %lu %u\n", (unsigned long) srcptr_new, hash);
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
          moved_since_hash_match = TRUE;
        }
        else
        {
          working_hunk.src_end = srcptr;
          working_hunk.dst_end = dstptr;
          insert_hunk(diff, &working_hunk);

          
          mode = DIFF_MODE_UNSYNCED_NEAR;
        }
        break;
      
        case DIFF_MODE_UNSYNCED_NEAR:

        /* Try all combinations of bytes up to the dst hash size
           I'm not convinced that you have to do it all */
        
        /* Currently make it 16 */
        for (search_size=0;search_size<dst->file_options.hashbytes;search_size++)
        {
          if (DIFF_MODE_UNSYNCED_NEAR != mode) {break;}
          
          VDPRINTF("Search %lu\n", search_size);
          for (srcptr_search = 0; srcptr_search < search_size; srcptr_search++)
          {
            dstptr_search = search_size - (srcptr_search);
            if (srcptr+srcptr_search > src->size) {continue;}
            if (dstptr+dstptr_search > dst->size) {continue;}      
            
            VDPRINTF("Search %lu %lu %lu\n", (unsigned long) search_size, (unsigned long)srcptr_search, (unsigned long)dstptr_search);
            if (src->memory[srcptr+srcptr_search] == dst->memory[dstptr+dstptr_search])
            {
              srcptr_new = srcptr+srcptr_search;
              dstptr_new = dstptr+dstptr_search;
              mode = DIFF_MODE_SYNC;
              
              working_hunk.src_start = srcptr;
              working_hunk.dst_start = dstptr;
              working_hunk.type = HI_DIFF_TYPE_SAME;
              
              DPRINTF("Near Sync at %lu %lu\n", (unsigned long) srcptr_new, (unsigned long) dstptr_new);
              break;

            }
          }
        }
        
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
          VDPRINTF("Unsynced far mode %lu %lu\n",(unsigned long) srcptr_new, hash);
          /* Lookup the current hash value in the list */
          value = g_hash_table_lookup(dst->buzhashes, (gpointer) hash);
          if (NULL != value)
          {
            DPRINTF("Found hash %lu %u\n", (unsigned long)srcptr, hash);
            
            /* Check for one after the current dstptr */
            for (idx = 0; idx < value[0]; idx++)
            {
              VDPRINTF("Value at %lu %lu %lu %i\n",(unsigned long)srcptr, (unsigned long)dstptr, (unsigned long)value[idx+1], idx);
              if (value[idx+1] >= dstptr)
              {
                if (value[idx+1] < dstptr+bytes_jump)
                {
                  dstptr_new = value[idx+1];
                  DPRINTF("Far Sync at %lu %lu %i\n",(unsigned long)srcptr, (unsigned long)dstptr_new, idx);
                  mode = DIFF_MODE_SYNC;
                  working_hunk.src_start = srcptr;
                  working_hunk.dst_start = dstptr;
                  working_hunk.type = HI_DIFF_TYPE_SAME_INDEXED;
                  
                  moved_since_hash_match = FALSE;
                  break;
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
        hash = buzhash_roll(hash, src->memory[srcptr+dst->file_options.hashbytes], src->memory[srcptr ==0 ? 0 : srcptr], srcptr+dst->file_options.hashbytes, dst->file_options.hashbytes);
        VDPRINTF("%lu %lu %lu\n", (unsigned long) srcptr, (unsigned long) srcptr_new, hash);
      }
    }
    dstptr = dstptr_new;
    
  }
  
  /* Add the last working hunk */
  if (mode == DIFF_MODE_SYNC)
  {
    working_hunk.src_end = srcptr;
    working_hunk.dst_end = dstptr;
    insert_hunk(diff, &working_hunk);
    if (dstptr != dst->size)
    {
      working_hunk.src_start = srcptr;
      working_hunk.src_end = srcptr;
      working_hunk.dst_start = dstptr;
      working_hunk.dst_end = dst->size;
      working_hunk.type = HI_DIFF_TYPE_DIFF;
      insert_hunk(diff, &working_hunk);
    }
  }
  else
  {
    working_hunk.src_end = src->size;
    working_hunk.dst_end = dst->size;
    working_hunk.type = HI_DIFF_TYPE_DIFF;
    insert_hunk(diff, &working_hunk);
  }
  
  /* Backtrack the indexed ones */
  g_tree_foreach(diff->hunks, (GTraverseFunc) backtrack_hunks, diff);
  
  /* Insert the missing diff hunks */
  insert_missing_diffs(diff);
  
  return diff;
}