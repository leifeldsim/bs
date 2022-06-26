#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "OSMPRun/OSMPRun.h"
#include <time.h>
#include <stdlib.h>
#include "OSMPLib/OSMPLib.h"


int getRandomDest(int rank, int size){
    srand(time(NULL));
    int randNum;
    int temp;
    do{
        temp = (randNum = rand()%(size+1));
    }while(temp == rank || temp == size);
    return randNum;
}

//argv[3] muss shm_name sein
int main(int argc,char *argv[]) {
    if (argc < 3){
        return -1;
    }

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

    int dst = getRandomDest(rank, size);
    int count = 50;
    char* recv = malloc((unsigned long) count);
    char *msg = calloc(sizeof(char), (unsigned long) count);
    printf("dest: %d\n", dst);
    sprintf(msg, "%d sendet an %d", rank, dst);
    printf("%s\n\n", msg);
    int src;
    int length;

    OSMP_Request request;
    OSMP_CreateRequest(&request);

    OSMP_Request_t* req = (OSMP_Request_t*) request;

    if(req == NULL){
        printf("Error requestcreate\n");
        fflush(stdout);
    }

    if(req->status != OSMP_REQUEST_READY){
        printf("Error request status\n");
        fflush(stdout);
    }

    if (OSMP_Isend(msg, count, OSMP_CHAR, dst, req) == OSMP_FAIL) {
        printf("OSMP_ISend failed\n");
        return OSMP_FAIL;
    }

    OSMP_Barrier();

    if (OSMP_Irecv(recv, count, OSMP_CHAR, &src, &length, req) == OSMP_FAIL) {
        printf("OSMP_IRecv failed\n");
        return OSMP_FAIL;
    }
//    OSMP_Wait(request);

    printf("Process %d received from %d: %s\n", rank, *req->source, (char*) req->buffer);

    OSMP_RemoveRequest(request);

    if(OSMP_Finalize() == OSMP_FAIL){
        printf("OSMP_Finalize Error");
    }

    return 0;
}
