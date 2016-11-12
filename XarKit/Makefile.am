lib_LTLIBRARIES = libXarKit.la
libXarKit_la_SOURCES = \
                       XarArchive.m \
                       XarEnumerator.m \
                       XarFile.m

libXarKit_la_LIBADD = \
                      ../xar/lib/libxar.la

AM_LDFLAGS = "$(shell $(GNUSTEP_CONFIG) --base-libs)"

xincludedir = $(includedir)/XarKit
xinclude_HEADERS = \
                   XarArchive.h \
                   XarEnumerator.h \
                   XarFile.h \
                   XarKit.h

mostlyclean-hook:
	rm -rf XarKit.framework

XarKit.framework: libXarKit.la $(srcdir)/Info.plist $(ac_aux_dir)/mkframework \
    $(xinclude_HEADERS)
	$(AM_V_GEN)MKDIR_P='$(MKDIR_P)' LN_S='$(LN_S)' \
        $(SHELL) $(ac_aux_dir)/mkframework $(SHREXT) $(srcdir)/Info.plist \
        XarKit "$(PACKAGE_VERSION)" "$(libdir)" "$(xinclude_HEADERS)" \
        "$(xincludedir)"

install-exec-hook: XarKit.framework
	cp -r $< $(libdir)/GNUstep/Frameworks
	cp -r $< $(exec_prefix)/Library/Frameworks

all-am: XarKit.framework
mostlyclean-local: mostlyclean-hook
	@:

AM_CPPFLAGS = \
              -I../xar/include \
              -I$(srcdir)/../xar/include
AM_OBJCFLAGS = $(shell $(GNUSTEP_CONFIG) --objc-flags)

EXTRA_DIST = Info.plist

SHREXT = $(shell $(LIBTOOL) --config > __temp && . __temp && rm -f __temp && echo $$shrext_cmds)

ACLOCAL_AMFLAGS = -I ../m4
