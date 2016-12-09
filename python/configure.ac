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
HAVE_PYTHON=yes
AX_PYTHON([> '2.7' and ver < '3'])
AS_IF([test "$ax_python_bin" = no], [HAVE_PYTHON=])
AX_PYTHON_DEVEL([> '2.7' and ver < '3'])
AC_CHECK_PROGS([PYREXC], [pyrexc2.7 pyrexc2 pyrexc], [--NONE--])
AS_IF([test "$PYREXC" = "--NONE--"], [HAVE_PYTHON=])
AM_CONDITIONAL([HAVE_PYTHON], [test -n "$HAVE_PYTHON"])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_SUBST([_EMPTY], [])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
