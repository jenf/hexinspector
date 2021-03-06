/* <filename>: Hexinspector
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
 * Structure overview
 */

#ifndef HI_DIFF_H
#define HI_DIFF_H

#include <sys/types.h>
#include <unistd.h>
/** Structure representing the differences between two files */
typedef struct hi_diff
{
  hi_file *src;                     /**< Source file */
  hi_file *dst;                     /**< Destination file */
  GTree /*<hi_diff_hunk>*/ *hunks;  /**<List of hunks of type hi_diff_hunk */
  GList /*<hi_diff_hunk>*/ *working_hunks; /**< List of hunks used when constructing the diff */
  struct hi_diff_hunk *last_hunk;          /**< Cached last requested hunk (performance improvement) */
} hi_diff;

/** The various states that a hunk can be in */
enum hi_diff_type
{
  HI_DIFF_TYPE_DIFF,              /* The file is different between the points */
  HI_DIFF_TYPE_SAME,              /* The file was the same between the points */
  HI_DIFF_TYPE_SAME_BACKTRACKED,  /* The file was the same between the points (backtracked) */
  HI_DIFF_TYPE_SAME_INDEXED,      /* The file was the same between the points, but the backtracking algorithm hasn't been applied */
  HI_DIFF_FIND_SRC,               /* A bad trick in glib to find the source easily */
  HI_DIFF_FIND_DST                /* A bad trick in glib to find the destination easily */
};

enum hi_diff_algorithm
{
  HI_DIFF_ALG_RABINKARP,         /* Rabin Karp algorithm */
  HI_DIFF_ALG_SIMPLE             /* Simple algorithm no insertion or deletions expected */
};

/** Structure representing a diff hunk */
typedef struct hi_diff_hunk
{
  enum hi_diff_type type;
  off_t src_start;
  off_t src_end;
  off_t dst_start;
  off_t dst_end;
} hi_diff_hunk;
  


hi_diff *hi_diff_calculate(hi_file *src, hi_file *dst, enum hi_diff_algorithm algorithm);
hi_diff_hunk *hi_diff_get_hunk(hi_diff *diff,
                               hi_file *file,
                               off_t pos);

#endif
