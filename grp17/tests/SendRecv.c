#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "OSMPRun/OSMPRun.h"
#include <time.h>
#include <stdlib.h>
#include "OSMPLib/OSMPLib.h"
#include "tests/Error_handling_OSMPLib/error_handling_OSMPLib.h"


int getRandomDest(int rank, int size){
    srand(time(NULL));
    int randNum;
    int temp;
    do{
        printf("rand Size: %d\n", size);
        fflush(stdout);
        temp = (randNum = rand()%(size+1));
    }while(temp == rank);
    return randNum;
}

//argv[3] muss shm_name sein
int main(int argc,char *argv[]) {
    int rank;
    int size;

    test_enough_params(argc);
    test_OSMP_Init(argc, argv);
    test_OSMP_Rank(&rank);
    test_OSMP_Size(&size);
    printf("pid: %d: rank: %d, size = %d\n", getpid(), rank, size);

    int dst = getRandomDest(rank, size);
    int count = 50;
    char* recv = malloc((unsigned long) count);
    char *msg = calloc(sizeof(char), (unsigned long) count);
    printf("dest: %d\n", dst);
    sprintf(msg, "%d sendet an %d", rank, dst);
    printf("%s\n\n", msg);
    int src;
    int length;

    OSMP_Barrier();

    if (OSMP_Send(msg, count, OSMP_CHAR, dst) == OSMP_FAIL) {
        printf("OSMP_Send failed\n");
        return OSMP_FAIL;
    }

    OSMP_Barrier();

    int rv = OSMP_Recv(recv, count, OSMP_CHAR, &src, &length);
    if (rv == OSMP_FAIL) {
        printf("OSMP_Recv failed\n");
        return OSMP_FAIL;
    }
    else if (rv == OSMP_INBOX_EMPTY) {
        printf("Inbox war leer\n");
    }
    else{
        printf("Process %d received from %d: %s\n", rank, src, recv);
    }

    if(OSMP_Finalize() == OSMP_FAIL){
        printf("OSMP_Finalize Error");
    }

    return 0;
}
