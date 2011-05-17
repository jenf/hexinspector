/* hi_hash.c: Hexinspector
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
#define BENCHMARK
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

/* Module variables */
static GStaticMutex M_Mutex = G_STATIC_MUTEX_INIT;

#ifdef STATS
static int M_max_len = 0;
#endif


#ifdef BENCHMARK
static int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y);

/* Subtract the `struct timeval' values X and Y,
 storing the result in RESULT.
 Return 1 if the difference is negative, otherwise 0.  */

static int timeval_subtract (struct timeval *result, struct timeval *x,struct timeval  *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }
  
  /* Compute the time remaining to wait.
   tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;
  
  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}
#endif

/**
 * \brief Free the hash value
 */
static gboolean merge_hashes(gpointer key, off_t *value, gpointer data)
{
  GHashTable *globalhash = data;
  off_t *globalvalue, *original;
  globalvalue = g_hash_table_lookup(globalhash, key);
  if (NULL == globalvalue)
  {
    /* Just add */
    g_hash_table_insert(globalhash, key, value);
  }
  else
  {
    
#if 0
    int i;

    for (i=1; i<=value[0]; i++)
    {
      printf("local %x %i %lu\n", key, i, (unsigned long) value[i]);
    }
#endif
    /* Must merge values together */    
    original = globalvalue;
    globalvalue = realloc(globalvalue, sizeof(off_t)*(value[0]+globalvalue[0]+1));
    VDPRINTF("%x %x %x\n",globalvalue+globalvalue[0]+1, value+1, value[0]*sizeof(off_t));
    if (globalvalue[1] < value[1])
    {
      memcpy(globalvalue+globalvalue[0]+1, value+1, value[0]*sizeof(off_t));
    }
    else
    {
      /* Ensure the order of the array is valid */
      VDPRINTF("Swap order %lu and %lu\n", (long unsigned) globalvalue[1], (long unsigned) value[1]);
      memmove(globalvalue+value[0]+1, globalvalue+1, globalvalue[0]*sizeof(off_t));
      memcpy(globalvalue+1, value+1, value[0]*sizeof(off_t));

    }
    globalvalue[0]+=value[0];    
    
#ifdef STATS
    M_max_len = MAX(M_max_len, globalvalue[0]);
#endif  
    
#if 0
    for (i=1; i<=globalvalue[0]+value[0]; i++)
    {
      printf("post %x %i %lu\n", key, i, (unsigned long) globalvalue[i]);
    }
#endif
    if (globalvalue != original)
    {
      g_hash_table_insert(globalhash, key, globalvalue);      
    }
    

    free(value);
  }
  return TRUE;
}

/**
 *
 * \brief Calculate the hash for the block
 */
static void hi_hash_generate_thread(gpointer instance_data,
                                    gpointer run_data)
{
  hi_file *file = run_data;
  size_t start = ((size_t *) instance_data)[0];
  size_t length = ((size_t *) instance_data)[1];
  size_t hash_start_offset;
  size_t i2;
#ifndef USE_RABINKARP
  static uint32_t table[256]=BUZHASH_TABLE;
#endif  
  uint32_t hash=0;
  off_t *value,*original_value;
  GHashTable *hashtable;
  
  /* Must use a local hash to avoid scalability problems on the mutex lock */
  hashtable = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
  
  for (hash_start_offset=start; hash_start_offset<length;hash_start_offset+=file->file_options.hashbytes)
  {
    hash=0;
//    DPRINTF("Hash %lu", (unsigned long) hash_start_offset);
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
      
    original_value = value = g_hash_table_lookup(hashtable, hash);
    
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

    value[value[0]]=hash_start_offset;
    
    if (value != original_value)
    {
      VDPRINTF("Inserting %x\n", value);
      g_hash_table_insert(hashtable, hash, value);      
    }
  }
  
  /* Move the local hash into the global hash */
  g_static_mutex_lock(&M_Mutex);
  g_hash_table_foreach_remove(hashtable, merge_hashes, file->buzhashes);
  g_static_mutex_unlock(&M_Mutex);
  g_hash_table_destroy(hashtable);
}

/**
 * \brief Hash prelude functions
 */
gboolean hi_hash_prelude(hi_file *file)
{
#ifdef USE_RABINKARP  
  file->file_options.popvalue = rabinkarp_calculate_popvalue(file->file_options.hashbytes, PRIME, BASE);
#endif
}

/**
 * \brief Buzhash generation
 */
gboolean hi_hash_generate(hi_file *file)
{
  size_t block_start,i2;
  uint32_t hash=0;

  off_t *value,*original_value;

  GThreadPool *pool;
  int max_threads = 4; /* TODO: Pass as a variable */
  int blocksize = file->file_options.hashbytes*1024; /* TODO: Pass as a variable */
  size_t *start_offsets;
  int i;
#ifdef BENCHMARK
  struct timeval starttime, endtime, difftime;
  float timing;
  gettimeofday(&starttime, NULL);
#endif

#ifdef STATS
  M_max_len = 0;
#endif
  file->buzhashes = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
  if (NULL == file->buzhashes)
  {
    DPRINTF("Buzhash hash table did not create\n");
    return FALSE;
  }
  
  pool = g_thread_pool_new(hi_hash_generate_thread, (gpointer) file, max_threads, FALSE, NULL);
  start_offsets = calloc((file->size/file->file_options.hashbytes)+1,2*sizeof(size_t));
  i=0;
  
  for (block_start=0; block_start<file->size;block_start+=blocksize)
  {
    start_offsets[i]=block_start;
    start_offsets[i+1]=block_start+blocksize;
    g_thread_pool_push(pool, (gpointer)&(start_offsets[i]), NULL);
    i+=2; 
  }
  
  DPRINTF("Waiting for completion\n");
  /* Wait for completion */
  g_thread_pool_free(pool, FALSE, TRUE);
  
  DPRINTF("Hash Jobs finished\n");
  free(start_offsets);
  
#ifdef STATS
  guint size = g_hash_table_size(file->buzhashes);
  DPRINTF("Hash items %lu size %u, biggest hash %i\n", file->size/file->file_options.hashbytes, size, M_max_len);
#endif
#ifdef BENCHMARK
  gettimeofday(&endtime, NULL);
  timeval_subtract(&difftime, &endtime, &starttime);
  timing = difftime.tv_sec + ((float) difftime.tv_usec/1000000);
  float mbytes = file->size /1024.0/1024.0;
  printf("Hash Time taken %04f seconds, %f mbytes, %f mbytes/sec\n", timing, mbytes, mbytes/timing);
#endif
//  exit(1);
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
void hi_hash_destroy(hi_file *file)
{
  if (file->buzhashes != NULL)
  {
    g_hash_table_foreach_remove(file->buzhashes,destroy_hash_value,NULL);
    g_hash_table_destroy(file->buzhashes);
    file->buzhashes = NULL;
  }
}
