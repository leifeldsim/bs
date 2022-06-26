#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "OSMPRun/OSMPRun.h"
#include <time.h>
#include <stdlib.h>
#include "OSMPLib/OSMPLib.h"

//argv[3] muss shm_name sein
int main(int argc,char *argv[]) {
    if (argc < 3){
        return -1;
    }

    int preRank;
    OSMP_Rank(&preRank);
    printf("preRank: %d\n", preRank);

    if(OSMP_Init(&argc, &argv) == OSMP_FAIL){
        printf("Error OSMP_Init\n");
        fflush(stdout);
        return -1;
    }

    int rank;
    if(OSMP_Rank(&rank) == OSMP_FAIL){
        printf("Error OSMP_Rank\n");
        fflush(stdout);
        return -1;
    }

    int size;
    if(OSMP_Size(&size) == OSMP_FAIL){
        printf("Error OSMP_Size\n");
        fflush(stdout);
        return -1;
    }
    printf("pid: %d: rank: %d, size = %d\n", getpid(), rank, size);


    if(OSMP_Finalize() == OSMP_FAIL){
        printf("OSMP_Finalize Error");
    }

    int postRank;
    OSMP_Rank(&postRank);
    printf("postRank: %d\n", postRank);
    fflush(stdout);

    return 0;
}
