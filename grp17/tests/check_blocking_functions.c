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

    OSMP_Barrier();
    if (size < 2) {
        printf("Zu wenig Prozesse gestartet");
        exit(1);
    }


    for (int i = 0; i < 1000; ++i) {
        OSMP_Barrier();
    }

    int count = 50;
    int source;
    int len;
    int dst;
    char* recv = malloc((unsigned long) count);
    char *msg = calloc(sizeof(char), (unsigned long) count);
    printf("dest: %d\n", dst);
    sprintf(msg, "%d sendet an %d", rank, dst);

    for (int i = 0; i < 1000; ++i) {
        if (rank == size -1)
            dst = 0;
        else
            dst = rank + 1;

        if (OSMP_Send(msg, count, OSMP_CHAR, dst) == OSMP_FAIL) {
            printf("Beim Senden ist etwas schiefgelaufen");
        }
        OSMP_Barrier();
        if (OSMP_Recv(recv, count, OSMP_CHAR, &source, &len) == OSMP_FAIL) {
            printf("Beim Empfangen ist etwas schiefgelaufen");
        }

    }
    OSMP_Barrier();

    test_OSMP_Finalize();


    return 0;
}
