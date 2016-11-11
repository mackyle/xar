EXTRA_DIST = xarfile.pyx setup.py test.sh

_EMPTY = 

noinst_DATA = stamp-all

stamp-all: $(srcdir)/setup.py $(srcdir)/xarfile.pyx
	CPPFLAGS='$(CPPFLAGS) $(AM_CPPFLAGS)' LDFLAGS='$(LDFLAGS) $(AM_LDFLAGS)' \
        $(PYTHON) $(srcdir)/setup.py build; \
        touch $@

clean-hook:
	rm -f stamp-all; \
	CPPFLAGS='$(CPPFLAGS) $(AM_CPPFLAGS)' LDFLAGS='$(LDFLAGS) $(AM_LDFLAGS)' \
	$(PYTHON) $(srcdir)/setup.py clean

install-exec-hook:
	CPPFLAGS='$(CPPFLAGS) $(AM_CPPFLAGS)' LDFLAGS='$(LDFLAGS) $(AM_LDFLAGS)' \
	$(PYTHON) $(srcdir)/setup.py install

check-am:
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
