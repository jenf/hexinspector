# Process this file with automake to produce Makefile.in
## Copyright (c) 2010 Jen Freeman

## $Id$

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
##
## Author contact information:
##
## Web: http://github.com/jenf/hexinspector

AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = hi

hi_SOURCES = hi.c hi_file.c hi_buzhash.c hi_diff.c

hi_SOURCES += hi_ncurses_main.c hi_ncurses_file_pager.c

hi_CFLAGS = $(GLIB_CFLAGS)
hi_LDADD = $(GLIB_LIBS) $(NCURSES_LIBS)