#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include "OSMPRun.h"
#include "OSMPLib/OSMPLib.h"
#include <semaphore.h>
#include <errno.h>
#include <pthread.h>

int proc_count;
struct shared_memory *test;

void create_processes(char* argv[]) {
    pid_t pid;
    for(int j = 1; j <= proc_count; j++){
        if((pid = fork()) == 0){
            if(pid == 0){
                add_proc(getpid());
                execv(argv[2], argv);
                printf("Exec Error\n");
            }
        }
    }
}

void add_proc(pid_t pid){
    if(sem_wait(&test->mutex_count_procs) == -1){
        perror("Error OSMPLib: sem_wait count_procs");
    }

    int index = test->count_procs;
    memcpy(&test->proc_ids[index], &pid, sizeof pid);
    test->count_procs = ++index;

    if(sem_post(&test->mutex_count_procs) == -1){
        perror("Error OSMPLib: sem_post count_procs");
    }
}

//argv[1] muss die Anzahl der gewuenschten Prozesse sein(int)
//argv[2] ist der KindProzess, also die ausführbare Datei (char*)
//argv[3] ist ein String der in den sharedMemory geschrieben wird (char*)
int main(int argc, char *argv[]){
    if(argc < 3){
        printf("falsche Parameterangaben");
        return -1;
    }

    char* shm_name = calloc(50, 1);
    pid_t pid = getpid();
    snprintf(shm_name, 50, "%s_%i", "/shm", pid);

    proc_count = atoi(argv[1]);
    if(proc_count < 1 || proc_count > MAX_PROC){
        printf("Anzahl zu startender Prozesse muss mindestens 1 und maximal %d\n", MAX_PROC);
        return -1;
    }
    length_shm = sizeof (struct shared_memory);

    printf("Length shm: %ld\n", length_shm);
    fflush(stdout);

    int fd_shmCreate = shm_open(shm_name, O_EXCL | O_CREAT | O_RDWR, 0640);
    if(fd_shmCreate == -1){
        printf("Parent Error: shm_open\n");
        return -1;
    }

    printf("name: %s, fd: %d\n", shm_name, fd_shmCreate);

    if(ftruncate(fd_shmCreate, length_shm) == -1){
        printf("Parent Error: ftruncate\n");
        return -1;
    }
    printf("mmap called with len: %ld, fd: %d\n", length_shm, fd_shmCreate);
    test = (struct shared_memory*) mmap(NULL, (size_t) length_shm, PROT_WRITE | PROT_READ, MAP_SHARED, fd_shmCreate, 0);

    if(test == MAP_FAILED){
        printf("Parent Error: MMAP failed\n");
        return -1;
    }

    test->count_procs = 0;
    init_shm(test);

    strcpy(argv[3], shm_name);

    create_processes(argv);

    while(0 < wait(NULL));

    if(shm_unlink(shm_name) == -1){
        printf("Parent Error: unlink failed(name: %s): %s\n", shm_name, strerror(errno));
        return -1;
    }
    exit(0);
}

int error(char* msg){
    printf("Parent Error: %s\n", msg);
    fflush(stdout);
    return OSMP_FAIL;
}

int init_shm(struct shared_memory* shm){

    //== 0 -> blockieren
    //!=0 -> nicht blockieren
    for(int i = 0; i < MAX_PROC; i++){
        if(sem_init(&shm->mutex_inbox[i], 1, 1) == -1) {return error("sem_init mutex_inbox");}
        if(sem_init(&shm->empty_inbox[i], 1, OSMP_MAX_MESSAGES_PROC) == -1) {return error("sem_init empty_inbox");}
        if(sem_init(&shm->full_inbox[i], 1, 0) == -1) {return error("sem_init full_inbox");}
    }

    for(int i = 0; i < OSMP_MAX_SLOTS; i++){
        if(sem_init(&shm->message_mutex[i], 1, 1) == -1) {return error("sem_init message_mutex");}
    }
    for (int i = 0; i < MAX_PROC; i++) {
        shm->first_inbox[i] = NO_NEXT_MESSAGE;
        shm->last_inbox[i] = NO_NEXT_MESSAGE;
        shm->process_ready[i] = PROCESS_NOT_INITIALIZED;
    }

    // Nur bis OSMP_MAX_SLOTS-1, da letzte Nachricht nicht mitverkettet wird
    for (int i = 0; i < OSMP_MAX_SLOTS - 1; i++) {
        shm->S[i].next_msg = i + 1;
    }
    shm->S[OSMP_MAX_SLOTS - 1].next_msg = NO_NEXT_MESSAGE;

    shm->first_free = 0;
    shm->last_free = OSMP_MAX_SLOTS - 1;

    shm->root_rank = NO_ROOT;

    if(sem_init(&shm->mutex_empty_slots, 1, 1)) {return error("sem_init mutex_empty_slots");}
    if(sem_init(&shm->empty_empty_slots, 1, OSMP_MAX_SLOTS)) {return error("sem_init empty_empty_slots");}
    if(sem_init(&shm->full_empty_slots, 1, 0)) {return error("sem_init full_empty_slots");}


    if(sem_init(&shm->mutex_count_procs, 1, 1)) {return error("sem_init mutex_count_procs");}

    if(sem_init(&shm->mutex_is_free_to_read, 1, 1)) {return error("sem_init bcast_seen_msg");}
    if(sem_init(&shm->mutex_barrier_init, 1, 1)) {return error("sem_init bcast_seen_msg");}
    if(sem_init(&shm->mutex_barrier_running, 1, 1)) {return error("sem_init bcast_seen_msg");}
    if(sem_init(&shm->mutex_barrier_finalize, 1, 1)) {return error("sem_init bcast_seen_msg");}

    pthread_barrierattr_t barrier_attr;
    pthread_barrierattr_init(&barrier_attr);
    pthread_barrierattr_setpshared(&barrier_attr, PTHREAD_PROCESS_SHARED);
    pthread_barrier_init(&shm->barrier, &barrier_attr, (unsigned int) proc_count);
    shm->barrier_running_count = 0;

    return OSMP_SUCCESS;
}

