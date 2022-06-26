/******************************************************************************
* FILE: osmp_SendRecv.c
* DESCRIPTION:
*
OSMP program with a simple pair of blocking OSMP_Send/OSMP_Recv calls
*
* LAST MODIFICATION: Hans Effinger, March 01, 2022
******************************************************************************/
#include <stdio.h>

#include "OSMPLib/OSMPLib.h"

int main(int argc, char *argv[])
{
    int rv, size, rank, source;
    int bufin[2], bufout[2], len;
    rv = OSMP_Init( &argc, &argv );
    rv = OSMP_Size( &size );
    rv = OSMP_Rank( &rank );
    if( size != 2 ) { /* Fehlerbehandlung */ }
    if( rank == 0 ){ // OSMP process 0
        bufin[0] = 4711;
        bufin[1] = 4712;
        rv = OSMP_Send( bufin, 2, OSMP_INT, 1 );
        if(rv == OSMP_FAIL){
            printf("OSMP_Send failed");
        }
    }
    else{ // OSMP process 1
        rv = OSMP_Recv( bufout, 2, OSMP_INT, &source, &len );
        if(rv == OSMP_FAIL){
            printf("OSMP_Recv failed");
        }else{
            printf("OSMP process %d received %d byte from %d [%d:%d] \n", rank, len, source, bufout[0], bufout[1]);
        }
    }
    rv = OSMP_Finalize();
    if(rv == OSMP_FAIL){
        printf("OSMP_Finalize failed");
    }
    return 0;
}