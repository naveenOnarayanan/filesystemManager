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
    struct mount_list * mount = find_mount(folderName);
    if (mount == NULL) {
        printf("Directory path is not mounted\n");
        return NULL;
    }

    const char * path = get_relative_path(folderName, mount);

    return_type result = make_remote_call(mount->serverIPorHost,
                                          mount->serverPort,
                                          "fsOpenDir", 1,
                                          strlen(path) + 1, path);

    int inError = result.in_error;

    if (inError == 1) {
        int errorNum = *(int *)(result.return_val);
        errno = errorNum;
    }

    FSDIR * dir = (FSDIR *) result.return_val;
    FSDIR * add_dir_result = (FSDIR *) malloc(sizeof(int));
    *add_dir_result = add_dir(dir, mount);
    return add_dir_result;
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
    struct dir_list * dir_obj = find_dir(folder);

    if (dir_obj == NULL) {
        return NULL;
    }

    return_type result = make_remote_call(dir_obj->mount->serverIPorHost,
                                          dir_obj->mount->serverPort,
                                          "fsReadDir", 1,
                                          sizeof(folder), folder);

    int inError = result.in_error;

    if (inError == 1) {
        int errorNum = *(int *)(result.return_val);
        errno = errorNum;
        return NULL;
    }

    struct fsDirent * fsdir = result.return_val;

    if (result.return_size == 0) {
        return NULL;
    } else {
        return (struct fsDirent *) result.return_val;
    }
}

int fsOpen(const char *fname, int mode) {
    struct mount_list * mount = find_mount(fname);
    if (mount == NULL) {
        return -1;
    }

    const char * path = get_relative_path(fname, mount);

    return_type result = make_remote_call(mount->serverIPorHost,
                                          mount->serverPort,
                                          "fsOpen", 2,
                                          strlen(path) + 1, path,
                                          sizeof(int), (void *) &mode);

    int inError = result.in_error;

    if (inError == 1) {
        int errorNum = *(int *)(result.return_val);
        errno = errorNum;
        return -1;
    }

   if (result.return_size == sizeof(int)) {
        int fd = *(int *)result.return_val;
        return add_fd(mount, fd);
    }

    return 0;
}

int fsClose(int fd) {
    struct file_desc_list * fd_obj = find_fd(&fd, FILTER_BY_ID);
    if (fd_obj == NULL) {
        return -1;
    }

    return_type result = make_remote_call(fd_obj->mount->serverIPorHost,
                                          fd_obj->mount->serverPort,
                                          "fsClose", 1,
                                          sizeof(int), &fd_obj->fd);

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
    struct file_desc_list * file_obj = find_fd(&fd, FILTER_BY_ID);
    if (file_obj == NULL) {
        return -1;
    }

    return_type result = make_remote_call(file_obj->mount->serverIPorHost,
                                          file_obj->mount->serverPort,
                                          "fsRead", 2,
                                          sizeof(int), (void *) &file_obj->fd,
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

    memcpy(buf, result.return_val, result.return_size);
    return result.return_size;

}

int fsWrite(int fd, const void *buf, const unsigned int count) {
    struct file_desc_list * file_obj = find_fd(&fd, FILTER_BY_ID);
    if (file_obj == NULL) {
        return -1;
    }

    return_type result = make_remote_call(file_obj->mount->serverIPorHost,
                                          file_obj->mount->serverPort,
                                          "fsWrite", 3,
                                          sizeof(int), (void *) &file_obj->fd,
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
    if (mount == NULL) {
        return -1;
    }

    const char * relative_path = get_relative_path(name, mount);

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

    if (result.return_size == 0) {
        return -1;
    }

    return *(int *)result.return_val;
}
