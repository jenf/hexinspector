#include <hi_file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "buzhash.h"
#include <macros.h>

/**
 * \brief Open a hexinspector file instance
 */
hi_file *hi_open_file(char *filename)
{
  int fd, ret;
  struct stat buf;
  hi_file *file;
  
  if (filename == NULL)
  {
    DPRINTF("Filename is NULL\n");
    return NULL;
  }

  DPRINTF("Opening file %s\n", filename);
  fd = open(filename, O_RDONLY);
  
  if (fd == -1)
  {
    DERRNO("Couldn't open file");
    return NULL;
  }
  
  ret = fstat(fd, &buf);
  
  if (ret == -1)
  {
    DERRNO("Couldn't fstat file");
    goto close_fd;
  }
  file = malloc(sizeof(hi_file));
  if (file == NULL)
  {
    DPRINTF("Couldn't malloc memory\n");
    goto close_fd;
  }
  
  file->filename=strdup(filename);
  if (file->filename == NULL)
  {
    DPRINTF("Couldn't malloc memory\n");
    goto free_structure;
  }
  
  file->size = buf.st_size;
  file->memory = mmap(NULL, file->size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (NULL == file->memory)
  {
    DPRINTF("Couldn't mmap file\n");
    goto free_filename;
  }
  
  close(fd);
  return file;

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
void hi_close_file(hi_file *file)
{
  if (file == NULL)
  {
    DPRINTF("File is NULL\n");
    return;
  }
  
  munmap(file->memory,file->size);
  file->memory = NULL;
  
  free(file->filename);
  file->filename = NULL;
  
  free(file);
}

