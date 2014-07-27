#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ece454_fs.h"

void printBuf(char *buf, int size) {
    /* Should match the output from od -x */
    int i;
    for(i = 0; i < size; ) {
    if(i%16 == 0) {
        printf("%08o ", i);
    }

    int j;
    for(j = 0; j < 16;) {
        int k;
        for(k = 0; k < 2; k++) {
        if(i+j+(1-k) < size) {
            printf("%02x", (unsigned char)(buf[i+j+(1-k)]));
        }
        }

        printf(" ");
        j += k;
    }

    printf("\n");
    i += j;
    }
}

int main(int argc, char *argv[]) {
    if(argc < 5) {
    fprintf(stderr, "usage: %s <srv-ip/name> <srv-port> <local dir name> <mode>\n", argv[0]);
    exit(1);
    }
    int mode = atoi(argv[4]);
    if (mode == 1)
    {
        char *dirname = argv[3];
        printf("fsMount(): %d\n", fsMount(argv[1], atoi(argv[2]), dirname));
        FSDIR *fd = fsOpenDir(dirname);
        if (fd == NULL) {
            perror("fsOpenDir"); exit(1);
        }

        struct fsDirent *fdent = NULL;
        for (fdent = fsReadDir(fd); fdent != NULL; fdent = fsReadDir(fd)) {
            printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
        }

        if (errno != 0) {
            perror("fsReadDir");
        }

        printf("fsCloseDir(): %d\n", fsCloseDir(fd));

        int ff = open("/dev/urandom", 0);
        if (ff < 0) {
            perror("open(/dev/urandom)"); exit(1);
        }
        else printf("open(): %d\n", ff);

        char fname[256];
        sprintf(fname, "%s/", dirname);
        if (read(ff, (void *)(fname + strlen(dirname) + 1), 10) < 0) {
            perror("read(/dev/urandom)"); exit(1);
        }

        int i;
        for (i = 0; i < 10; i++) {
            //printf("%d\n", ((unsigned char)(fname[i]))%26);
            fname[i + strlen(dirname) + 1] = ((unsigned char)(fname[i + strlen(dirname) + 1])) % 26 + 'a';
        }
        fname[10 + strlen(dirname) + 1] = (char)0;
        printf("Filename to write: %s\n", (char *)fname);

        char buf[256];
        if (read(ff, (void *)buf, 256) < 0) {
            perror("read(2)"); exit(1);
        }

        printBuf(buf, 256);

        printf("close(): %d\n", close(ff));

        ff = fsOpen(fname, 1);
        if (ff < 0) {
            perror("fsOpen(write)"); exit(1);
        }
        int fsWriteResult = fsWrite(ff, buf, 256);
        if (fsWriteResult < 256) {
            fprintf(stderr, "fsWrite() wrote fewer than 256 -- wrote:%d\n", fsWriteResult);
        }

        if (fsClose(ff) < 0) {
            perror("fsClose"); exit(1);
        }

        char readbuf[256];
        if ((ff = fsOpen(fname, 0)) < 0) {
            perror("fsOpen(read)"); exit(1);
        }

        int readcount = -1;

        if ((readcount = fsRead(ff, readbuf, 256)) < 256) {
            fprintf(stderr, "fsRead() read fewer than 256\n");
        }

        if (memcmp(readbuf, buf, readcount)) {
            fprintf(stderr, "return buf from fsRead() differs from data written!\n");
        }
        else {
            printf("fsread(): return buf identical to data written upto %d bytes.\n", readcount);
        }

        if (fsClose(ff) < 0) {
            perror("fsClose"); exit(1);
        }

        printf("fsRemove(%s): %d\n", fname, fsRemove(fname));

        if (fsUnmount(dirname) < 0) {
            perror("fsUnmount"); exit(1);
        }
    }
    else if (mode == 2)
    {
        char *dirname = argv[3];
        FSDIR *fd;
        printf("fsMount(): %d\n", fsMount(argv[1], atoi(argv[2]), dirname));
        fd = fsOpenDir(dirname);
        if (fd == NULL) {
            perror("fsOpenDir"); exit(1);
        }

        struct fsDirent *fdent = NULL;
        for (fdent = fsReadDir(fd); fdent != NULL; fdent = fsReadDir(fd)) {
            printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
        }

        if (errno != 0) {
            perror("fsReadDir");
        }

        printf("fsCloseDir(): %d\n", fsCloseDir(fd));

        int ff = open("/dev/urandom", 0);
        if (ff < 0) {
            perror("open(/dev/urandom)"); exit(1);
        }
        else printf("open(): %d\n", ff);

        char fname[256];
        sprintf(fname, "%s/%s", dirname, "gordonjosh");
        /*
        if (read(ff, (void *)(fname + strlen(dirname) + 1), 10) < 0) {
        perror("read(/dev/urandom)"); exit(1);
        }
        */

        int i;
            for (i = 0; i < 10; i++) {
                //printf("%d\n", ((unsigned char)(fname[i]))%26);
                fname[i + strlen(dirname) + 1] = ((unsigned char)(fname[i + strlen(dirname) + 1])) % 26 + 'a';
            }
            fname[10 + strlen(dirname) + 1] = (char)0;
        printf("Filename to write: %s\n", (char *)fname);

        char buf[256];
        if (read(ff, (void *)buf, 256) < 0) {
            perror("read(2)"); exit(1);
        }

        printBuf(buf, 256);

        printf("close(): %d\n", close(ff));
        char input[256];
        char readbuf[256];
        while (strcmp(input, "exit") != 0)
        {
            scanf("%79s", input); 
            if (strcmp(input, "openwrite") == 0)
            {
                ff = fsOpen(fname, 1);
                if (ff < 0) {
                    perror("fsOpen(write)"); exit(1);
                }
            }
            else if (strcmp(input, "write") == 0)
            {
                if (fsWrite(ff, buf, 256) < 256) {
                    fprintf(stderr, "fsWrite() wrote fewer than 256\n");
                }
            }
            else if (strcmp(input, "close") == 0)
            {
                if (fsClose(ff) < 0) {
                    perror("fsClose"); exit(1);
                }
            }
            else if (strcmp(input, "openread") == 0)
            {

                if ((ff = fsOpen(fname, 0)) < 0) {
                    perror("fsOpen(read)"); exit(1);
                }
            }
            else if (strcmp(input, "read") == 0)
            {
                int readcount = -1;

                if ((readcount = fsRead(ff, readbuf, 256)) < 256) {
                    fprintf(stderr, "fsRead() read fewer than 256\n");
                }

                if (memcmp(readbuf, buf, readcount)) {
                    fprintf(stderr, "return buf from fsRead() differs from data written!\n");
                }
                else {
                    printf("fsread(): return buf identical to data written upto %d bytes.\n", readcount);
                }
            }
            else if (strcmp(input, "remove") == 0)
            {
                printf("fsRemove(%s): %d\n", fname, fsRemove(fname));
            }
            else if (strcmp(input, "unmount") == 0)
            {

                if (fsUnmount(dirname) < 0) {
                    perror("fsUnmount"); exit(1);
                }
            }

        }
    }
    else if (mode == 3)
    {
        //Validate basic directory functionality
        
        //Mount
        char *dirname = argv[3];
        printf("fsMount(): %d\n", fsMount(argv[1], atoi(argv[2]), dirname));
        
        char *subdirectory = malloc(strlen(dirname) + 1 + 6 + 1);
        strcpy(subdirectory, dirname);
        strcat(subdirectory, "/");
        strcat(subdirectory, "gordon");
        strcat(subdirectory, "\0");
        printf("subdirectory path: %s", subdirectory);
        
        char *subsubdirectory = malloc(strlen(subdirectory) + 1 + 6 + 1);
        strcpy(subsubdirectory, subdirectory);
        strcat(subsubdirectory, "/");
        strcat(subsubdirectory, "josh");
        strcat(subsubdirectory, "\0");
        printf("subsubdirectory path: %s", subsubdirectory);
        
        FSDIR *fd1;
        FSDIR *fd2;
        struct fsDirent *fdent1;
        struct fsDirent *fdent2;
        
        
        //Open main directory
        printf("Open main directory\n");
        fd1 = fsOpenDir(dirname);
        if (fd1 == NULL) {
            perror("fsOpenDir"); exit(1);
        }

        //Read main directory
        printf("Read main directory\n");
        fdent1 = NULL;
        for (fdent1 = fsReadDir(fd1); fdent1 != NULL; fdent1 = fsReadDir(fd1)) {
            printf("\t %s, %d\n", fdent1->entName, (int)(fdent1->entType));
        }
        if (errno != 0) {
            perror("fsReadDir");
        }

        //Close main directory
        printf("Close main directory\n");
        printf("fsCloseDir(): %d\n", fsCloseDir(fd1));
        
        
        
        //Reopen main directory
        printf("Open main directory\n");
        fd1 = fsOpenDir(dirname);
        if (fd1 == NULL) {
            perror("fsOpenDir"); exit(1);
        }

        //Reread main directory
        printf("Read main directory\n");
        fdent1 = NULL;
        for (fdent1 = fsReadDir(fd1); fdent1 != NULL; fdent1 = fsReadDir(fd1)) {
            printf("\t %s, %d\n", fdent1->entName, (int)(fdent1->entType));
        }
        if (errno != 0) {
            perror("fsReadDir");
        }

        //Reclose main directory
        printf("Close main directory\n");
        printf("fsCloseDir(): %d\n", fsCloseDir(fd1));
        
        
        
        //Open subdirectory
        printf("Open subdirectory\n");
        fd1 = fsOpenDir(subdirectory);
        if (fd1 == NULL) {
            perror("fsOpenDir"); exit(1);
        }

        //Read subdirectory
        printf("Read subdirectory\n");
        fdent1 = NULL;
        for (fdent1 = fsReadDir(fd1); fdent1 != NULL; fdent1 = fsReadDir(fd1)) {
            printf("\t %s, %d\n", fdent1->entName, (int)(fdent1->entType));
        }
        if (errno != 0) {
            perror("fsReadDir");
        }

        //Close subdirectory
        printf("Close subdirectory\n");
        printf("fsCloseDir(): %d\n", fsCloseDir(fd1));
        
        
        
        //Open subsubdirectory
        printf("Open subsubdirectory\n");
        fd1 = fsOpenDir(subsubdirectory);
        if (fd1 == NULL) {
            perror("fsOpenDir"); exit(1);
        }

        //Read subsubdirectory
        printf("Read subsubdirectory\n");
        fdent1 = NULL;
        for (fdent1 = fsReadDir(fd1); fdent1 != NULL; fdent1 = fsReadDir(fd1)) {
            printf("\t %s, %d\n", fdent1->entName, (int)(fdent1->entType));
        }
        if (errno != 0) {
            perror("fsReadDir");
        }

        //Close subsubdirectory
        printf("Close subsubdirectory\n");
        printf("fsCloseDir(): %d\n", fsCloseDir(fd1));
        
        
        
        //Open main directory
        printf("Open main directory\n");
        fd1 = fsOpenDir(dirname);
        if (fd1 == NULL) {
            perror("fsOpenDir"); exit(1);
        }

        //Read main directory
        printf("Read main directory\n");
        fdent1 = NULL;
        for (fdent1 = fsReadDir(fd1); fdent1 != NULL; fdent1 = fsReadDir(fd1)) {
            printf("\t %s, %d\n", fdent1->entName, (int)(fdent1->entType));
        }
        if (errno != 0) {
            perror("fsReadDir");
        }
        
        //Open subdirectory
        printf("Open subdirectory\n");
        fd2 = fsOpenDir(subdirectory);
        if (fd2 == NULL) {
            perror("fsOpenDir"); exit(1);
        }

        //Read subdirectory
        printf("Read subdirectory\n");
        fdent2 = NULL;
        for (fdent2 = fsReadDir(fd2); fdent2 != NULL; fdent2 = fsReadDir(fd2)) {
            printf("\t %s, %d\n", fdent2->entName, (int)(fdent2->entType));
        }
        if (errno != 0) {
            perror("fsReadDir");
        }
        
        //Close main directory
        printf("Close main directory\n");
        printf("fsCloseDir(): %d\n", fsCloseDir(fd1));

        //Close subdirectory
        printf("Close subdirectory\n");
        printf("fsCloseDir(): %d\n", fsCloseDir(fd2));

        
        //Test for errors of opendir
        printf("Open NULL directory\n");
        fd2 = fsOpenDir(NULL);
        if (fd2 == NULL) {
            printf("success %d\n", errno);
        }
        else {
            printf("ERROR USING INVALID DIRECTORY\n"); exit(1);
        }
        
        printf("Open invalid directory\n");
        fd2 = fsOpenDir("gordon");
        if (fd2 == NULL) {
            printf("success %d\n", errno);
        }
        else {
            printf("ERROR USING INVALID DIRECTORY\n"); exit(1);
        }
        
        printf("Open empty directory path\n");
        fd2 = fsOpenDir("");
        if (fd2 == NULL) {
            printf("success %d\n", errno);
        }
        else {
            printf("ERROR USING EMPTY DIRECTORY PATH\n"); exit(1);
        }
        
        printf("Open //// directory path\n");
        fd2 = fsOpenDir("////");
        if (fd2 == NULL) {
            printf("success %d\n", errno);
        }
        else {
            printf("ERROR USING EMPTY DIRECTORY PATH\n"); exit(1);
        }
        
        printf("Open invalid myfolder subdirectory path\n");
        char* invalid = malloc(strlen(dirname) + 1 + 6 + 1);
        strcpy(invalid, dirname);
        strcat(invalid, "/");
        strcat(invalid, "huhuhu");
        strcat(invalid, "\0");
        fd2 = fsOpenDir(invalid);
        if (fd2 == NULL) {
            printf("success %d\n", errno);
        }
        else {
            printf("ERROR USING EMPTY DIRECTORY PATH\n"); exit(1);
        }
        free(invalid);
        
        

        //Test for errors of readdir and closedir
        printf("Null fsdir\n");
        fd2 = NULL;
        fdent2 = fsReadDir(fd2);
        if (errno == 0) {
            printf("ERROR using NULL\n"); exit(1);
        }
        printf("success %d\n", errno);
        
        printf("fsCloseDir(): %d\n", fsCloseDir(NULL));
        
        printf("Already closed fsdir\n");
        fdent2 = fsReadDir(fd1);
        if (errno == 0) {
            printf("ERROR using closed fsdir\n"); exit(1);
        }
        printf("success %d\n", errno);
        
        printf("invalid fsdir\n");
        fd1->id = 525;
        fdent2 = fsReadDir(fd1);
        if (errno == 0) {
            printf("ERROR using closed fsdir\n"); exit(1);
        }
        printf("success %d\n", errno);
        
        printf("fsCloseDir(): %d\n", fsCloseDir(fd1));
        printf("success %d\n", errno);
        
        
        //Unmount
        if (fsUnmount(dirname) < 0) {
            perror("fsUnmount"); exit(1);
        }
    }
    else if (mode == 4)
    {
        //Validate custom directory functionality
        
        //Mount
        char *dirname = argv[3];
        printf("fsMount(): %d\n", fsMount(argv[1], atoi(argv[2]), dirname));
        
        char *subdirectory = malloc(strlen(dirname) + 1 + 6 + 1);
        strcpy(subdirectory, dirname);
        strcat(subdirectory, "/");
        strcat(subdirectory, "gordon");
        strcat(subdirectory, "\0");
        printf("subdirectory path: %s\n", subdirectory);
        
        char *subsubdirectory = malloc(strlen(subdirectory) + 1 + 6 + 1);
        strcpy(subsubdirectory, subdirectory);
        strcat(subsubdirectory, "/");
        strcat(subsubdirectory, "josh");
        strcat(subsubdirectory, "\0");
        printf("subsubdirectory path: %s\n", subsubdirectory);
        
        FSDIR *fd1 = NULL;
        struct fsDirent *fdent1;
        char input[256];
        while (strcmp(input, "exit") != 0)
        {
            scanf("%79s", input);
            if (strcmp(input, "opendir") == 0)
            {
                if (fd1 != NULL)
                {
                    printf("already opened a file\n");
                }
                else
                {
                    scanf("%79s", input); 
                    if (strcmp(input, "main") == 0)
                    {
                        //Open main directory
                        printf("Open main directory\n");
                        fd1 = fsOpenDir(dirname);
                        if (fd1 == NULL) {
                            perror("fsOpenDir"); exit(1);
                        }
                    }
                    else if (strcmp(input, "sub") == 0)
                    {
                        printf("Open subdirectory\n");
                        fd1 = fsOpenDir(subdirectory);
                        if (fd1 == NULL) {
                            perror("fsOpenDir"); exit(1);
                        }
                    }
                    else if (strcmp(input, "subsub") == 0)
                    {
                        printf("Open subsubdirectory\n");
                        fd1 = fsOpenDir(subsubdirectory);
                        if (fd1 == NULL) {
                            perror("fsOpenDir"); exit(1);
                        }
                    }
                }
            }
            else if (strcmp(input, "readdir") == 0)
            {
                if (fd1 == NULL)
                {
                    printf("open dir first\n");
                }
                else
                {
                    //Read main directory
                    printf("Read directory\n");
                    fdent1 = NULL;
                    for (fdent1 = fsReadDir(fd1); fdent1 != NULL; fdent1 = fsReadDir(fd1)) {
                        printf("\t %s, %d\n", fdent1->entName, (int)(fdent1->entType));
                    }
                    if (errno != 0) {
                        perror("fsReadDir");
                    }
                }
            }
            else if (strcmp(input, "closedir") == 0)
            {
                if (fd1 == NULL)
                {
                    printf("open dir first\n");
                }
                else
                {
                    //Close main directory
                    printf("Close directory\n");
                    printf("fsCloseDir(): %d\n", fsCloseDir(fd1));
                    fd1 = NULL;
                }
            }
        }
        
        free(subdirectory);
        free(subsubdirectory);
        
        
        //Unmount
        if (fsUnmount(dirname) < 0) {
            perror("fsUnmount"); exit(1);
        }
    }
    else if (mode == 5)
    {
        //Mount
        char *dirname = argv[3];
        printf("fsMount(): %d\n", fsMount(argv[1], atoi(argv[2]), dirname));

        
        
        int ff = open("/dev/urandom", 0);
        if (ff < 0) {
            perror("open(/dev/urandom)"); exit(1);
        }
        else printf("open(): %d\n", ff);

        char fname[256];
        sprintf(fname, "%s/", dirname);
        if (read(ff, (void *)(fname + strlen(dirname) + 1), 10) < 0) {
            perror("read(/dev/urandom)"); exit(1);
        }

        int i;
        for (i = 0; i < 10; i++) {
            //printf("%d\n", ((unsigned char)(fname[i]))%26);
            fname[i + strlen(dirname) + 1] = ((unsigned char)(fname[i + strlen(dirname) + 1])) % 26 + 'a';
        }
        fname[10 + strlen(dirname) + 1] = (char)0;
        printf("Filename to write: %s\n", (char *)fname);

        char buf[256];
        if (read(ff, (void *)buf, 256) < 0) {
            perror("read(2)"); exit(1);
        }

        printBuf(buf, 256);

        printf("close(): %d\n", close(ff));

        printf("Open write\n");
        ff = fsOpen(fname, 1);
        if (ff < 0) {
            perror("fsOpen(write)"); exit(1);
        }

        printf("Write 0 bytes\n");
        if (fsWrite(ff, buf, 0) != 0) {
            fprintf(stderr, "fsWrite() did not write 0\n");
        }

        printf("Write buffer\n");
        if (fsWrite(ff, buf, 256) < 256) {
            fprintf(stderr, "fsWrite() wrote fewer than 256\n");
        }

        printf("CloseFile\n");
        if (fsClose(ff) < 0) {
            perror("fsClose"); exit(1);
        }

        printf("Open read\n");
        char readbuf[256];
        if ((ff = fsOpen(fname, 0)) < 0) {
            perror("fsOpen(read)"); exit(1);
        }

        int readcount = -1;

        if ((readcount = fsRead(ff, readbuf, 256)) < 256) {
            fprintf(stderr, "fsRead() read fewer than 256\n");
        }

        if (memcmp(readbuf, buf, readcount)) {
            fprintf(stderr, "return buf from fsRead() differs from data written!\n");
        }
        else {
            printf("fsread(): return buf identical to data written upto %d bytes.\n", readcount);
        }

        if (fsClose(ff) < 0) {
            perror("fsClose"); exit(1);
        }

        printf("fsRemove(%s): %d\n", fname, fsRemove(fname));

        if (fsUnmount(dirname) < 0) {
            perror("fsUnmount"); exit(1);
        }
    }
        

    return 0;
}
