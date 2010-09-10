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

#include <hi_file.h>
#include <hi_priv.h>
#include <unistd.h>
#include <stdint.h>
#include <buzhash.h>

/**
 * \brief Free the hash value
 */
static void destroy_hash_value(gpointer value)
{
  free(value);
}

/**
 * \brief Buzhash generation
 */
gboolean hi_buzhash_generate(hi_file *file)
{
  size_t i,i2;
  uint32_t hash=0;
  uint32_t table[256]=BUZHASH_TABLE;
  off_t *value;
  
  file->buzhashes = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, destroy_hash_value);
  if (NULL == file->buzhashes)
  {
    DPRINTF("Buzhash hash table did not create\n");
    return FALSE;
  }
  
  for (i=0; i<file->size;i+=128)
  {
    hash=0;
    for (i2=i;(i2<file->size && i2<i+128);i2++)
    {
      hash = combine(hash, table[(int)file->memory[i2]], 128 % 32);
    }
    DPRINTF("%lu %x %x\n", i, hash, value);
    
    value = g_hash_table_lookup(file->buzhashes, hash);
    
    /* Hashes should be relatively sparse in terms of collisions, therefore use a simple array, at least for now */
    

  }
  
  return TRUE;
}