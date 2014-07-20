/* Mahesh V. Tripunitara
   University of Waterloo
   Part of a simplified RPC implementation
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include "simplified_rpc/ece454rpc_types.h"
#include "s_util.c"


#if 1
#define _DEBUG_1_
#endif

/* We allocate a global variable for returns. However, 
 * r.return_val is allocated dynamically. We look to
 * the server_stub to deallocate after it returns the
 * response to the client. */

return_type r;

extern printRegisteredProcedures();

return_type isAlive(const int i, arg_type *a) {
    int* isAlive = (int *)malloc(sizeof(int));
    *isAlive = 1;
    r.in_error = 1;
    r.return_val = (void*)isAlive;
    r.return_size = sizeof(int);
    return r;
}

return_type fsOpen(const int nparams, arg_type *a) {
    if (nparams != 2) {
        r.return_val = set_error(EINVAL);
        r.in_error = 1;
        r.return_size = sizeof(int);
        return r;
    }

    char * folderName = a->arg_val;
    int mode = *(int*)a->next->arg_val;

    if (resource_in_use(folderName) == 0) {
        char * serverFolder = append_local_path(folderName);

        printf("Final folder name: %s\n", serverFolder);

    
        int flag;
        if (mode == 0) {
            flag = O_RDONLY;
        } else {
            flag = O_WRONLY | O_CREAT;
        }

        int *fileDescriptor = (int*)malloc(sizeof(int));
        *fileDescriptor = open(serverFolder, flag, S_IRWXU);
        printf("Error: %s\n", strerror(errno));

        printf("FileDescriptor: %d\n", *fileDescriptor);

        if (*fileDescriptor < 0) {
            r.return_val = set_error(EBADF);
            r.in_error = 1;
            r.return_size = sizeof(int);
        } else {
            add_resource(folderName, *fileDescriptor);

            r.return_val = (void *)fileDescriptor;
            r.return_size = sizeof(int);
            r.in_error = 0;
        }

        free(serverFolder);
    } else {
        r.return_val = set_error(EBUSY);
        r.return_size = 0;
        r.in_error = 1;
    }

    return r;
}

return_type fsClose(const int nparams, arg_type *a) {
    if (nparams != 1) {
        r.return_val = set_error(EINVAL);
        r.return_size = sizeof(int);
        r.in_error = 1;
        return r;
    } 

    int resource_id = *(int *)a->arg_val;
    int * resource_removed = malloc(sizeof(int));
    *resource_removed = remove_resource(*(int *)a->arg_val);

    if (*resource_removed == 1) {
        r.return_val = (void *) resource_removed;
        r.return_size = sizeof(int);
        r.in_error = 0;
    } else {
        r.return_val = set_error(EBUSY);
        r.return_size = sizeof(int);
        r.in_error = 1;
    }
    return r;
}

return_type fsRead(const int nparams, arg_type *a) {
    printf("params: %d\n", nparams);
    if (nparams == 2) {
        int fileDescriptor = *(int *)a->arg_val;
        unsigned int count = *(unsigned int*) a->next->arg_val;
        printf("File descriptor: %d\n", fileDescriptor);
        char * buff = malloc(count * sizeof(char));
        int total_read = read(fileDescriptor, buff, (size_t)count);
        if (total_read >= 0) {
            r.return_size = total_read;
            r.return_val = buff;
            r.in_error = 0;
        } else {
            printf("Oh no! %s\n", strerror(errno));
        }
    } else {
        printf("Number of parameters dont match! %d\n", nparams);
        r.return_val = set_error(EINVAL);
        r.return_size = sizeof(int);
        r.in_error = 1;
    }

    return r;
}

return_type fsWrite(const int nparams, arg_type *a) {
    if (nparams != 3) {
        r.return_val = set_error(EINVAL);
        r.return_size = sizeof(int);
        r.in_error = 1;
        return r;
    }

    int fd = *(int*)a->arg_val;
    int count = *(int *)a->next->arg_val;

    arg_type * buff_arg = a->next->next;
    int buff_length = buff_arg->arg_size;
    char * buff = malloc(buff_length * sizeof(char));
    memcpy(buff, buff_arg->arg_val, buff_length);

    int * write_result = malloc(sizeof(int));
    *write_result = write(fd, buff, (size_t)count);

    printf("The write result: %d\n", *write_result);

    if (*write_result > -1) {
        r.return_val = (void *) write_result;
        r.return_size = sizeof(int);
        r.in_error = 0;
    } else {
        // Don't know which one is more appropriate in this case
        r.return_val = set_error(EPERM); // Operation not permitted
        //r.return_val = set_error(EAGAIN); // Try Again
        r.return_size = sizeof(int);
        r.in_error = 1;
    }

    return r;
}

