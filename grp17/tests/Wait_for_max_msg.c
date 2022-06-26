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
    OSMP_Barrier();
    test_OSMP_Size(&size);
    if(size != 17){
        printf("falsche Anzahl an Prozessen");
        exit(1);
    }

    int count = 3;
    char* msg = "lol";
    char* recv = malloc(sizeof(2000));
    int source;
    int len;

    if(rank != 0) {
        for (int i = 0; i < 16; ++i) {
            OSMP_Send(msg, count, OSMP_CHAR, rank);
        }
        OSMP_Barrier();
        if(rank == 1) {
            sleep(4);
            OSMP_Recv(recv, count, OSMP_CHAR, &source, &len);
        }
    }
    else {
        OSMP_Barrier();
        OSMP_Send(msg, count, OSMP_CHAR, rank);

    }
    //OSMP_Recv(recv, count, OSMP_CHAR, &source, &len);


}