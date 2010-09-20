/* hi_search.c: Hexinspector
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
 * Search a file for a regular expression
 */
#include <sys/types.h>
#include <hi_file.h>
#include <hi_search.h>
#include <macros.h>
#ifndef NO_PCRE
#include <pcre.h>

#define OVECCOUNT 30

static const char *errors[] = {"No match","Empty pattern"};
static gboolean hi_search_exec(hi_file *file,
                               off_t begin_offset,
                               off_t *found_offset,
                               pcre *re);
gboolean hi_search_compile_and_exec(hi_file *file,
                                    char *pattern,
                                    off_t begin_offset,
                                    off_t *found_offset,
                                    const char **error)
{
  gboolean ret;
  pcre *re;
  int f_offset;
  *found_offset = 0;
  *error = NULL;
  if (strcmp(pattern,"")==0)
  {
    *error = errors[1];
    return FALSE;
  } 
  re = pcre_compile(pattern, 0, error, &f_offset, NULL);
  *found_offset = f_offset;
  if (re == NULL)
  {
    VDPRINTF("Failed pattern compile due to %s\n", *error);
    return FALSE;
  }
  ret = hi_search_exec(file, begin_offset, &f_offset, re);
  *found_offset = f_offset;
  if (ret == FALSE)
  {
    *error = errors[0];
  }
  
  pcre_free(re);
  return ret;
}

static gboolean hi_search_exec(hi_file *file,
                               off_t begin_offset,
                               off_t *found_offset,
                               pcre *re)
{
  int rc;
  int ovector[OVECCOUNT];
  rc = pcre_exec(re, NULL, (const char *)file->memory, file->size, begin_offset, 0,
                 ovector, OVECCOUNT);
  if (rc < 0)
  {
    VDPRINTF("Could not find due to %i\n", rc);
    return FALSE;
  }
  *found_offset = ovector[0];
  VDPRINTF("Found at %i\n", *found_offset);
  return TRUE;
}

#else

static char disabled[] = "Search support disabled";

gboolean hi_search_compile_and_exec(hi_file *file,
                                    char *pattern,
                                    off_t begin_offset,
                                    off_t *found_offset,
                                    const char **error)
{
  *error = disabled;
  return FALSE;
}

#endif
