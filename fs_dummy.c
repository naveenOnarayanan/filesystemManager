/* 
 * Mahesh V. Tripunitara
 * University of Waterloo
 * A dummy implementation of the functions for the remote file
 * system. This file just implements those functions as thin
 * wrappers around invocations to the local filesystem. In this
 * way, this dummy implementation helps clarify the semantics
 * of those functions. Look up the man pages for the calls
 * such as opendir() and read() that are made here.
 */
#include "ece454_fs.h"
#include "ece454rpc_types.h"
#include <string.h>
#include <stdlib.h>

struct fsDirent dent;
char * folderAlias;
char * serverIPOrDomainName;
int serverPort;
int folderAliasLength = 0;


int fsMount(const char *srvIpOrDomName, const unsigned int srvPort, const char *localFolderName) {
    return_type check = make_remote_call(srvIpOrDomName,
                                            srvPort,
                                            "isAlive", 0);
    int isServerAlive = *(int *)(check.return_val);

    if (isServerAlive == 1) {
        serverIPOrDomainName = malloc(strlen(srvIpOrDomName) * sizeof(char));
        strcpy(serverIPOrDomainName, srvIpOrDomName);

        serverPort = srvPort;

        folderAliasLength = strlen(localFolderName);
        folderAlias = malloc(folderAliasLength * sizeof(char));

        strcpy(folderAlias, localFolderName);
        
        printf("Folder name: %s\n", folderAlias);
        return isServerAlive;
    } else {
        return -1;
    }
}

int fsUnmount(const char *localFolderName) {
    printf("folderAlias: %s\n", folderAlias);
    if (strcmp(folderAlias, localFolderName) == 0) {
        folderAliasLength = 0;
        serverPort = 0;
        free(serverIPOrDomainName);
        free(folderAlias);
        return 0;
    }
    return -1;
}

FSDIR* fsOpenDir(const char *folderName) {
    return(opendir(folderName));
}

int fsCloseDir(FSDIR *folder) {
    return(closedir(folder));
}

struct fsDirent *fsReadDir(FSDIR *folder) {
    const int initErrno = errno;
    struct dirent *d = readdir(folder);

    if(d == NULL) {
	if(errno == initErrno) errno = 0;
	return NULL;
    }

    if(d->d_type == DT_DIR) {
	dent.entType = 1;
    }
    else if(d->d_type == DT_REG) {
	dent.entType = 0;
    }
    else {
	dent.entType = -1;
    }

    memcpy(&(dent.entName), &(d->d_name), 256);
    return &dent;
}

int fsOpen(const char *fname, int mode) {
    printf("serverIP: %s\n", serverIPOrDomainName);
    printf("serverPort: %d\n", serverPort);

    if (strncmp(fname, folderAlias, folderAliasLength) == 0) {
        char * tmp = fname;
        tmp+= folderAliasLength;

        return_type result = make_remote_call(serverIPOrDomainName,
                                              serverPort,
                                              "fsOpen", 2,
                                              strlen(tmp) + 1, tmp,
                                              sizeof(int), (void *) &mode);

       if (result.return_size == sizeof(int)) {
            return *(int *)result.return_val;
        }
    }

    return 0;
}

int fsClose(int fd) {
    return(close(fd));
}

int fsRead(int fd, void *buf, const unsigned int count) {
    return_type result = make_remote_call(serverIPOrDomainName,
                                          serverPort,
                                          "fsRead", 2,
                                          sizeof(int), (void *) &fd,
                                          sizeof(unsigned int), (void *)&count);

    if (result.return_size == 0) {
        return -1;
    }

    printf("Return size: %d\n", result.return_size);
    printBuf(result.return_val, result.return_size);

    memcpy(buf, result.return_val, result.return_size);
    return 0;

    //return(read(fd, buf, (size_t)count));
}

int fsWrite(int fd, const void *buf, const unsigned int count) {
    return(write(fd, buf, (size_t)count)); 
}

int fsRemove(const char *name) {
    return(remove(name));
}
