if RUN_DOXYGEN
all-am: doxygen-all-am
doxygen-all-am: Doxyfile
	$(AM_V_GEN)$(DOXYGEN) Doxyfile
doxygen-clean-am:
	rm -rf html latex
doxygen-install-am: html
	rm -rf $(docdir)
	cp -rfT html $(docdir)
doxygen-uninstall-am:
	rm -rf $(docdir)
html latex: doxygen-all-am
	@:
endif
