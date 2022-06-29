//
// Created by kevin on 17.06.22.
//

#include "OSMPLib/OSMPLib.h"
#include <stdio.h>
#include <stdlib.h>

void test_OSMP_Init(int argc, char **argv) {
    if(OSMP_Init(&argc, &argv) == OSMP_FAIL){
        printf("Error OSMP_Init\n");
        fflush(stdout);
        exit(-1);
    }
}

void test_OSMP_Rank(int *rank) {
    if(OSMP_Rank(rank) == OSMP_FAIL){
        printf("Error OSMP_Rank\n");
        fflush(stdout);
        exit(-1);
    }
}

void test_OSMP_Size(int *size) {
    if(OSMP_Size(size) == OSMP_FAIL){
        printf("Error OSMP_Size\n");
        fflush(stdout);
        exit(-1);
    }
}

void test_OSMP_Finalize() {
    if(OSMP_Finalize() == OSMP_FAIL){
        printf("OSMP_Finalize Error");
    }
}

void test_OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest) {
    if (OSMP_Send(buf, count, datatype, dest) == OSMP_FAIL) {
        printf("OSMP_Send failed\n");
    }
}

void test_enough_params(int argc) {
    if (argc < 3){
        exit(-1);
    }
}