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
 * Hexinspector main app
 */

#define AUTHORS \
  "Jennifer Freeman"

#include <hi_file.h>
#include <hi_diff.h>
#include <stdint.h>
#include <macros.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <hi_ncurses.h>

void version(char *program_name)
{
  fprintf(stderr, "%s version %s\n"
          "This is free software; see the source for copying conditions.  There is NO\n"
          "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
          "See: http://github.com/jenf/hexinspector\n"
          "Authors: %s\n", program_name, PACKAGE_VERSION, AUTHORS);
  exit(0);
}

void help(char *program_name)
{
  hi_file_options options;
  hi_file_get_default_options(&options);
  fprintf(stderr, "Usage: %s [OPTION] <file1> [<file2>]\n", program_name);
  fprintf(stderr, "Hex file viewer/comparer.\n\n"
         " -h/--help          This help text\n"
         " -b/--hashbytes=num Number of bytes that are used in each hash, lower=more memory, default %i\n"
         " -d/--diff_jump_limit=num Percentage of the file that a diff is allowed to jump, default %f\n"
         " -m/--minimum_same=num Minimum number of bytes that must be the same in order to be considered back in sync, default %i\n",
         options.hashbytes, options.diff_jump_percent, options.minimum_same);
  exit(0);
}

int main(int argc, char *argv[])
{

  int c;
  hi_file *file = NULL, *file2 = NULL;
  hi_diff *diff = NULL;
  hi_file_options options;
  
  static struct option long_options[] =
  {
    {"help", no_argument, 0, 'h'},
    {"hashbytes", required_argument, 0, 'b'},
    {"diff_jump_limit", required_argument, 0, 'd'},
    {"minimum_same", required_argument, 0, 'm'},
    {"version", no_argument, 0, 'v'},
    {0,0,0,0}
  };
  
  hi_file_get_default_options(&options);
  
  while (1)
  {
    int option_index = 0;
    c = getopt_long(argc, argv, "hvb:d:m:",
                    long_options, &option_index);
    
    if (c == -1)
      break;
    
    switch (c)
    {
      case 'h':
        help(argv[0]);
        break;
        
      case 'v':
        version(argv[0]);
        break;
        
      case 'b':
        options.hashbytes = atoi(optarg);
        if (options.hashbytes < 16)
        {
          fprintf(stderr,"Hash size less than 16 is not recommended\n");
          help(argv[0]);
        }
        break;
        
      case 'd':
        options.diff_jump_percent = atof(optarg);
        if (options.diff_jump_percent < 0.0)
        {
          fprintf(stderr,"Maximum diff jump percentage must be greater than 0%%\n");
          help(argv[0]);
        }
        else if (options.diff_jump_percent > 100.0)
        {
          fprintf(stderr,"Maximum diff jump percentage must be less than 100%%\n");
          help(argv[0]);
        }
        break;
        
      case 'm':
        options.minimum_same = atoi(optarg);
        if (options.minimum_same < 1)
        {
          fprintf(stderr,"Minimum same must be greater than 1\n");
          help(argv[0]);
        }
        break;
      default:
        help(argv[0]);
    }
  }
  
  if ((argc - optind) < 1)
  {
    help(argv[0]);
  }
  
  options.generate_hash = FALSE;
  file = hi_file_open(argv[optind], &options);
  
  if (NULL == file)
  {
    fprintf(stderr,"Could not open %s\n", argv[optind]);
    exit(0);
  }
  if ((argc - optind) >=2)
  {
    options.generate_hash = TRUE;
    file2 = hi_file_open(argv[optind+1], &options);
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
  hi_file_close(file2);
  hi_file_close(file);
  return 0;
}
