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

#ifndef HI_MACRO_H
#define HI_MACRO_H


#ifdef HAVE_DEBUG
#include <errno.h>
#include <string.h>
#include <stdio.h>
#define DPRINTF(...) fprintf(stderr,__VA_ARGS__)
#define DERRNO(fmt) fprintf(stderr,"%s:%s\n", fmt, strerror(errno));
#else
#define DPRINTF(...)
#define DERRNO(...)
#endif

#ifdef VERBOSE_DEBUG
#define VDPRINTF(...) DPRINTF(__VA_ARGS__)
#else
#define VDPRINTF(...)
#endif

/* Branch predicition markers */
#if __GNUC__ < 3
#define __builtin_expect(x, n) (x)
#else
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x)   __builtin_expect(!!(x), 0)
#endif

#endif
