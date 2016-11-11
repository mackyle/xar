#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Xar-Python], [1.0], [kuba@kaszycki.net.pl], [xar-python])
AC_CONFIG_SRCDIR([xarfile.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([../build-aux])
AC_CONFIG_MACRO_DIR([../m4])
AM_INIT_AUTOMAKE([foreign])
LT_INIT

AM_MAINTAINER_MODE([enable])

# Checks for programs.
AC_PROG_CC
AX_PYTHON([> '2.7' and ver < '3'])
AX_PYTHON_DEVEL([> '2.7' and ver < '3'])
AC_CHECK_PROGS([PYREXC], [pyrexc2.7 pyrexc2 pyrexc])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_SUBST([_EMPTY], [])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
