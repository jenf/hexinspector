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

enum diff_mode
{
  DIFF_MODE_SYNC,
  DIFF_MODE_UNSYNCED_NEAR,
  DIFF_MODE_UNSYNCED_FAR,
};

hi_diff *hi_diff_calculate(hi_file *src, hi_file *dst)
{
  off_t srcptr=0, dstptr=0, srcptr_new=0, dstptr_new=0;
  off_t srcptr_search, dstptr_search;
  off_t search_size;
  enum diff_mode mode = DIFF_MODE_SYNC;
  uint32_t hash = 0;
  off_t *value;  
  off_t current_value;
  int index;
  gboolean moved_since_hash_match = TRUE;
  int bytes_jump;
  
  bytes_jump = (dst->file_options.diff_jump_percent*dst->size)/100;
  if (bytes_jump < dst->file_options.hashbytes)
  {
    bytes_jump = dst->file_options.hashbytes * 2;
  }
  
  DPRINTF("Calculating diffs size %i %i Jumplimit %i\n", src->size, dst->size, bytes_jump);
  
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
          DPRINTF("Diff at %lu %lu\n",(unsigned long)  srcptr, (unsigned long) dstptr);
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
          value = g_hash_table_lookup(dst->buzhashes, hash);
          if (NULL != value)
          {
            DPRINTF("Found hash %lu %u\n", (unsigned long)srcptr, hash);
            
            /* Check for one after the current dstptr */
            for (index = 0; index < value[0]; index++)
            {
              VDPRINTF("Value at %lu %lu %lu %i\n",(unsigned long)srcptr, (unsigned long)dstptr, (unsigned long)value[index+1], index);
              if (value[index+1] >= dstptr)
              {
                if (value[index+1] < dstptr+bytes_jump)
                {
                  dstptr_new = value[index+1];
                  DPRINTF("Far Sync at %lu %lu %i\n",(unsigned long)srcptr, (unsigned long)dstptr_new, index);
                  mode = DIFF_MODE_SYNC;
                  moved_since_hash_match = FALSE;
                  break;
                }
                else
                {
                  DPRINTF("Would have jumped to far  %lu %lu %lu %i\n",(unsigned long)srcptr, (unsigned long)dstptr, (unsigned long)value[index+1], index);
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
      else
      {
      }
    }
    dstptr = dstptr_new;
    
  }
}