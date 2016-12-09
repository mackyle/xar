#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Xar-Python], [2.0.0], [kuba@kaszycki.net.pl], [xar-python])
AC_CONFIG_SRCDIR([xarfile.pyx])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([../build-aux])
AC_CONFIG_MACRO_DIR([../m4])
AM_INIT_AUTOMAKE([foreign])
LT_INIT

AM_MAINTAINER_MODE([enable])

# Checks for programs.
AC_PROG_CC
PKG_PROG_PKG_CONFIG
HAVE_PYTHON=yes
AX_PYTHON([> '2.7' and ver < '3'])
AS_IF([test "$ax_python_bin" = no], [HAVE_PYTHON=])
AX_PYTHON_DEVEL([> '2.7' and ver < '3'])
AC_CHECK_PROGS([PYREXC], [pyrexc2.7 pyrexc2 pyrexc], [--NONE--])
AS_IF([test "$PYREXC" = "--NONE--"], [HAVE_PYTHON=])
AM_CONDITIONAL([HAVE_PYTHON], [test -n "$HAVE_PYTHON"])

# Checks for libraries.
PKG_CHECK_MODULES([LIBXML2], [libxml-2.0 >= 2.6.11])
PKG_CHECK_MODULES([OPENSSL], [openssl])
PKG_CHECK_MODULES([ZLIB], [zlib])

AX_CHECK_BZLIB

PKG_CHECK_MODULES([LIBLZMA], [liblzma], [AC_DEFINE([HAVE_LIBLZMA], [1], [Define
to 1 if you have development C headers and libraries of LZMA])], [:])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_SUBST([_EMPTY], [])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
