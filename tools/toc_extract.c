#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <libxml/tree.h>
#include <xar/xar.h>
#include <xar/util.h>
#include <zlib.h>

#define BUFFER_SIZE 8192

int main(int argc, char* argv[]) {
    char *bufcomp, *bufuncomp;
    struct xar_header h;
    int fd, len;
    ssize_t red;
    z_stream zs;

    fd = open(argv[1], O_RDONLY);
    if( fd < 0 ) {
        perror("open");
        exit(1);
    }

    if( read(fd, &h, 8) < 0 ) {
        fprintf(stderr, "Error reading magic, size, version from header\n");
        exit(1);
    }

    h.magic = ntohl(h.magic);
    h.size = ntohs(h.size);
    h.version = ntohs(h.version);

    len = sizeof(h.magic)+sizeof(h.size)+sizeof(h.version);
    if( read(fd, &h.toc_length_compressed, h.size-len) < 0 ) {
        perror("read");
        exit(1);
    }

    h.toc_length_compressed = xar_ntoh64(h.toc_length_compressed);
    h.toc_length_uncompressed = xar_ntoh64(h.toc_length_uncompressed);
    h.cksum_alg = ntohl(h.cksum_alg);

    bufcomp = malloc(BUFFER_SIZE);
    bufuncomp = malloc(BUFFER_SIZE);
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    inflateInit(&zs);
    red = 0;
    while( red < h.toc_length_compressed ) {
        int sz = BUFFER_SIZE;
        int ret;

        if( (h.toc_length_compressed - red) < BUFFER_SIZE ) 
            sz = h.toc_length_compressed - red;

        ret = read(fd, bufcomp, sz);
        if( ret < 0 ) {
            perror("read toc");
            exit(1);
        }
        red+=ret;
        zs.next_in = bufcomp;
        zs.avail_in = ret;
        while( zs.avail_in != 0 ) {
            zs.next_out = bufuncomp;
            zs.avail_out = BUFFER_SIZE;
            inflate(&zs, Z_SYNC_FLUSH);
            len = BUFFER_SIZE-zs.avail_out;
            fwrite(bufuncomp, 1, len, stdout);
	}
    }
    close(fd);
}
