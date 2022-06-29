//
// Created by kevin on 29.06.22.
//

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
    int count = 5;
    char *msg = calloc(sizeof(char), (unsigned long) count);
    msg = "hello";
    OSMP_Send(msg, count, OSMP_CHAR, rank);

    int source;
    int len;
    if(OSMP_Recv(msg, count, OSMP_INT, &source, &len) == OSMP_FAIL) {
        printf("\t\t\t recv hat im Test den falschen Datentypen erkannt\n");
    }
    test_OSMP_Finalize();


    return 0;
}
