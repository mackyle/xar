#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <xar/xar.h>
#include <sys/fcntl.h>

static int32_t err_callback(int32_t sev, int32_t err, xar_errctx_t ctx, void *usrctx) {
	fprintf(stderr, "err callback\n");
}

int main(int argc, char *argv[]) {
	xar_t x;
	xar_iter_t i;
	xar_file_t f;
	xar_stream s;
	char buffer[8192];

	x = xar_open(argv[1], READ);
	if( !x ) {
		fprintf(stderr, "Error opening archive\n");
		exit(1);
	}

	xar_register_errhandler(x, err_callback, NULL);

	i = xar_iter_new();
	if( !i ) {
		fprintf(stderr, "Error creating xar iterator\n");
		exit(1);
	}

	for(f = xar_file_first(x, i); f; f = xar_file_next(i)) {
		char *path;
		const char *type;
		int32_t ret;
		int fd;

		path = xar_get_path(f);

		xar_prop_get(f, "type", &type);
		if( !type ) {
			fprintf(stderr, "File has no type %s\n", path);
			free(path);
			continue;
		}
		if( strcmp(type, "file") != 0 ) {
			fprintf(stderr, "Skipping %s\n", path);
			free(path);
			continue;
		}
	
		fprintf(stderr, "Extracting %s\n", path);
		if( xar_extract_tostream_init(x, f, &s) != XAR_STREAM_OK ) {
			fprintf(stderr, "Error initializing stream %s\n", path);
			free(path);
			continue;
		}
		
		fd = open(path, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
		if( !fd ) {
			fprintf(stderr, "Error opening output file %s\n", path);
			free(path);
			continue;
		}
		s.avail_out = sizeof(buffer);
		s.next_out = buffer;

		while( (ret = xar_extract_tostream(&s)) != XAR_STREAM_END ) {
			if( ret == XAR_STREAM_ERR ) {
				fprintf(stderr, "Error extracting stream %s\n", path);
				exit(2);
			}

			write(fd, buffer, sizeof(buffer)-s.avail_out);

			s.avail_out = sizeof(buffer);
			s.next_out = buffer;
		}

		if( xar_extract_tostream_end(&s) != XAR_STREAM_OK ) {
			fprintf(stderr, "Error ending stream %s\n", path);
		}

		close(fd);
		free(path);
	}
}
