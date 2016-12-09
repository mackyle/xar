#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Xar Tools], [2.0.0], [kuba@kaszycki.net.pl], [xar-tools])
AC_CONFIG_SRCDIR([strextract.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([../build-aux])
AC_CONFIG_MACRO_DIR([../m4])
AM_INIT_AUTOMAKE([foreign])
LT_INIT

# Checks for programs.
AC_PROG_CC
PKG_PROG_PKG_CONFIG

# Checks for libraries.
PKG_CHECK_MODULES([LIBXML2], [libxml-2.0 >= 2.6.11])
PKG_CHECK_MODULES([OPENSSL], [openssl])
PKG_CHECK_MODULES([ZLIB], [zlib])

AX_CHECK_BZLIB

PKG_CHECK_MODULES([LIBLZMA], [liblzma], [AC_DEFINE([HAVE_LIBLZMA], [1], [Define
to 1 if you have development C headers and libraries of LZMA])], [:])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_INT32_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strdup strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
