bin_PROGRAMS = xar-strextract xar-toc_extract

xar_strextract_SOURCES = strextract.c
xar_toc_extract_SOURCES = toc_extract.c

LDADD = \
        ../xar/lib/libxar.la \
        $(ZLIB_LIBS) \
        $(LIBLZMA_LIBS) \
        $(OPENSSL_LIBS) \
        -lbz2

AM_LDFLAGS = \
             -L$(BZLIB_HOME)/lib

AM_CPPFLAGS = \
              $(LIBXML2_CFLAGS) \
              -I../xar/include \
              -I$(srcdir)/../xar.include \
              $(ZLIB_CFLAGS) \
              $(LIBLZMA_CFLAGS) \
              -I$(BZLIB_HOME)/include \
              $(OPENSSL_CFLAGS)

ACLOCAL_AMFLAGS = -I ../m4
