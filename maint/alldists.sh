#!/bin/sh

PKGVER="`make -f Makefile --eval 'echo $(PACKAGE_VERSION)'`"

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
