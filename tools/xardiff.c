#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <libxml/tree.h>
#include <libxml/hash.h>
#include <xar/xar.h>
#include <xar/toc.h>
#include <xar/archive.h>

int Noatime = 0;

void xar_compare(xar_toc_t one, xar_toc_t two) {
    mode_t mode1 = xar_toc_get_mode(one);
    uid_t nuid1 = xar_toc_get_uid(one);
    gid_t ngid1 = xar_toc_get_gid(one);
    char* user1 = xar_toc_get_user(one);
    char* group1 = xar_toc_get_group(one);
    char* type1 = xar_toc_get_type(one);
    char* atime1 = xar_toc_get_atime(one);
    char* mtime1 = xar_toc_get_mtime(one);
    char* ctime1 = xar_toc_get_ctime(one);
    off_t size1 = xar_toc_get_size(one);

    mode_t mode2 = xar_toc_get_mode(two);
    uid_t nuid2 = xar_toc_get_uid(two);
    gid_t ngid2 = xar_toc_get_gid(two);
    char* user2 = xar_toc_get_user(two);
    char* group2 = xar_toc_get_group(two);
    char* type2 = xar_toc_get_type(two);
    char* atime2 = xar_toc_get_atime(two);
    char* mtime2 = xar_toc_get_mtime(two);
    char* ctime2 = xar_toc_get_ctime(two);
    off_t size2 = xar_toc_get_size(two);

    int comp;
    char* name = xar_toc_get_path(one);

    comp = size1 - size2;
    if( comp != 0 )
        printf("%s has different sizes\n", name);

    comp = strcmp(type1, type2);
    if( comp != 0 )
        printf("%s has different types\n", name);

    comp = mode1 - mode2;
    if( comp != 0 )
        printf("%s has different modes\n", name);

    comp = strcmp(user1, user2);
    if( comp != 0 )
        printf("%s has different users\n", name);

    comp = strcmp(group1, group2);
    if( comp != 0 )
        printf("%s has different groups\n", name);

    comp = nuid1 - nuid2;
    if( comp != 0 )
        printf("%s has different uids\n", name);

    comp = ngid1 - ngid2;
    if( comp != 0 )
        printf("%s has different gids\n", name);

    comp = strcmp(atime1, atime2);
    if( !Noatime && comp != 0 )
        printf("%s has different atimes\n", name);

    comp = strcmp(mtime1, mtime2);
    if( comp != 0 )
        printf("%s has different mtimes\n", name);

    comp = strcmp(ctime1, ctime2);
    if( comp != 0 )
        printf("%s has different ctimes\n", name);
    free(ctime1);
    free(ctime2);
    free(mtime1);
    free(mtime2);
    free(atime1);
    free(atime2);
    free(user1);
    free(user2);
    free(group1);
    free(group2);
    free(type1);
    free(type2);
    free(name);

}

static xar_archive_t xar_create(char *file1) {
	xar_archive_t xat;
	int fd;
	char tmpnam[] = "/tmp/xardiff.XXXXXX";

	fd = mkstemp(tmpnam);
	if( fd < 0 ) return NULL;

	xat = xar_open_with_fd(fd, 1);
	if( xat == NULL ) return NULL;

	xar_opt_set_toconly(xat, 1);

	xar_archive_recurse_add(xat, file1);

	return xat;
}

void xar_diff(char* file1, char* file2) {
    xar_archive_t one, two;
    xar_toc_t t1, t2;
    xmlHashTablePtr hash = xmlHashCreate(0);

    one = xar_open_with_path(file1, 0);
    if( one==NULL ) {
        one = xar_create(file1);
        if( one == NULL ) {
            printf("Couldn't open path(%s): %s\n", file1, strerror(errno));
            return;
        }
    }
    two = xar_open_with_path(file2, 0);
    if( two==NULL ) {
        two = xar_create(file2);
        if( two == NULL ) {
            printf("Couldn't open path(%s): %s\n", file2, strerror(errno));
            return;
        }
    }

    for(t1 = xar_toc_get_first_element(one); t1; t1=xar_toc_get_next_element(t1)) {
        char* path = xar_toc_get_path(t1);
        xar_toc_t n = xar_toc_find_file(two, path);
        xmlHashAddEntry(hash, path, t1);
        if( !n ) {
            printf("%s not found in %s\n", path, file2);
            free(path);
            xar_toc_free(n);
            continue;
        } else {
           xar_compare(t1, n);
        }
        free(path);
        xar_toc_free(n);
    }

    for(t1 = xar_toc_get_first_element(two); t1; t2=t1,t1=xar_toc_get_next_element(t1),free(t2)) {
        char* path = xar_toc_get_path(t1);
        xar_toc_t n = xmlHashLookup(hash, path);
        if( !n ) {
            printf("%s not found in %s\n", path, file1);
            free(path);
            xar_toc_free(n);
            continue;
        }
        free(path);
        xar_toc_free(n);

    }
}

void print_usage(char *name) {
    printf("Usage: %s [-a] file1.xar file2.xar\n", name);
    printf("           -a\t\t Do not print differences about atime\n");
}

int main(int argc, char* argv[]) {
    int c;

    while((c = getopt(argc, argv, "ah")) != -1) {
        switch(c) {
        case 'a': Noatime = 1;
                  break;
        case 'h':
        default:
            print_usage(argv[0]);
            exit(1);
        }
    }
    if( argc - optind != 2 ) {
        print_usage(argv[0]);
        exit(1);
    }

    xar_diff(argv[optind], argv[optind+1]);
    exit(0);
}
