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
#include "fs_util.c"
#include "simplified_rpc/ece454rpc_types.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

struct fsDirent dent;
char * folderAlias;
char * serverIPOrDomainName;
int serverPort;
size_t folderAliasLength = 0;


int fsMount(const char *srvIpOrDomName, const unsigned int srvPort, const char *localFolderName) {
    struct mount_list * mount = find_mount(localFolderName);
    if (mount != NULL) {
        printf("already mounted\n");
        return -1;
    }

    return_type check = make_remote_call(srvIpOrDomName,
                                            srvPort,
                                            "isAlive", 0);

    int inError = check.in_error;

    if (inError == 1) {
        int errorNum = *(int *)(check.return_val);
        errno = errorNum;
        return -1;
    }

    int isServerAlive = *(int *)(check.return_val);

    if (isServerAlive == 1) {

        add_mount(srvIpOrDomName, srvPort, localFolderName);
        
        return isServerAlive;
    } else {
        return -1;
    }
}

int fsUnmount(const char *localFolderName) {
    return remove_mount(localFolderName);
}

FSDIR* fsOpenDir(const char *folderName) {
    printf("The folder is called: %s\n", folderName);
    struct mount_list * mount = find_mount(folderName);
    printf("Mount returned: %s\n", mount->serverIPorHost);
    if (mount == NULL) {
        return NULL;
    }

    const char * path = get_relative_path(folderName, mount);

    printf("Relative folder path: %s\n", path);

    return_type result = make_remote_call(mount->serverIPorHost,
                                          mount->serverPort,
                                          "fsOpenDir", 1,
                                          strlen(path) + 1, path);

    printf("returned from call\n");
    int inError = result.in_error;

    if (inError == 1) {
        int errorNum = *(int *)(result.return_val);
        errno = errorNum;
    }

    printf("Returned val: %d\n", *(int *)result.return_val);

    FSDIR * dir = (FSDIR *) result.return_val;
    add_dir(dir, mount);

    return dir;



    // if (strncmp(folderName, folderAlias, folderAliasLength) == 0) {
    //     char * temp = folderName;
    //     temp += folderAliasLength;
    //     FSDIR retDir;

    //     return_type dir = make_remote_call(srvIpOrDomName,
    //                                         srvPort,
    //                                         "fsOpenDir",
    //                                         0);

    //     if (dir.return_size == 0) {
    //         // TODO: Set the errno appropriately
    //         // errno = -1;
    //         return NULL;
    //     } else {
    //         retDir = return_type.return_val;
    //         return retDir;
    //     }
    // }

    //return(opendir(folderName));
}

int fsCloseDir(FSDIR *folder) {
    struct dir_list * dir_obj = find_dir(folder);
    if (dir_obj == NULL) {
        return -1;
    }

    return_type result = make_remote_call(dir_obj->mount->serverIPorHost,
                                          dir_obj->mount->serverPort,
                                          "fsCloseDir", 1,
                                          sizeof(folder), folder);

    int inError = result.in_error;

    if (inError == 1) {
        int errorNum = *(int *)(result.return_val);
        errno = errorNum;
        return -1;
    }

    if (result.return_size == 0) {
        return -1;
    } else {
        return *(int *)result.return_val;
    }


}

struct fsDirent * fsReadDir(FSDIR *folder){
    printf("ReadDir ptr: %p\n", folder);
    struct dir_list * dir_obj = find_dir(folder);

    if (dir_obj == NULL) {
        return NULL;
    }

    return_type result = make_remote_call(dir_obj->mount->serverIPorHost,
                                          dir_obj->mount->serverPort,
                                          "fsReadDir", 1,
                                          sizeof(folder), folder);

    int inError = result.in_error;
    printf("In error? : %d\n", inError);

    if (inError == 1) {
        int errorNum = *(int *)(result.return_val);
        errno = errorNum;
        return NULL;
    }

    struct fsDirent * fsdir = result.return_val;
    printf("READ DIR: %s\n", fsdir->entName);

    if (result.return_size == 0) {
        return NULL;
    } else {
        return (struct fsDirent *) result.return_val;
    }

 //    const int initErrno = errno;
 //    struct dirent *d = readdir(folder);

 //    if(d == NULL) {
	// if(errno == initErrno) errno = 0;
	// return NULL;
 //    }

