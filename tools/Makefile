CFLAGS = -g `xml2-config --cflags`
LDFLAGS = -lxar `xml2-config --libs` -lcrypto -lbz2 -lacl
all: xardiff vitoc toc_extract
xardiff: xardiff.o
vitoc: vitoc.o
toc_extract: toc_extract.o

clean:
	rm -f xardiff xardiff.o
	rm -f vitoc vitoc.o
	rm -f toc_extract toc_extract.o
