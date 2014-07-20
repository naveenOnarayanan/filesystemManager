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
host_folder hostFolder;


extern printRegisteredProcedures();

return_type isAlive(const int i, arg_type *a) {
    int* isAlive = (int *)malloc(sizeof(int));
    *isAlive = 1;
    r.return_val = (void*)isAlive;
    r.return_size = sizeof(int);
    return r;
}

return_type fsOpen(const int nparams, arg_type *a) {
    if (nparams != 2) {
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }

    char * folderName = a->arg_val;
    int mode = *(int*)a->next->arg_val;

    if (resource_in_use(folderName) == 0) {
        size_t totalLength = hostFolder.hostedFolderNameLength;
        totalLength += strlen(folderName) + 1;
        char * serverFolder = malloc(totalLength * sizeof(char));
        strcpy(serverFolder, (char *) hostFolder.hostedFolderName);
        strcat(serverFolder, (char *) folderName);

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
            r.return_val = NULL;
            r.return_size = 0;
        } else {
            add_resource(folderName, *fileDescriptor);

            r.return_val = (void *)fileDescriptor;
            r.return_size = sizeof(int);
        }

        free(serverFolder);
    } else {
        r.return_val = NULL;
        r.return_size = 0;
    }

    return r;
}

return_type fsClose(const int nparams, arg_type *a) {
    if (nparams != 1) {
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    } 

    int resource_id = *(int *)a->arg_val;
    int * resource_removed = malloc(sizeof(int));
    *resource_removed = remove_resource(*(int *)a->arg_val);

    if (*resource_removed == 1) {
        r.return_val = (void *) resource_removed;
        r.return_size = sizeof(int);
    } else {
        r.return_val = NULL;
        r.return_size = 0;
    }
    return r;
}

return_type fsRead(const int nparams, arg_type *a) {
    printf("params: %d\n", nparams);
    if (nparams == 2) {
        int fileDescriptor = *(int *)a->arg_val;
        unsigned int count = *(unsigned int*) a->next->arg_val;
        printf("File descriptor: %d\n", fileDescriptor);
        printf("Count: %d\n", count);
        char * buff = malloc(count * sizeof(char));
        if (read(fileDescriptor, buff, (size_t)count) >= 0) {
            printf("Was able to read file\n");
            r.return_size = strlen(buff);
            r.return_val = buff;
            printBuf(buff, 256);
        } else {
            printf("read return: %d\n", read(fileDescriptor, buff, (size_t)count));
            printf("Oh no! %s\n", strerror(errno));
        }
    } else {
        printf("Number of parameters dont match! %d\n", nparams);
        r.return_val = NULL;
        r.return_size = 0;
    }

    return r;
}

return_type fsWrite(const int nparams, arg_type *a) {
    if (nparams != 3) {
        r.return_val = NULL;
        r.return_size = 0;
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

    if (*write_result > -1) {
        r.return_val = (void *) &write_result;
        r.return_size = sizeof(int);
    } else {
        r.return_val = NULL;
        r.return_size = 0;
    }

    return r;
}

return_type fsRemove(const int nparams, arg_type *a) {
    if (nparams != 1) {
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }

    if (resource_in_use(a->arg_val) == 0) {
        int * remove_result = malloc(sizeof(int));
        *remove_result = remove(a->arg_val);

        r.return_val = (void *)&remove_result;
        r.return_size = sizeof(int);
    } else {
        r.return_val = NULL;
        r.return_size = 0;
    }

    return r;
}

return_type fsOpenDir(const int nparams, arg_type *a) {
    printf("The number of parameters: %d\n", nparams);
    if (nparams != 1) {
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }

    printf("Trying to open dir: %s\n", (char *)a->arg_val);

    DIR * dir = opendir(a->arg_val);

    if (dir == NULL) {
        r.return_val = NULL;
        r.return_size = 0;
    } else {
        int * id = malloc(sizeof(int));
        printf("ID: %p\n", id);
        *id = add_dir(dir);
        printf("ID: %p\n", id);
        printf("ID: %d\n", *id);

        r.return_val = (void *) id;
        r.return_size = sizeof(int);
    }

    return r;

}

return_type fsCloseDir(const int nparams, arg_type *a) {
    if (nparams != 1) {
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }

    struct dir_queue * dir = find_dir(*(int *)a->arg_val);

    if (dir == NULL) {
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }

    int * close_dir_result = malloc(sizeof(int));
    *close_dir_result = closedir(dir->dir);

    if (*close_dir_result == 0) {
        remove_dir(*(int *)a->arg_val);
    }

    r.return_val = (void *)close_dir_result;
    r.return_size = sizeof(int);
    return r;
}

return_type fsReadDir(const int nparams, arg_type *a) {
    if (nparams != 1) {
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }

    int id = *(int *) a->arg_val;
    struct dir_queue * dir = find_dir(id);
    if (dir == NULL) {
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }

    struct dirent * dir_info = readdir(dir->dir);
    printf("Directory info: %p\n", dir_info);
    printf("Dirent: %s\n", dir_info->d_name);
    if (dir_info == NULL) {
        r.return_val = NULL;
        r.return_size = 0;
    } else {
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