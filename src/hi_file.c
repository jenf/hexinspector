/* hi_file.c: Hexinspector
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
 * Basic file operations
 */

#include <hi_file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <hi_priv.h>
#include <string.h>

void hi_file_get_default_options(hi_file_options *options)
{
  options->hashbytes = 128;
  options->diff_jump_percent = 5;
  options->minimum_same = 4;
}

/**
 * \brief Open a hexinspector file instance
 * If options is NULL, default ones will be used
 */
hi_file *hi_file_open(char *filename,hi_file_options *options)
{
  int fd;
  hi_file *file;
  
  struct hi_file_options default_options;
  
  if (NULL == filename)
  {
    DPRINTF("Filename is NULL\n");
    return NULL;
  }
  
  if (NULL == options)
  {
    options = &default_options;
    hi_file_get_default_options(options);
  }

  DPRINTF("Opening file %s\n", filename);
  fd = open(filename, O_RDONLY);
  
  if (fd == -1)
  {
    DERRNO("Couldn't open file");
    return NULL;
  }



  file = calloc(sizeof(hi_file), 1);
  if (file == NULL)
  {
    DPRINTF("Couldn't malloc memory\n");
    goto close_fd;
  }
  file->size = lseek(fd, 0, SEEK_END);
  
  if (file->size == -1)
  {
    DERRNO("Couldn't get file size\n");
    goto free_structure;
  }

  file->filename=strdup(filename);
  if (file->filename == NULL)
  {
    DPRINTF("Couldn't malloc memory\n");
    goto free_structure;
  }
  
  file->file_options = *options;
  DPRINTF("About to map %i\n", file->size);
  file->memory = mmap(NULL, file->size, PROT_READ, MAP_PRIVATE, fd, 0);
  if ((void *)-1 == file->memory)
  {
    DPRINTF("Couldn't mmap file\n");
    goto free_filename;
  }
  close(fd);
  return file;
  
free_mmap:
  munmap(file->memory,file->size);
free_filename:
  free(file->filename);
free_structure:
  free(file);
close_fd:
  close(fd);
  return NULL;
}

/**
 * \brief Close and release all memory associated with a hexinspector file instance
 */
void hi_file_close(hi_file *file)
{
  if (file == NULL)
  {
    DPRINTF("File is NULL\n");
    return;
  }
  
  hi_hash_destroy(file);
  munmap(file->memory,file->size);
  file->memory = NULL;
  
  free(file->filename);
  file->filename = NULL;
  
  free(file);
}

