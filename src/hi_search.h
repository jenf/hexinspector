/* hi_search.h: Hexinspector
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
 * Search functionality currently depends on pcre
 */

#ifndef HI_SEARCH_H
#define HI_SEARCH_H

gboolean hi_search_compile_and_exec(hi_file *file,
                                    char *pattern,
                                    off_t begin_offset,
                                    off_t *found_offset,
                                    char **error);
#endif
