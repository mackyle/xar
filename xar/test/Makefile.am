EXTRA_DIST = attr checksums compression data data.xsl demo.xar functions \
             hardlink heap heap1.xsl validate.c

TESTS = ./attr ./buffer$(EXEEXT) ./checksums ./compression ./data ./hardlink \
        ./heap ./validate$(EXEEXT)
TESTS_ENVIRONMENT = GREP=$(GREP) SED=$(SED) AWK=$(AWK) srcdir=$(abs_srcdir) \
                    builddir=$(abs_builddir) \
                    XAR=$(abs_builddir)/../src/xar$(EXEEXT) \
                    XARFILE=$(abs_srcdir)/demo.xar

check_PROGRAMS = buffer validate
buffer_SOURCES = buffer.c
validate_SOURCES = validate.c
buffer_LDADD = ../lib/libxar.la
validate_LDADD = ../lib/libxar.la $(OPENSSL_LIBS)

AM_CFLAGS = $(OPENSSL_CFLAGS)
AM_CPPFLAGS = -I../include

CLEANFILES = bin.xar bin2.xar h.xar i.xar j.xar
