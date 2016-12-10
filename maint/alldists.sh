#!/bin/sh

PKGVER="`make -f Makefile --eval 'a:
	@echo $(PACKAGE_VERSION)' a`"

# This script preassumes that build = host.

make dist-bzip2
make dist-gzip
make dist-lzip
make dist-shar
make dist-tarZ
make dist-xz
make dist-zip
make distdir
xar/src/xar -cvf xar-${PKGVER}.xar xar-${PKGVER}