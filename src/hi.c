/* hi.c: Hexinspector
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
 * Test app
 */

#include <hi_file.h>
#include <hi_diff.h>
#include <stdint.h>
#include <macros.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <hi_ncurses.h>

void help(char *program_name)
{
  printf("Usage: %s [OPTION] <file1> [<file2>]\n", program_name);
  exit(0);
}

int main(int argc, char *argv[])
{

  int c;
  hi_file *file, *file2;
  hi_diff *diff = NULL;
  
  static struct option long_options[] =
  {
    {"help", no_argument, 0, 'h'},
    {0,0,0,0}
  };
  
  while (1)
  {
    int option_index = 0;
    c = getopt_long(argc, argv, "h",
                    long_options, &option_index);
    
    if (c == -1)
      break;
    
    switch (c)
    {
      case 'h':
        help(argv[0]);
        break;
        
      default:
        help(argv[0]);
    }
  }
  
  if ((argc - optind) < 1)
  {
    help(argv[0]);
  }
  
  file = hi_open_file(argv[optind], NULL);
  
  if (NULL == file)
  {
    fprintf(stderr,"Could not open %s\n", argv[optind]);
    exit(0);
  }
  
  if ((argc - optind) >=2)
  {
    file2 = hi_open_file(argv[optind+1], NULL);
    if (NULL == file2)
    {
      fprintf(stderr,"Could not open %s\n", argv[optind+1]);
      exit(0);
    }
  }

  if (file2 != NULL)
  {
    diff = hi_diff_calculate(file, file2);    
    
  }
  
#if 1
  hi_ncurses_main(file, file2, diff);
#endif
  hi_close_file(file2);
  hi_close_file(file);
  
}
