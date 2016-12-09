#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([XarKit], [2.0.0], [kuba@kaszycki.net.pl], [xarkit])
AC_CONFIG_SRCDIR([XarEnumerator.m])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([../build-aux])
AC_CONFIG_MACRO_DIR([../m4])
AM_INIT_AUTOMAKE([foreign])
LT_INIT

AC_REQUIRE_AUX_FILE([mkframework])
AC_SUBST([ac_aux_dir])

# Checks for programs.
AC_PROG_CC
AC_PROG_OBJC
PKG_PROG_PKG_CONFIG
AC_CHECK_PROGS([GNUSTEP_CONFIG], [gnustep-config], [--NONE--])
AM_CONDITIONAL([HAVE_GNUSTEP], [test ! "$GNUSTEP_CONFIG" = '--NONE--'])

# Checks for libraries.
PKG_CHECK_MODULES([LIBXML2], [libxml-2.0 >= 2.6.11])
PKG_CHECK_MODULES([OPENSSL], [openssl])
PKG_CHECK_MODULES([ZLIB], [zlib])

AX_CHECK_BZLIB

PKG_CHECK_MODULES([LIBLZMA], [liblzma], [AC_DEFINE([HAVE_LIBLZMA], [1], [Define
to 1 if you have development C headers and libraries of LZMA])], [:])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
