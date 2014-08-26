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

AC_INIT([libykneomgr], [0.1.5], [yubico-devel@googlegroups.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])

# http://www.gnu.org/s/libtool/manual/html_node/Updating-version-info.html
AC_SUBST([LT_CURRENT], 1)
AC_SUBST([LT_REVISION], 5)
AC_SUBST([LT_AGE], 1)

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
    [use specific backend/linkage; 'pcsc', 'macscard', 'winscard', or 'globalplatform'])],
    [],
    [with_backend=check])

case "$with_backend$host" in
  check*-darwin*)
    AC_MSG_NOTICE([Detected Mac: selecting macscard backend])
    AC_MSG_NOTICE([use --with-backend to override])
    with_backend=macscard ;;
  check*-mingw*)
    AC_MSG_NOTICE([Detected Windows: selecting winscard backend])
    AC_MSG_NOTICE([use --with-backend to override])
    with_backend=winscard ;;
esac

if test "x$with_backend" = xcheck || test "x$with_backend" = xpcsc; then
  PKG_CHECK_MODULES([PCSC], [libpcsclite],
                    [with_backend=pcsc], [:])
fi

if test "x$with_backend" = xcheck \
   || test "x$with_backend" = xglobalplatform; then
  PKG_CHECK_MODULES([GLOBALPLATFORM], [globalplatform],
                    [with_backend=globalplatform], [:])
fi

if test "x$with_backend" = xcheck; then
  AC_CHECK_HEADERS([PCSC/winscard.h])
  AC_MSG_CHECKING([between Mac/Windows winscard])
  if test "x$ac_cv_header_PCSC_winscard_h" = xyes; then
    with_backend=macscard
    AC_MSG_RESULT([Mac])
  else
    with_backend=winscard
    AC_MSG_RESULT([Windows])
  fi
fi

if test "x$with_backend" = xwinscard; then
  AC_MSG_NOTICE([checking for winscard with Windows linkage])
  AC_LIB_HAVE_LINKFLAGS(winscard,, [#include <winscard.h>],
                        [SCardBeginTransaction(0)])
  if test "x$ac_cv_libwinscard" != xyes; then
    AC_MSG_ERROR([cannot find Windows winscard library/headers])
  fi
fi

if test "x$with_backend" = xmacscard; then
  AC_MSG_NOTICE([checking for PCSC with Mac linkage])
  AC_CHECK_HEADERS([PCSC/winscard.h])
  PCSC_MACOSX_LIBS="-Wl,-framework -Wl,PCSC"
  save_LIBS="$LIBS"
  LIBS="$LIBS $PCSC_MACOSX_LIBS"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <PCSC/wintypes.h>
				    #include <PCSC/winscard.h>]],
				  [[SCardBeginTransaction(0)]])],
                 [AC_SUBST([PCSC_MACOSX_LIBS])],
                 [AC_MSG_ERROR([cannot find Mac PCSC library/headers])])
  LIBS="$save_LIBS"
fi

if test "x$with_backend" = xglobalplatform; then
  AC_DEFINE([BACKEND_GLOBALPLATFORM], 1,
            [Define to 1 if you the GLOBALPLATFORM backend.])
elif test "x$with_backend" = xpcsc || test "x$with_backend" = xwinscard \
  || test "x$with_backend" = xmacscard; then
  AC_DEFINE([BACKEND_PCSC], 1, [Define to 1 if you the PCSC backend.])
else
  AC_MSG_ERROR([cannot find PCSC/winscard library/headers])
fi

AM_CONDITIONAL([BACKEND_GLOBALPLATFORM], test x$with_backend = xglobalplatform)
AM_CONDITIONAL([BACKEND_PCSC], test x$with_backend != xglobalplatform)

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

PKG_CHECK_MODULES(LIBZIP, libzip)

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
  Mac PCSC
            LIBS:   ${PCSC_MACOSX_LIBS}
  libzip
          CFLAGS:   ${LIBZIP_CFLAGS}
            LIBS:   ${LIBZIP_LIBS}

])
