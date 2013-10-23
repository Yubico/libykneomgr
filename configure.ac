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

AC_INIT([libykneomgr], [0.0.0], [yubico-devel@googlegroups.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

# http://www.gnu.org/s/libtool/manual/html_node/Updating-version-info.html
AC_SUBST([LT_CURRENT], 0)
AC_SUBST([LT_REVISION], 0)
AC_SUBST([LT_AGE], 0)

AM_INIT_AUTOMAKE([-Wall -Werror])
AM_SILENT_RULES([yes])
AC_PROG_CC
gl_EARLY
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_INIT([win32-dll])

GTK_DOC_CHECK([1.1])
AM_MISSING_PROG([HELP2MAN], help2man, $missing_dir)

AC_ARG_WITH([backend],
  [AS_HELP_STRING([--with-backend=ARG],
    [use specific backend; 'pcsc', 'winscard' or 'globalplatform'])],
    [],
    [with_backend=check])

if test "x$with_backend" = xcheck; then
   AC_MSG_CHECKING([for backend (guessing)])
   case "$host" in
    *-mingw*) with_backend=winscard ;;
    *) PKG_CHECK_EXISTS([globalplatform],
                        [with_backend=globalplatform],
                        [with_backend=pcsc]) ;;
   esac
   AC_MSG_RESULT([$with_backend])
fi

if test "x$with_backend" = "xglobalplatform"; then
  PKG_CHECK_MODULES([GLOBALPLATFORM], [globalplatform])
  AC_DEFINE([BACKEND_GLOBALPLATFORM], 1,
            [Define to 1 if you the GLOBALPLATFORM backend.])
elif test "x$with_backend" = "xpcsc"; then
  PKG_CHECK_MODULES([PCSC], [libpcsclite])
  AC_DEFINE([BACKEND_PCSC], 1, [Define to 1 if you the PCSC backend.])
elif test "x$with_backend" = "xwinscard"; then
  AC_LIB_HAVE_LINKFLAGS(winscard,, [#include <winscard.h>],
                        [SCardBeginTransaction(NULL)])
  AC_DEFINE([BACKEND_PCSC], 1, [Define to 1 if you the PCSC backend.])
else
  AC_MSG_ERROR([unknown backend: $with_backend])
fi
AM_CONDITIONAL([BACKEND_GLOBALPLATFORM], test x$with_backend = xglobalplatform)
AM_CONDITIONAL([BACKEND_PCSC], test x$with_backend = xpcsc)
AM_CONDITIONAL([BACKEND_WINSCARD], test x$with_backend = xwinscard)

gl_INIT

AC_ARG_ENABLE([gcc-warnings],
  [AS_HELP_STRING([--enable-gcc-warnings],
		  [turn on lots of GCC warnings (for developers)])],
  [case $enableval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $enableval for gcc-warnings option]) ;;
   esac
   gl_gcc_warnings=$enableval],
  [gl_gcc_warnings=no]
)

if test "$gl_gcc_warnings" = yes; then
  nw="$nw -Wsystem-headers"          # Ignore system headers
  nw="$nw -Wpadded"                  # Struct's arenot padded
  nw="$nw -Wc++-compat"              # We don't care strongly about C++
  nw="$nw -Wtraditional"             # Warns on #elif which we use often
  nw="$nw -Wtraditional-conversion"  # Too many warnings for now
  nw="$nw -Wconversion"              # Too many warnings for now
  nw="$nw -Wsuggest-attribute=pure"  # Is it worth using attributes?
  nw="$nw -Wsuggest-attribute=const" # Is it worth using attributes?

  gl_MANYWARN_ALL_GCC([ws])
  gl_MANYWARN_COMPLEMENT(ws, [$ws], [$nw])
  for w in $ws; do
    gl_WARN_ADD([$w])
  done

  gl_WARN_ADD([-fdiagnostics-show-option])
fi

AC_SUBST([YKNEOMGR_VERSION_MAJOR],
	 `echo $PACKAGE_VERSION | sed 's/\(.*\)\..*\..*/\1/g'`)
AC_SUBST([YKNEOMGR_VERSION_MINOR],
	 `echo $PACKAGE_VERSION | sed 's/.*\.\(.*\)\..*/\1/g'`)
AC_SUBST([YKNEOMGR_VERSION_PATCH],
	 `echo $PACKAGE_VERSION | sed 's/.*\..*\.\(.*\)/\1/g'`)
AC_SUBST([YKNEOMGR_VERSION_NUMBER],
	 `printf "0x%02x%02x%02x" $YKNEOMGR_VERSION_MAJOR \
		 $YKNEOMGR_VERSION_MINOR $YKNEOMGR_VERSION_PATCH`)

AC_CONFIG_FILES([
  Makefile
  gl/Makefile
  lib/Makefile
  lib/ykneomgr/Makefile
  lib/ykneomgr/ykneomgr-version.h
  lib/ykneomgr.pc
  src/Makefile
  tests/Makefile
  gtk-doc/Makefile
])
AC_OUTPUT

AC_MSG_NOTICE([summary of build options:

  Version:          ${VERSION}
     Library:       current $LT_CURRENT revision $LT_REVISION age $LT_AGE
     Header:        major $YKNEOMGR_VERSION_MAJOR minor $YKNEOMGR_VERSION_MINOR patch $YKNEOMGR_VERSION_PATCH number $YKNEOMGR_VERSION_NUMBER
  Host type:        ${host}
  Install prefix:   ${prefix}
  Compiler:         ${CC}
           CFLAGS:  ${CFLAGS}
         CPPFLAGS:  ${CPPFLAGS}
         Warnings:  ${WARN_CFLAGS}
  Library types:    Shared=${enable_shared}, Static=${enable_static}
  Backend:          ${with_backend}
  GlobalPlatform
          CFLAGS:   ${GLOBALPLATFORM_CFLAGS}
            LIBS:   ${GLOBALPLATFORM_LIBS}
  PCSC
          CFLAGS:   ${PCSC_CFLAGS}
            LIBS:   ${PCSC_LIBS}
  Winscard
            LIBS:   ${LTLIBWINSCARD}
])
