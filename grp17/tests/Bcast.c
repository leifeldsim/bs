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

    int count = 50;
    char* recv = malloc((unsigned long) count);
    char *msg = malloc((unsigned long) count);
    msg = "Testmsg";
    if(rank == 0){//write
        printf("das ist die nachricht vor dem bcast: %s\n", msg);
        if(OSMP_Bcast(msg, strlen(msg) , OSMP_CHAR, 0) == OSMP_FAIL)
            fprintf(stderr, "OSMP_Bcast: (root) unexpected failure\n");
    } else{//read
        if(OSMP_Bcast(recv, strlen(msg) , OSMP_CHAR, 0) == OSMP_FAIL){
            fprintf(stderr, "OSMP_Bcast: unexpected failure\n");
        }else{
            if (recv == NULL)
                printf("moin meister \n");
            printf("Process %d received from Bcast: %s\n", rank, recv);

        }
    }



    if(OSMP_Finalize() == OSMP_FAIL){
        printf("OSMP_Finalize Error");
    }

    return 0;
}

