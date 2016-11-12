EXTRA_DIST = xarfile.pyx setup.py test.sh

_EMPTY = 

noinst_DATA = stamp-all

PASS_ENV = \
           CPPFLAGS='$(CPPFLAGS) $(AM_CPPFLAGS) $(INCLUDES)' \
           LDFLAGS='$(LDFLAGS) $(AM_LDFLAGS)' \
           CFLAGS='$(CFLAGS) $(AM_CFLAGS)' \
           LIBS='$(LIBS) $(LDADD)' \
           CC='$(CC)' \
           CCLD='$(CC)' \
           LD='$(CC)' \
           PYTHON='$(PYTHON)'

stamp-all: $(srcdir)/setup.py $(srcdir)/xarfile.pyx
	$(PASS_ENV) \
        $(PYTHON) $(srcdir)/setup.py build; \
        touch $@

mostlyclean-am:
	rm -f stamp-all; \
	$(PASS_ENV) \
	$(PYTHON) $(srcdir)/setup.py clean -a

install-exec-hook:
	$(PASS_ENV) \
	$(PYTHON) $(srcdir)/setup.py install

check-am:
	$(PASS_ENV) \
	$(PYTHON) $(srcdir)/setup.py check

@_EMPTY@ifneq (-$(srcdir)-,-$(builddir)-)
xarfile.pyx: $(srcdir)/xarfile.pyx
	$(AM_V_GEN)$(LN_S) $< $@
setup.py: $(srcdir)/setup.py
	$(AM_V_GEN)$(LN_S) $< $@

DISTCLEANFILES = xarfile.pyx setup.py
@_EMPTY@endif

ACLOCAL_AMFLAGS = -I ../m4

AM_CPPFLAGS = -I../xar/include -I$(srcdir)/../xar/include
AM_LDFLAGS = -L../xar/lib -L../xar/lib/.libs
