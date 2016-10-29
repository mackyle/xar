lib_LTLIBRARIES = libxar.la

libxar_la_SOURCES = archive.c arcmod.c b64.c bzxar.c darwinattr.c data.c ea.c err.c ext2.c fbsdattr.c filetree.c hash.c io.c linuxattr.c lzmaxar.c macho.c script.c signature.c stat.c subdoc.c util.c zxar.c
libxar_la_LIBADD = $(LIBXML2_LIBS) $(ZLIB_LIBS) $(LIBLZMA_LIBS) $(OPENSSL_LIBS)
libxar_la_LDFLAGS = -no-undefined -version-info 0:1:0

AM_CFLAGS = $(LIBXML2_CFLAGS) $(ZLIB_CFLAGS) $(LIBLZMA_CFLAGS) $(OPENSSL_CFLAGS)
AM_CPPFLAGS = -I../include
