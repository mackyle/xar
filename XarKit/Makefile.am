if HAVE_GNUSTEP
lib_LTLIBRARIES = libXarKit.la
libXarKit_la_SOURCES = \
                       XarArchive.m \
                       XarBase.m \
                       XarEnumerator.m \
                       XarFile.m \
                       XarSubdoc.m

libXarKit_la_LIBADD = \
                      ../xar/lib/libxar.la \
                      $(ZLIB_LIBS) \
                      $(LIBLZMA_LIBS) \
                      $(OPENSSL_LIBS) \
                      -lbz2

xincludedir = $(includedir)/XarKit
xinclude_HEADERS = \
                   XarArchive.h \
                   XarBase.h \
                   XarEnumerator.h \
                   XarFile.h \
                   XarKit.h \
                   XarSubdoc.h

mostlyclean-hook:
	rm -rf XarKit.framework

XarKit.framework: libXarKit.la $(srcdir)/Info.plist $(ac_aux_dir)/mkframework \
    $(xinclude_HEADERS)
	$(AM_V_GEN)MKDIR_P='$(MKDIR_P)' LN_S='$(LN_S)' \
        $(SHELL) $(ac_aux_dir)/mkframework $(SHREXT) $(srcdir)/Info.plist \
        XarKit "$(PACKAGE_VERSION)" "$(libdir)" "$(xinclude_HEADERS)" \
        "$(xincludedir)"

install-exec-hook: XarKit.framework
	$(INSTALL) -d $(libdir)/GNUstep/Frameworks && \
        cp -r $< $(libdir)/GNUstep/Frameworks
	$(INSTALL) -d $(exec_prefix)/Library/Frameworks && \
        cp -r $< $(exec_prefix)/Library/Frameworks

uninstall-hook:
	rm -rf $(libdir)/GNUstep/Frameworks/XarKit \
        $(exec_prefix)/Library/Frameworks/XarKit

all-am: XarKit.framework
mostlyclean-local: mostlyclean-hook
	@:

AM_OBJCFLAGS = $(shell $(GNUSTEP_CONFIG) --objc-flags)

EXTRA_DIST = Info.plist

SHREXT = $(shell $(LIBTOOL) --config > __temp && . __temp && rm -f __temp && echo $$shrext_cmds)
endif

ACLOCAL_AMFLAGS = -I ../m4

AM_LDFLAGS = \
             -L$(BZLIB_HOME)/lib \
             "$(shell $(GNUSTEP_CONFIG) --base-libs)"

AM_CPPFLAGS = \
              $(LIBXML2_CFLAGS) \
              -I../xar/include \
              -I$(srcdir)/../xar.include \
              $(ZLIB_CFLAGS) \
              $(LIBLZMA_CFLAGS) \
              -I$(BZLIB_HOME)/include \
              $(OPENSSL_CFLAGS)
