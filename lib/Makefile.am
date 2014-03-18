# Copyright (C) 2013-2014 Yubico AB
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

SUBDIRS = ykneomgr

AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(srcdir)/ykneomgr -I$(builddir)/ykneomgr
AM_CPPFLAGS += -I$(top_srcdir)/gl -I$(top_builddir)/gl
AM_CPPFLAGS += $(GLOBALPLATFORM_CFLAGS) $(PCSC_CFLAGS) $(LIBZIP_CFLAGS)

lib_LTLIBRARIES = libykneomgr.la

libykneomgr_la_SOURCES = internal.h
libykneomgr_la_SOURCES += ykneomgr.pc.in ykneomgr.map
libykneomgr_la_SOURCES += global.c version.c error.c devs.c

if BACKEND_GLOBALPLATFORM
libykneomgr_la_SOURCES += backend_globalplatform.c
endif
if BACKEND_PCSC
libykneomgr_la_SOURCES += backend_pcsc.c
endif

libykneomgr_la_LIBADD = $(GLOBALPLATFORM_LIBS) $(PCSC_LIBS) $(LTLIBWINSCARD)
libykneomgr_la_LIBADD += $(PCSC_MACOSX_LIBS) $(LIBZIP_LIBS)
libykneomgr_la_LIBADD += ../gl/libgnu.la
libykneomgr_la_LDFLAGS = -no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

if HAVE_LD_VERSION_SCRIPT
libykneomgr_la_LDFLAGS += -Wl,--version-script=$(srcdir)/ykneomgr.map
else
libykneomgr_la_LDFLAGS += -export-symbols-regex '^ykneomgr_.*'
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ykneomgr.pc
