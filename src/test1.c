#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "buzhash.h"

int main(int argc, char *argv[])
{
  int fd;
  struct stat buf;
  char *addr;
  int i,i2;
  uint32_t hash=0;
  uint32_t table[256]=BUZHASH_TABLE;
  
  fd = open(argv[1], O_RDONLY);
  fstat(fd, &buf);

  addr = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  for (i=0; i<buf.st_size;i+=128)
  {
    hash=0;
    for (i2=i;(i2<buf.st_size && i2<i+128);i2++)
    {
      hash = combine(hash, table[addr[i2]], 128 % 32);
 
    }
    printf("%i %u\n", i, hash);
  }
}
