# Copyright (C) 2013 Yubico AB
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS=-I$(top_srcdir)/lib -I$(top_builddir)/lib

bin_PROGRAMS = ykneomgr

ykneomgr_SOURCES = ykneomgr.c
ykneomgr_SOURCES += cmdline.ggo cmdline.c cmdline.h
ykneomgr_LDADD = ../lib/libykneomgr.la

cmdline.c cmdline.h: cmdline.ggo Makefile.am
	gengetopt --no-handle-help --input $^

BUILT_SOURCES = cmdline.c cmdline.h
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

dist_man_MANS = ykneomgr.1
DISTCLEANFILES = $(dist_man_MANS)
EXTRA_DIST = ykneomgr.h2m

ykneomgr.1: $(srcdir)/ykneomgr.c $(srcdir)/ykneomgr.h2m $(srcdir)/cmdline.ggo $(top_srcdir)/configure.ac
	$(HELP2MAN) \
		--output=$@ $(builddir)/ykneomgr$(EXEEXT) \
		--name="YubiKey NEO management tool" \
		--include $(srcdir)/ykneomgr.h2m \
		--no-info
