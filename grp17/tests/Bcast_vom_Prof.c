/******************************************************************************
* FILE: osmp_Bcast.c
* DESCRIPTION:
*
OSMP program with simple OSMP_Bcast call
*
* LAST MODIFICATION: Hans Effinger, March 01, 2022
******************************************************************************/
#include <stdio.h>

#include "OSMPLib/OSMPLib.h"
#define EINGEFUEGTE_ZAHL 10


int main(int argc, char *argv[])
{
    int rv, size, rank;
    char buffer[OSMP_MAX_PAYLOAD_LENGTH], count;
    rv = OSMP_Init( &argc, &argv );
    rv = OSMP_Size( &size );
    rv = OSMP_Rank( &rank );
    if( size < 2 ) { /* Fehlerbehandlung */ }
    if( rank == 1 )
    { // OSMP process 1, this is the broadcasting ”root”
        count = EINGEFUEGTE_ZAHL ;
        memcpy(buffer, "EINGEFUEGT" , count);
// number of bytes
// copy data to buffer and broadcast …
        rv = OSMP_Bcast( buffer, count, OSMP_BYTE, 1 );
    }
    else
    { // all other OSMP processes receive the message
        count = EINGEFUEGTE_ZAHL ;
// number of bytes
        rv = OSMP_Bcast( buffer, count, OSMP_BYTE, 1 );
// use data in buffer
    }
    rv = OSMP_Finalize();
    return 0;
}