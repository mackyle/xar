CFLAGS = -g `xml2-config --cflags`
LDFLAGS = -lxar `xml2-config --libs` -lcrypto 
all: xardiff vitoc toc_extract
xardiff: xardiff.o
strextract: strextract.o
vitoc: vitoc.o
toc_extract: toc_extract.o

clean:
	rm -f xardiff xardiff.o
	rm -f vitoc vitoc.o
	rm -f toc_extract toc_extract.o
