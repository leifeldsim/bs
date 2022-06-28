#ifndef GRP17_OSMPLib_H
#define GRP17_OSMPLib_H

#include <semaphore.h>
#include "OSMPRun/OSMPRun.h"

typedef void* OSMP_Request;

// maximale Zahl der Nachrichten pro Prozess
#define OSMP_MAX_MESSAGES_PROC 16
// maximale Anzahl der Nachrichten, die insgesamt vorhanden sein dürfen
#define OSMP_MAX_SLOTS 256
// maximale Länge der Nutzlast einer Nachricht
#define OSMP_MAX_PAYLOAD_LENGTH 1024

#define OSMP_SUCCESS 0
#define OSMP_FAIL 1
#define OSMP_NO_FREE_SLOTS 1337
#define MAX_PROC 20
#define OSMP_INBOX_EMPTY 2
#define PROCESS_READY 1
#define PROCESS_NOT_READY 0
#define PROCESS_NOT_INITIALIZED -1

#define BARRIER_NOT_RUNNING 0
#define BARRIER_RUNNING 1

//Nachrichten Verkettung
#define NO_NEXT_MESSAGE -1

//OSMP_Request
#define OSMP_REQUEST_READY 1
#define OSMP_REQUEST_WORKING 2
#define OSMP_REQUEST_FINISHED 0
#define OSMP_REQUEST_ERROR -1

//OSMP_Bcast
#define NO_ROOT -1
#define OSMP_BROADCAST_RECEIVER -100

#define TEMP_LENGTH 1064464 //todo ändern

typedef enum {
    OSMP_SHORT=sizeof(short),
    OSMP_CHAR=sizeof(char),
    OSMP_INT=sizeof(int),
    OSMP_LONG=sizeof(long),
    OSMP_FLOAT=sizeof(float),
    OSMP_DOUBLE=sizeof(double),
    OSMP_BYTE=sizeof(char)
} OSMP_Datatype;

extern int OSMP_Init(int *argc, char ***argv);
/**
 * Die Routine liefert in *size die Zahl der OSMP-Prozesse ohne den OSMP-Starter Prozess zurück. Sollte
 * mit der Zahl übereinstimmen, die in der Kommandozeile dem OSMP-Starter übergeben wird.
 * @param size Zahl der OSMP-Prozesse (output)
 * @return
 */
int OSMP_Size(int *size);
int OSMP_Rank(int *rank);
int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest);
int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len);
int OSMP_Finalize(void);

int OSMP_Barrier();
int OSMP_Bcast(void *buf, int count, OSMP_Datatype datatype, int root);

int OSMP_Isend(const void *buf, int count, OSMP_Datatype datatype,int dest, OSMP_Request request);
int OSMP_Irecv(void *buf, int count, OSMP_Datatype datatype,int *source, int *len, OSMP_Request request);
int OSMP_Test(OSMP_Request request, int *flag);
int OSMP_Wait (OSMP_Request request);
int OSMP_CreateRequest(OSMP_Request* request);
int OSMP_RemoveRequest(OSMP_Request* request);

void add_proc(pid_t pid);
void write_into_message(int message_index, int sender, int receiver, OSMP_Datatype type, const void* buf, int buf_len);
void change_next_message_index(int message_index, int new_index_of_next_message);
int depr_is_free_slots_used();
int get_num_of_active_procs(int *count);

struct message{
    int sender;
    int receiver;
    OSMP_Datatype type;
    OSMP_Datatype buf[OSMP_MAX_PAYLOAD_LENGTH];
    int buf_len;
    int next_msg;//index
}message;

typedef struct {
    int status;
    sem_t mutex_is_working;
    pthread_t tid;

    // # Thread Argumente
    // Allgemein
    void* buffer;
    int count;
    OSMP_Datatype datatype;

    // für send
    int dest;

    // für receive
    int* source;
    int* len;
}OSMP_Request_t;

struct shared_memory{
    pid_t proc_ids[MAX_PROC];
    int process_ready[MAX_PROC];
    int count_procs;
    sem_t mutex_count_procs;
    struct message S[OSMP_MAX_SLOTS];
    sem_t message_mutex[OSMP_MAX_SLOTS];
    //int num_of_msg_per_proc[OSMP_MAX_MESSAGES_PROC];
    //int num_of_msg;

    //Bcast
    int root_rank;
    int bcast_msg_index;
    int bcast_error_code;
    sem_t mutex_is_free_to_read; //TODO anders initialisieren

    //Inbox
    int first_inbox[MAX_PROC];
    int last_inbox[MAX_PROC];
    sem_t mutex_inbox[MAX_PROC];
    sem_t full_inbox[MAX_PROC];
    sem_t empty_inbox[MAX_PROC];

    //Free_Slots
    int first_free;
    int last_free;
    sem_t mutex_empty_slots;
    sem_t full_empty_slots;
    sem_t empty_empty_slots;

    //Barrier
    pthread_barrierattr_t barrier_attr;
    pthread_barrier_t barrier;
    sem_t mutex_barrier_init;
    sem_t mutex_barrier_finalize;

    sem_t mutex_barrier_running;
    int barrier_running_count;
};

#endif
