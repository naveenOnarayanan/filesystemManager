/* Mahesh V. Tripunitara
   University of Waterloo
   Part of a simplified RPC implementation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ece454rpc_types.h"

#if 0
#define _DEBUG_1_
#endif

/* We allocate a global variable for returns. However, 
 * r.return_val is allocated dynamically. We look to
 * the server_stub to deallocate after it returns the
 * response to the client. */

return_type r;

return_type isMounted() {
    r.return_size = sizeof(int);
    r.return_val = (void *)1;
    return r;
}


int main(int argc, char *argv[]) {
    register_procedure("isMounted", 0, isMounted);

    if (argc == 2) {
        setServingDirectory(argv[1]);
    }

#ifdef _DEBUG_1_
    printRegisteredProcedures();
#endif

    launch_server();
    return 0;
}
