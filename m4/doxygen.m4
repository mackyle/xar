AC_DEFUN([AX_PROG_DOXYGEN], [
  AC_ARG_ENABLE([doxygen], [AS_HELP_STRING([--disable-doxygen],
    [Disable Doxygen generated documentation])], [enable_doxygen=$enableval],
    [enable_doxygen=yes])
  AC_ARG_VAR([DOXYGEN], [Path to the Doxygen executable])
  AS_IF([test -z "$DOXYGEN"], [AC_CHECK_PROGS([DOXYGEN], [doxygen])])
  AM_CONDITIONAL([RUN_DOXYGEN],
                 [test "$enable_doxygen" = "yes" && test -n "$DOXYGEN"])
  AS_UNSET([enable_doxygen])
  AC_CONFIG_FILES([doc/Doxyfile])
])
