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

int main(int argc, char *argv[])
{
  size_t i,i2;
  uint32_t hash=0;
  uint32_t table[256]=BUZHASH_TABLE;
  hi_file *file;
  
  file = hi_open_file(argv[1]);
  
  for (i=0; i<file->size;i+=128)
  {
    hash=0;
    for (i2=i;(i2<file->size && i2<i+128);i2++)
    {
      hash = combine(hash, table[(int)file->memory[i2]], 128 % 32);
 
    }
    DPRINTF("%lu %u\n", i, hash);
  }
}
