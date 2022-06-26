//
// Created by kevin on 17.06.22.
//


#ifndef PROJEKT_ERROR_HANDLING_OSMPLIB_H
#define PROJEKT_ERROR_HANDLING_OSMPLIB_H

void test_OSMP_Init(int argc, char **argv);
void test_OSMP_Rank(int *rank);
void test_OSMP_Size(int *size);
void test_OSMP_Finalize();
void test_OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest);
void test_enough_params(int argc);



#endif