 //    if(d->d_type == DT_DIR) {
	// dent.entType = 1;
 //    }
 //    else if(d->d_type == DT_REG) {
	// dent.entType = 0;
 //    }
 //    else {
	// dent.entType = -1;
 //    }

 //    memcpy(&(dent.entName), &(d->d_name), 256);
 //    return &dent;
}

int fsOpen(const char *fname, int mode) {
    struct mount_list * mount = find_mount(fname);
    if (mount == NULL) {
        return -1;
    }

    printf("Mount folder: %s\n", mount->localFolder);

    const char * path = get_relative_path(fname, mount);

    printf("OPEN: %s\n", path);

    return_type result = make_remote_call(mount->serverIPorHost,
                                          mount->serverPort,
                                          "fsOpen", 2,
                                          strlen(path) + 1, path,
                                          sizeof(int), (void *) &mode);

    printf("Return val for fsOpen: %d\n", *(int *)result.return_val);

    int inError = result.in_error;

    if (inError == 1) {
        int errorNum = *(int *)(result.return_val);
        errno = errorNum;
        return -1;
    }

   if (result.return_size == sizeof(int)) {
        int fd = *(int *)result.return_val;
        add_fd(mount, fd);
        return fd;
    }

    return 0;
}

int fsClose(int fd) {
    struct file_desc_list * fd_obj = find_fd(&fd, FILE_DESCRIPTOR);
    if (fd_obj == NULL) {
        return -1;
    }

    return_type result = make_remote_call(fd_obj->mount->serverIPorHost,
                                          fd_obj->mount->serverPort,
                                          "fsClose", 1,
                                          sizeof(int), &fd);

    int inError = result.in_error;

    if (inError == 1) {
        int errorNum = *(int *)(result.return_val);
        errno = errorNum;
        return -1;
    }

    if (result.return_size > 0) {
        return remove_fd(fd);
    } else {
        return -1;
    }
}

int fsRead(int fd, void *buf, const unsigned int count) {
    struct file_desc_list * file_obj = find_fd(&fd, FILE_DESCRIPTOR);
    if (file_obj == NULL) {
        return -1;
    }

    return_type result = make_remote_call(file_obj->mount->serverIPorHost,
                                          file_obj->mount->serverPort,
                                          "fsRead", 2,
                                          sizeof(int), (void *) &fd,
                                          sizeof(unsigned int), (void *)&count);

    int inError = result.in_error;

    if (inError == 1) {
        int errorNum = *(int *)(result.return_val);
        errno = errorNum;
        return -1;
    }

    if (result.return_size == 0) {
        return -1;
    }

    printf("Return size: %d\n", result.return_size);
    printBuf(result.return_val, result.return_size);

    memcpy(buf, result.return_val, result.return_size);
    return result.return_size;

    //return(read(fd, buf, (size_t)count));
}

int fsWrite(int fd, const void *buf, const unsigned int count) {
    struct file_desc_list * file_obj = find_fd(&fd, FILE_DESCRIPTOR);
    if (file_obj == NULL) {
        return -1;
    }

    printf("Size of buff to write: %lu\n", strlen(buf));
    return_type result = make_remote_call(file_obj->mount->serverIPorHost,
                                          file_obj->mount->serverPort,
                                          "fsWrite", 3,
                                          sizeof(int), (void *) &fd,
                                          sizeof(unsigned int), (void *)&count,
                                          count, buf);

    int inError = result.in_error;

    if (inError == 1) {
        int errorNum = *(int *)(result.return_val);
        errno = errorNum;
        return -1;
    }

    if (result.return_size == 0) {
        return -1;
    }

    return *(int *)result.return_val;
}

int fsRemove(const char *name) {
    struct mount_list * mount = find_mount(name);
    printf("MOUNT FOUND IN REMOVE: %p\n", mount);
    if (mount == NULL) {
        return -1;
    }

    char * relative_path = get_relative_path(name, mount);

    return_type result = make_remote_call(mount->serverIPorHost,
                                          mount->serverPort,
                                          "fsRemove", 1,
                                          strlen(relative_path) + 1, relative_path);

    int inError = result.in_error;

    if (inError == 1) {
        int errorNum = *(int *)(result.return_val);
        errno = errorNum;
        return -1;
    }

    printf("Result returned: %d\n", result.return_size);
    if (result.return_size == 0) {
        return -1;
    }

    return *(int *)result.return_val;
}
