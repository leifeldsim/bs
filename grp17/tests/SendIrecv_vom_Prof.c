/******************************************************************************
* FILE: osmp_SendIrecv.c
* DESCRIPTION:
*
OSMP program with a simple pair of OSMP_Send/OSMP_Irecv calls
*
* LAST MODIFICATION: Hans Effinger, March 01, 2022
******************************************************************************/
#include <stdio.h>

#include "OSMPLib/OSMPLib.h"
#define SIZE 50

int main(int argc, char *argv[])
{
    int rv, size, rank, source, len;
    char *bufin, *bufout;
    OSMP_Request myrequest = NULL;
    rv = OSMP_Init( &argc, &argv );
    rv = OSMP_Size( &size );
    rv = OSMP_Rank( &rank );
    if( size != 2 ) { /* Fehlerbehandlung */ }
    if( rank == 0 )
    { // OSMP process 0
        bufin = malloc(SIZE);
        len = 4;
        memcpy(bufin, "Test", len);
// check for != NULL
// length
        rv = OSMP_Send( bufin, len, OSMP_BYTE, 1 );
    }
    else
    { // OSMP process 1
        bufout = malloc(SIZE);
// check for != NULL
        rv = OSMP_CreateRequest( &myrequest );
        rv = OSMP_Irecv( bufout, SIZE, OSMP_BYTE, &source, &len, myrequest );
// do something important…
// check if operation is completed and wait if not
        rv = OSMP_Wait( myrequest );
// OSMP_Irecv() competed, use bufout
        rv = OSMP_RemoveRequest( &myrequest );
    }
    rv = OSMP_Finalize();
    return 0;
}