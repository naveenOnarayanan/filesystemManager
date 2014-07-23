#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ece454_fs.h"


int main(int argc, char *argv[]) {
    if(argc < 4) {
	fprintf(stderr, "usage: %s <srv-ip/name> <srv-port> <local dir name>\n", argv[0]);
	exit(1);
    }

    char *dirname = argv[3];
    printf("fsMount(): %d\n", fsMount(argv[1], atoi(argv[2]), dirname));

    char * fname = "/wtf/file_1.txt";
    int ff = fsOpen(fname, 1);
    printf("The ff: %d\n", ff);
    if(ff < 0) {
	perror("fsOpen(write)"); exit(1);
    }

    sleep(10);

    if(fsClose(ff) < 0) {
	perror("fsClose"); exit(1);
    }

    if(fsUnmount(dirname) < 0) {
	perror("fsUnmount"); exit(1);
    }

    return 0;
}

