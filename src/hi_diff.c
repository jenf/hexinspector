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
  
  DPRINTF("Calculating diffs size %i %i\n", src->size, dst->size);
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
        for (search_size=0;search_size<dst->file_options.hashbytes;search_size++)
        {
          if (DIFF_MODE_UNSYNCED_NEAR != mode) {break;}
          
          DPRINTF("Search %lu\n", search_size);
          for (srcptr_search = 0; srcptr_search < search_size; srcptr_search++)
          {
            dstptr_search = search_size - (srcptr_search);
            if (srcptr+srcptr_search > src->size) {continue;}
            if (dstptr+dstptr_search > dst->size) {continue;}      
            
            DPRINTF("Search %lu %lu %lu\n", (unsigned long) search_size, (unsigned long)srcptr_search, (unsigned long)dstptr_search);
            if (src->memory[srcptr+srcptr_search] == dst->memory[dstptr+dstptr_search])
            {
              srcptr_new = srcptr+srcptr_search;
              dstptr_new = dstptr+dstptr_search;
              mode = DIFF_MODE_SYNC;
              DPRINTF("Resync at %lu %lu\n", (unsigned long) srcptr_new, (unsigned long) dstptr_new);
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
        
        default:
        srcptr_new++;
    }
    
    /* TODO: Update rolling buzhash */
    srcptr = srcptr_new;
    dstptr = dstptr_new;
    
  }
}