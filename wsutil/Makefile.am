# Makefile.am
#
# $Id$
#
# Wireshark - Network traffic analyzer
# By Gerald Combs <gerald@wireshark.org>
# Copyright 1998 Gerald Combs
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

ACLOCAL_AMFLAGS = `../aclocal-flags`

include Makefile.common

if HAVE_WARNINGS_AS_ERRORS
AM_CFLAGS = -Werror
endif

# Optional objects that I know how to build. These will be
# linked into libwsutil.
wsutil_optional_objects =	\
	@GETOPT_LO@ 		\
	@INET_ATON_LO@		\
	@INET_NTOP_LO@		\
	@INET_PTON_LO@		\
	@STRNCASECMP_LO@	\
	@STRPTIME_LO@

lib_LTLIBRARIES = libwsutil.la
libwsutil_la_LDFLAGS = -version-info 0:0:0 @LDFLAGS_SHAREDLIB@

INCLUDES = -I$(srcdir)/..

libwsutil_la_SOURCES =		\
	$(LIBWSUTIL_SRC)	\
	$(LIBWSUTIL_INCLUDES)

EXTRA_libwsutil_la_SOURCES=	\
	inet_aton.c		\
	inet_aton.h		\
	inet_ntop.c		\
	inet_pton.c		\
	inet_v6defs.h		\
	strncasecmp.c		\
	strptime.c		\
	strptime.h		\
	wsgetopt.c		\
	wsgetopt.h

libwsutil_la_DEPENDENCIES=	\
	$(wsutil_optional_objects)

libwsutil_la_LIBADD =		\
	@GLIB_LIBS@		\
	$(wsutil_optional_objects)

EXTRA_DIST =		\
	Makefile.common	\
	Makefile.nmake	\
	file_util.c	\
	file_util.h 	\
	libwsutil.def	\
	unicode-utils.c	\
	unicode-utils.h

CLEANFILES = \
	libwsutil.a	\
	libwsutil.la	\
	*~

MAINTAINERCLEANFILES = \
	Makefile.in

checkapi:
	$(PERL) ../tools/checkAPIs.pl -g abort -g termoutput \
	$(LIBWSUTIL_SRC)
#	file_util.c unicode-utils.c
