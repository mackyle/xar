bin_PROGRAMS = xar
xar_SOURCES = xar.c
xar_LDADD = ../lib/libxar.la

man_MANS = xar.1

AM_CFLAGS = $(OPENSSL_CFLAGS) $(ZLIB_CFLAGS) $(LIBXML2_CFLAGS) $(LIBLZMA_CFLAGS)
AM_CPPFLAGS = -I../include -I$(srcdir)/.. -I$(srcdir)/../include -I..
