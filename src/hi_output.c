/* hi_output.c: Hexinspector
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
 * Hexinspector command line output module
 */

#define AUTHORS \
  "Jennifer Freeman"

#include <hi_file.h>
#include <hi_diff.h>
#include <stdint.h>
#include <macros.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct hi_diff_output {
    hi_diff *diff;
    FILE *fp;
} hi_diff_output;

static gboolean hi_output_diff_each(gpointer key, gpointer value, gpointer data)
{
    hi_diff_output *output = data;
    hi_diff_hunk *hunk = value;
    char *mode = NULL;
    off_t i;
    gboolean first;
    
    
    if (hunk->type == HI_DIFF_TYPE_DIFF)
    { 
        fprintf(output->fp, "@@ -%lu,%lu +%lu,%lu @@",
                (unsigned long)hunk->src_start, (unsigned long)(hunk->src_end-hunk->src_start),
                (unsigned long)hunk->dst_start, (unsigned long)(hunk->dst_end-hunk->dst_start));
        first = TRUE;
        for (i=hunk->src_start-32; i < hunk->src_start; i++)
        {
            if (i>0)
            {
                if (first || (i-hunk->src_start) % 32 == 0)
                {
                    first = FALSE;
                    fprintf(output->fp,"\n %02x", output->diff->src->memory[i]);
                }
                else
                    fprintf(output->fp," %02x", output->diff->src->memory[i]);
            }
        }
        for (i=hunk->src_start; i <= hunk->src_end; i++)
        {
            if ((i-hunk->src_start) % 32 == 0)
            {
                fprintf(output->fp,"\n-%02x", output->diff->src->memory[i]);
            }
            else
                fprintf(output->fp," %02x", output->diff->src->memory[i]);
        }
        for (i=hunk->dst_start; i <= hunk->dst_end; i++)
        {
            if ((i-hunk->dst_start) % 32 == 0)
            {
                fprintf(output->fp,"\n+%02x", output->diff->dst->memory[i]);
            }
            else
                fprintf(output->fp," %02x", output->diff->dst->memory[i]);
        }
        for (i=hunk->src_end+1; i < hunk->src_end+1+32; i++)
        {
            if (i < output->diff->src->size)
            {
                if ((i-(hunk->src_end+1)) % 32 == 0)
                    fprintf(output->fp,"\n %02x", output->diff->src->memory[i]);
                else
                    fprintf(output->fp," %02x", output->diff->src->memory[i]);
            }
        }
        fprintf(output->fp,"\n");

    }
    return FALSE;
}

void hi_output_diff(hi_file *file, hi_file *file2, hi_diff *diff, char *filename)
{
    FILE *fp = stdout;
    hi_diff_output output;
    
    if (filename != NULL)
    {
        fp = fopen(filename, "w");
    }

    if (fp == NULL)
    {
        fprintf(stderr, "Could not open %s for writing\n");
        exit(1);
    }

    output.fp = fp;
    output.diff = diff;

    fprintf(fp, "--- %s\n", file->filename);
    fprintf(fp, "+++ %s\n", file2->filename);
    g_tree_foreach(diff->hunks, hi_output_diff_each, &output);
    if (filename != NULL)
    {
        fclose(fp);
    }
}
