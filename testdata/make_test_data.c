#include <stdio.h>
#include <unistd.h>
#include <string.h>
enum test_data {
        test_data_zeros,
        test_data_ones,
        test_data_alignment1, /* Performance of non-aligned data fetches */
        test_data_sparse_ones,
        test_data_oddzero_evenone,
        test_data_last
};

int make_test_data(enum test_data data_type, int len)
{
  char *filename = NULL;
#define DATALEN (4096)
  char pattern[DATALEN];
  int i;

  switch (data_type)
  {
  case test_data_zeros:
    filename="test_data_zeros";
    memset(pattern, 0, DATALEN);
    break;
  case test_data_ones:
    filename="test_data_ones";
    memset(pattern, 1, DATALEN);
    break;
  case test_data_alignment1:
    filename="test_data_alignment1";
    memset(pattern, 0, DATALEN);
    pattern[4] = 1;
    break;
  case test_data_oddzero_evenone:
    filename="test_data_oddzero_evenone";
    for (i=0; i < DATALEN - (3+6); i+= (3+6))
    {
      memset(pattern+i, 0, 3);
      memset(pattern+i+3, 1, 6);
    }
  }

  if (filename != NULL)
  {
    FILE *fp;
    if (0 != access(filename, F_OK))
    {
        printf("Creating Test %s\n", filename);
        fp = fopen(filename, "w");
        for (i = 0; i < len; i+= DATALEN)
        {
            if ( (i == DATALEN) && data_type == test_data_alignment1)
            {
              memset(pattern, 0, DATALEN);
            }
            fwrite(pattern, DATALEN, 1, fp);
        }
        fclose(fp);
    }
    else
    {
      printf("Test data %s already exists\n", filename);
    }
  }
}

int main(void)
{
  enum test_data data;
  
  for (data = 0; data < test_data_last; data++)
  {
    make_test_data(data, 32*1024*1024);
  }
}
