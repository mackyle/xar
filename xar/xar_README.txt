To build:

./autogen.sh --noconfigure
PATH="`getconf PATH`:/usr/local/bin" CFLAGS='-Wall -Os' ./configure --without-lzma

If make is GNU make (try make --version to find out) use:

make
sudo make install

If gmake is GNU make (try gmake --version to find out) use:

gmake
sudo gmake install

NOTE: CLI signature patch based heavily upon:

http://opensource.apple.com/source/xar/xar-36.1/xarsig/xar-sig.c
