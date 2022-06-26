#ifndef DEFAULT_LIBRARY_H
#define DEFAULT_LIBRARY_H

#include <semaphore.h>
#include "OSMPLib/OSMPLib.h"
int main(int argc, char *argv[]);
void create_processes(char* argv[]);
int init_shm(struct shared_memory* shm);
void create_processes(char* argv[]);
off_t length_shm;
int error(char* msg);
#endif
