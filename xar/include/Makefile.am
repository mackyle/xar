nobase_include_HEADERS = \
                         xar/appledouble.h \
                         xar/archive.h \
                         xar/arcmod.h \
                         xar/asprintf.h \
                         xar/b64.h \
                         xar/bzxar.h \
                         xar/darwinattr.h \
                         xar/data.h \
                         xar/ea.h \
                         xar/ext2.h \
                         xar/fbsdattr.h \
                         xar/filetree.h \
                         xar/hash.h \
                         xar/io.h \
                         xar/linuxattr.h \
                         xar/lzmaxar.h \
                         xar/macho.h \
                         xar/script.h \
                         xar/signature.h \
                         xar/stat.h \
                         xar/strmode.h \
                         xar/subdoc.h \
                         xar/util.h \
                         xar/xar.h \
                         xar/zxar.h

DISTCLEANFILES = config.h

nobase_nodist_include_HEADERS = xar/config.h

EXTRA_DIST = config.h.in xar/config.h.in

xar/config.h: $(srcdir)/xar/config.h.in
	$(AM_V_GEN)(cd $(top_builddir) && $(SHELL) ./config.status \
        include/xar/config.h || exit $$?)
