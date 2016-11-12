#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([XarKit], [1.6.1], [kuba@kaszycki.net.pl], [xarkit])
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
AC_CHECK_PROGS([GNUSTEP_CONFIG], [gnustep-config], [--NONE--])
AM_CONDITIONAL([HAVE_GNUSTEP], [test ! "$GNUSTEP_CONFIG" = '--NONE--'])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
