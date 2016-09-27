AC_PREREQ(2.59)
AC_INIT([Xar], [1.6.1], [xar-devel@googlegroups.com], [xar])
AC_CONFIG_SRCDIR([lib/util.c])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])

LT_INIT

dnl xar version variables.
AC_SUBST([XAR_NUMERIC_VERSION], [0x01060180])
AC_PROG_CC
AC_SYS_LARGEFILE
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_GREP
AC_PROG_SED

AC_HEADER_STDC
AC_CHECK_HEADERS([ext2fs/ext2_fs.h sys/statfs.h sys/xattr.h sys/param.h sys/extattr.h libutil.h sys/acl.h sys/statfs.h sys/statvfs.h sys/types.h sys/mount.h])
AC_CHECK_FUNCS([lgetxattr lsetxattr getxattr setxattr getattrlist setattrlist lchmod lchown chflags statvfs statfs strmode])
AC_CHECK_TYPE([acl_t])

AC_CHECK_MEMBER([struct statfs.f_fstypename])
AC_CHECK_MEMBER([struct statvfs.f_fstypename])
AC_CHECK_MEMBER([struct stat.st_flags])

AC_SEARCH_LIBS([acl_get_file], [acl])

AC_CHECK_FUNC([asprintf])

PKG_CHECK_MODULES([LIBXML2], [libxml-2.0 >= 2.6.11])
PKG_CHECK_MODULES([OPENSSL], [openssl])
PKG_CHECK_MODULES([ZLIB], [zlib])

AX_CHECK_BZLIB

PKG_CHECK_MODULES([LIBLZMA], [liblzma], [AC_DEFINE([HAVE_LIBLZMA], [1], [Define to 1 if you have development C headers and libraries of LZMA])], [:])

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 include/xar.h
                 lib/Makefile
                 m4/Makefile
                 src/Makefile
                 test/Makefile
                 xar.spec])
AC_OUTPUT
