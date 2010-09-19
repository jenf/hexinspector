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
#ifndef NO_PCRE
#include <hi_file.h>
#include <hi_search.h>
#include <macros.h>
#include <pcre.h>

#define OVECCOUNT 30

static char no_match[] = "No match";
static gboolean hi_search_exec(hi_file *file,
                               off_t begin_offset,
                               off_t *found_offset,
                               pcre *re);
gboolean hi_search_compile_and_exec(hi_file *file,
                                    char *pattern,
                                    off_t begin_offset,
                                    off_t *found_offset,
                                    char **error)
{
  gboolean ret;
  pcre *re;
  *found_offset = 0;
  *error = NULL;
  re = pcre_compile(pattern, 0, error, found_offset, NULL);
  if (re == NULL)
  {
    VDPRINTF("Failed pattern compile due to %s\n", *error);
    return FALSE;
  }
  ret = hi_search_exec(file, begin_offset, found_offset, re);
  *error = no_match;
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
                                    char **error)
{
  *error = disabled;
  return FALSE;
}

#endif