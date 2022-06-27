#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "OSMPRun/OSMPRun.h"
#include <time.h>
#include <stdlib.h>
#include "OSMPLib/OSMPLib.h"
#include "tests/Error_handling_OSMPLib/error_handling_OSMPLib.h"

//argv[3] muss shm_name sein
int main(int argc,char *argv[]) {
    int rank;
    int size;

    test_enough_params(argc);
    test_OSMP_Init(argc, argv);
    test_OSMP_Rank(&rank);
    test_OSMP_Size(&size);
    printf("pid: %d: rank: %d, size = %d\n", getpid(), rank, size);

    if(rank == 0){
        test_OSMP_Finalize();
        return 0;
    }

    OSMP_Barrier();

    test_OSMP_Finalize();

    return 0;
}
