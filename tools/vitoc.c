#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <libxml/tree.h>
#include <xar/xar.h>
#include <xar/util.h>

int main(int argc, char* argv[]) {
	struct xar_header h;
	int fd, tmpfd, newfd;
	char tmpfile[] = "/tmp/tmptoc.XXXXXX";
	char *dname, *tmpnam, *newfile;
	ssize_t red = 0;
	char buf[128000];
	pid_t pid;
	struct stat sb;

	tmpnam = strdup(argv[1]);
	dname = dirname(tmpnam);
	asprintf(&newfile, "%s/tmpxar.XXXXXX", dname);
	free(tmpnam);

	fd = open(argv[1], O_RDONLY);
	if( fd < 0 ) {
		perror("open");
		exit(1);
	}

	if( read(fd, &h, sizeof(h)) < 0 ) {
		perror("read");
		exit(1);
	}

	//h.magic = ntohl(h.magic);
	//h.size = ntohs(h.size);
	//h.version = ntohs(h.version);
	h.toc_length = xar_ntoh64(h.toc_length);

	tmpfd = mkstemp(tmpfile);
	if( tmpfd < 0 ) {
		perror("mkstemp");
		exit(2);
	}

	while( red < h.toc_length ) {
		int sz = sizeof(buf);
		int ret;

		if( (h.toc_length - red) < sizeof(buf) ) 
			sz = h.toc_length - red;
		ret = read(fd, buf, sz);
		if( ret < 0 ) {
			perror("read toc");
			exit(3);
		}

		write(tmpfd, buf, ret);
		red+=ret;
	}

	close(tmpfd);

	pid = fork();
	if( pid == 0 ) {
		execlp("/usr/bin/vi", "vi", tmpfile, NULL);
		exit(5);
	}
	waitpid(pid, NULL, 0);

	tmpfd = open(tmpfile, O_RDONLY);
	newfd = mkstemp(newfile);

	stat(tmpfile, &sb);
	h.toc_length = (uint64_t)sb.st_size;

	write(newfd, &h, sizeof(h));

	red = 0;
	while( red < h.toc_length ) {
		int sz = sizeof(buf);
		int ret;

		if( (h.toc_length - red) < sizeof(buf) ) 
			sz = h.toc_length - red;
		ret = read(tmpfd, buf, sz);
		if( ret < 0 ) {
			perror("read toc");
			exit(3);
		}

		write(newfd, buf, ret);
		red+=ret;
	}

	while( (red = read(fd, buf, sizeof(buf))) > 0) {
		write(newfd, buf, red);
	}

	close(fd);
	close(tmpfd);
	close(newfd);

	if( rename(newfile, argv[1]) < 0 )
		perror("rename");
	free(newfile);
	unlink(tmpfile);
}