return_type fsRemove(const int nparams, arg_type *a) {
    if (nparams != 1) {
        r.return_val = set_error(EINVAL);
        r.return_size = sizeof(int);
        r.in_error = 1;
        return r;
    }

    if (resource_in_use(a->arg_val) == 0) {
        int * remove_result = malloc(sizeof(int));
        char * folderName = append_local_path(a->arg_val);
        printf("Folder to remove: %s\n", folderName);

        *remove_result = remove(folderName);
        printf("Print the error: %s\n",strerror(errno));

        r.return_val = (void *)remove_result;
        r.return_size = sizeof(int);
        r.in_error = 0;
    } else {
        r.return_val = set_error(EBUSY); // Resource is busy
        r.return_size = sizeof(int);
        r.in_error = 1;
    }

    return r;
}

return_type fsOpenDir(const int nparams, arg_type *a) {
    printf("The number of parameters: %d\n", nparams);
    if (nparams != 1) {
        r.return_val = set_error(EINVAL);
        r.return_size = sizeof(int);
        r.in_error = 1;
        return r;
    }

    printf("Trying to open dir: %s\n", (char *)a->arg_val);

    char * serverFolder = append_local_path(a->arg_val);
    DIR * dir = opendir(serverFolder);

    if (dir == NULL) {
        r.return_val = set_error(ENOTDIR);
        r.return_size = sizeof(int);
        r.in_error = 1;
    } else {
        int * id = malloc(sizeof(int));
        printf("ID: %p\n", id);
        *id = add_dir(dir);
        printf("ID: %p\n", id);
        printf("ID: %d\n", *id);

        r.return_val = (void *) id;
        r.return_size = sizeof(int);
        r.in_error = 0;
    }
    free(serverFolder);
    return r;

}

return_type fsCloseDir(const int nparams, arg_type *a) {
    if (nparams != 1) {
        r.return_val = set_error(EINVAL);
        r.return_size = sizeof(int);
        r.in_error = 1;
        return r;
    }

    struct dir_queue * dir = find_dir(*(int *)a->arg_val);

    if (dir == NULL) {
        r.return_val = set_error(ENOTDIR);
        r.return_size = sizeof(int);
        r.in_error = 1;
        return r;
    }


    int * close_dir_result = malloc(sizeof(int));
    *close_dir_result = closedir(dir->dir);

    printf("Closing dir: %d\n", *close_dir_result);

    if (*close_dir_result == 0) {
        remove_dir(*(int *)a->arg_val);
    }

    r.return_val = (void *)close_dir_result;
    r.return_size = sizeof(int);
    r.in_error = 0;
    return r;
}

return_type fsReadDir(const int nparams, arg_type *a) {
    if (nparams != 1) {
        r.return_val = set_error(EINVAL);
        r.return_size = sizeof(int);
        r.in_error = 1;
        return r;
    }

    int id = *(int *) a->arg_val;
    struct dir_queue * dir = find_dir(id);
    if (dir == NULL) {
        r.return_val = set_error(ENOTDIR);
        r.return_size = sizeof(int);
        r.in_error = 1;
        return r;
    }

    struct dirent * dir_info = readdir(dir->dir);
    if (dir_info == NULL) {
        r.return_val = set_error(ENOENT);
        r.return_size = sizeof(int);
        r.in_error = 1;
    } else {
        printf("Directory info: %p\n", dir_info);
        printf("Dirent: %s\n", dir_info->d_name);
        printf("Size of dir_info: %lu\n", sizeof(dir_info));
        
        struct fsDirent * dirent = malloc(sizeof(struct fsDirent));

        if(dir_info->d_type == DT_DIR) {
            dirent->entType = 1;
        }
        else if(dir_info->d_type == DT_REG) {
            dirent->entType = 0;
        }
        else {
            dirent->entType = -1;
        }

        memcpy(&dirent->entName, &dir_info->d_name, 256);

        r.return_size = sizeof(struct fsDirent);
        r.return_val = (void *)dirent;
        r.in_error = 0;
    }

    return r;
}

void registerMountFolder(const char * folderName) {
    size_t folderNameLen = strlen(folderName) + 1;
    hostFolder.hostedFolderName = malloc(folderNameLen * sizeof(char));
    memcpy(hostFolder.hostedFolderName, folderName, folderNameLen);
    hostFolder.hostedFolderNameLength = strlen(folderName);
}

int main(int argc, char*argv[]) {

    if (argc == 2) {
        registerMountFolder(argv[1]);
        register_procedure("isAlive", 0, isAlive);
        register_procedure("fsOpen", 2, fsOpen);
        register_procedure("fsClose", 1, fsClose);
        register_procedure("fsRead", 2, fsRead);
        register_procedure("fsWrite", 3, fsWrite);
        register_procedure("fsRemove", 1, fsRemove);
        register_procedure("fsOpenDir", 1, fsOpenDir);
        register_procedure("fsReadDir", 1, fsReadDir);
        register_procedure("fsCloseDir", 1, fsCloseDir);

#ifdef _DEBUG_1_
        printRegisteredProcedures();
#endif
        printf("Registered Folder: %s\n", hostFolder.hostedFolderName);

        launch_server();
    } else {
        printf("Format: ./server <folder_path>\n");
    }
    return 0;
}
