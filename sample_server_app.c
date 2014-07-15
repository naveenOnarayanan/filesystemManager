/* Mahesh V. Tripunitara
   University of Waterloo
   Part of a simplified RPC implementation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ece454rpc_types.h"

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
    r.return_val = (void*)isAlive;
    r.return_size = sizeof(int);
    return r;
}

return_type fsOpen(const int nparams, arg_type *a) {
    if (nparams == 2) {
        char * folderName = a->arg_val;
        int mode = *(int*)a->next->arg_val;

        printf("Folder Name: %s%s\n", getHostedFolder(), folderName);
        printf("Mode: %d\n", mode);

        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }
}

int main(int argc, char*argv[]) {

    if (argc == 2) {
        registerMountFolder(argv[1]);
        register_procedure("isAlive", 0, isAlive);
        register_procedure("fsOpen", 2, fsOpen);

#ifdef _DEBUG_1_
        printRegisteredProcedures();
#endif

        launch_server();
    }
    return 0;
}
