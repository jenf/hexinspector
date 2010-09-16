/* hi_buzhash.c: Hexinspector
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

//#define VERBOSE_DEBUG
#define STATS
#include <sys/types.h>
#include <hi_file.h>
#include <hi_priv.h>
#include <unistd.h>
#include <stdint.h>
#include <buzhash.h>
#include <stdlib.h>

#ifdef USE_RABINKARP
#include <rabinkarp.h>

#endif

/**
 * \brief Buzhash generation
 */
gboolean hi_buzhash_generate(hi_file *file)
{
  size_t hash_start_offset,i2;
  uint32_t hash=0;
  uint32_t table[256]=BUZHASH_TABLE;
  off_t *value,*original_value;
#ifdef STATS
  int max_len = 0;
#endif
  
#ifdef USE_RABINKARP  
  file->file_options.popvalue = rabinkarp_calculate_popvalue(file->file_options.hashbytes, PRIME, BASE);
#endif
  
  file->buzhashes = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
  if (NULL == file->buzhashes)
  {
    DPRINTF("Buzhash hash table did not create\n");
    return FALSE;
  }
  
  for (hash_start_offset=0; hash_start_offset<file->size;hash_start_offset+=file->file_options.hashbytes)
  {
    hash=0;
    for (i2=hash_start_offset;(i2<file->size && i2<hash_start_offset+file->file_options.hashbytes);i2++)
    {
#ifdef USE_RABINKARP
      hash = rabinkarp_add(hash, (unsigned char)file->memory[i2], PRIME, BASE);
#else
      hash = combine(hash, table[(unsigned char)file->memory[i2]], 1);
#endif
//      VDPRINTF("hash1 %lu %lu\n", (unsigned long) i2, hash);
    }
    VDPRINTF("hash %lu %x\n", (unsigned long) hash_start_offset, hash);
    
    original_value = value = g_hash_table_lookup(file->buzhashes, hash);
    
    /* Hashes should be relatively sparse in terms of collisions, therefore use a simple array, at least for now */
    if (NULL == value)
    {
      value = calloc(sizeof(off_t),2);
      /* TODO: Fix the check here */
      value[0]=1;
    }
    else
    {
      value = realloc(value, sizeof(off_t)*(value[0]+2));
      value[0]+=1;
    }
    
   VDPRINTF("Adding byte %i hash %x len %i\n", hash_start_offset, hash, value[0]);
#ifdef STATS
    max_len = MAX(max_len, value[0]);
#endif
    value[value[0]]=hash_start_offset;
    
    if (value != original_value)
    {
      VDPRINTF("Inserting %x\n", value);
      g_hash_table_insert(file->buzhashes, hash, value);      
    }

    

  }
  
#ifdef STATS
  guint size = g_hash_table_size(file->buzhashes);
  DPRINTF("Hash items %lu size %u, biggest hash %i\n", file->size/file->file_options.hashbytes, size, max_len);
#endif
  return TRUE;
}

/**
 * \brief Free the hash value
 */
static gboolean destroy_hash_value(gpointer key, gpointer value, gpointer data)
{
  free(value);
  return TRUE;
}

/**
 * \brief Destroy the buzhash data
 */
void hi_buzhash_destroy(hi_file *file)
{
  if (file->buzhashes != NULL)
  {
    g_hash_table_foreach_remove(file->buzhashes,destroy_hash_value,NULL);
    g_hash_table_destroy(file->buzhashes);
    file->buzhashes = NULL;
  }
}
