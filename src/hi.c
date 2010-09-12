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

int main(int argc, char *argv[])
{

  hi_file *file, *file2;
  hi_diff *diff = NULL;
  
  file = hi_open_file(argv[1], NULL);
  file2 = hi_open_file(argv[2], NULL);

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
