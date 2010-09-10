/* hi_file.h: Hexinspector
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


/*
 * Structure definitions representing a file
 */

#ifndef HI_FILE_H
#define HI_FILE_H
#include <unistd.h>

/* Structures, Enums etc */
typedef struct hi_file
{
  char *filename; /**< Filename */
  size_t size;    /**< Size of file */
  char *memory;   /**< Memory for the hex file */
} hi_file;

/* Functions */
hi_file *hi_open_file(char *filename);
void hi_close_file(hi_file *file);

#endif