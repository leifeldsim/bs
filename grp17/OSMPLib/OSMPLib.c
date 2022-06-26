#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "OSMPLib.h"
#include "semaphore.h"
#include <errno.h>
#include <pthread.h>
#include <sys/wait.h>

#define SLEEPTIMER 10

char* shm_name;
int procs_to_start;
struct shared_memory *shm;


int check_message_len(int count, OSMP_Datatype datatype) {
    if ((unsigned int) count * datatype > OSMP_MAX_PAYLOAD_LENGTH){
      printf("Die angegebene Laenge ist nicht zulässig");
      return OSMP_FAIL;
    }
    return OSMP_SUCCESS;
}

void debug_print(char specification[], char value[]) {
    int rank;
    OSMP_Rank(&rank);
    printf("Prozess %d:\t", rank);
    printf("%s%s\n", specification, value);
}




//Feststellen, ob Request bereit zum Arbeiten ist
int check_request(OSMP_Request_t* request) {
    if (request == NULL) {
        fprintf(stderr, "Request is null\n");
        return OSMP_FAIL;
    }

    int status = request->status;
    if (status != OSMP_REQUEST_READY) {
        if (status == OSMP_REQUEST_FINISHED) printf("Request wurde noch nicht ausgelesen\n");
        if (status == OSMP_REQUEST_ERROR) printf("Thread konnte seine Aufgabe nicht erledigen\n");
        if (status == OSMP_REQUEST_WORKING) printf("Thread ist noch am arbeiten\n");

        return OSMP_FAIL;
    }
    return OSMP_SUCCESS;
}





int check_non_reachable_rank(int proc_rank) {
    int num_of_procs;
    OSMP_Size(&num_of_procs);

    if (proc_rank < 0 || proc_rank >= num_of_procs) {
        fprintf(stderr, "Prozess %d ist nicht vorhanden\n", proc_rank);
        return OSMP_FAIL;
    }
//    else if (shm->process_ready[proc_rank] == PROCESS_NOT_READY) {
//        printf("Prozess %d ist im moment nicht aktiv", proc_rank);
//        return OSMP_FAIL;
//    }
    return OSMP_SUCCESS;
}




int OSMP_Init(int *argc, char ***argv) {
    procs_to_start = atoi(argv[0][1]);
    shm_name = argv[0][3];

    int file_descriptor = shm_open(shm_name, O_RDWR, 0640);
    if(file_descriptor == -1){
        printf("Error shm_open(name: %s): %s\n", shm_name, strerror(errno));
        fflush(stdout);
        return OSMP_FAIL;
    }

    shm = (struct shared_memory*) mmap(NULL, TEMP_LENGTH, PROT_WRITE | PROT_READ, MAP_SHARED, file_descriptor, 0);
    if(shm == MAP_FAILED){
        printf("Error mmap: %s\n", strerror(errno));
        fflush(stdout);
        return OSMP_FAIL;
    }

    printf("shm %s opened\n", shm_name);
    fflush(stdout);

    //set process to ready
    int rank;
    OSMP_Rank(&rank);
    shm->process_ready[rank] = PROCESS_READY;

    printf("Prozess %d hat Init aufgerufen\n", rank);

    int active;

    //TODO brauchen wir nicht mehr?
    //if(active != 1){
    //    sem_post(&shm->empty_barrier);
    //}

    get_num_of_active_procs(&active);
    return OSMP_SUCCESS;
}




int OSMP_Size(int *size) {
    if(shm == NULL){
        return OSMP_FAIL;
    }
    *size = shm->count_procs;
    return OSMP_SUCCESS;
}




int OSMP_Rank(int *rank) {
    if(shm == NULL){
        return OSMP_FAIL;
    }
    for(int i = 0; i < shm->count_procs; i++){
        if(shm->proc_ids[i] == getpid()){
            *rank = i;
            return OSMP_SUCCESS;
        }
    }
    return OSMP_FAIL;
}





int get_message_from_empty_slots(int* empty_message_index) {
    sem_wait(&shm->empty_empty_slots);
    sem_wait(&shm->mutex_empty_slots);

    int message_index = shm->first_free;
    if (message_index == -1) {      // Keine Nachrichten können versendet werden
        printf("Keine freien Nachrichtenslots mehr vorhanden");
        printf("Fehler bei empty_empty_slots");
        return OSMP_NO_FREE_SLOTS;
    }
    else if (message_index == shm->last_free) {     // Nur noch ein freier Nachrichtenslot offen
        shm->first_free = -1;
        shm->last_free = -1;
    }
    else
        shm->first_free = shm->S[message_index].next_msg;
    sem_post(&shm->mutex_empty_slots);
    sem_post(&shm->full_empty_slots);

    *empty_message_index = message_index;

    return OSMP_SUCCESS;
}


int add_message_to_empty_slots(int message_index) {
    sem_wait(&shm->full_empty_slots);
    sem_wait(&shm->mutex_empty_slots);
    // Freie Felder verwalten
    if (shm->first_free == -1) {        // Keine freien Nachrichten mehr
        shm->first_free = message_index;
        shm->last_free = message_index;
    }
    else {
        change_next_message_index(shm->last_free, message_index);
        shm->last_free = message_index;
    }
    sem_post(&shm->mutex_empty_slots);
    sem_post(&shm->empty_empty_slots);
    return OSMP_SUCCESS;
}




int read_msg_to_buf(void *buf, int message_index, int count) {
    printf("message_index: %d\n", message_index);
    printf("Count: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("%d\n\n", i);
        //printf("%s hellooo\n", (char*) shm->S[message_index].buf[i]);
        ((OSMP_Datatype*) buf)[i] = shm->S[message_index].buf[i];
    }
    debug_print("read_msg_to_buf funktioniert", "");

    return OSMP_SUCCESS;
}



int move_first_inbox_to_free_slots(int inbox_index) {

    int message_index = shm->first_inbox[inbox_index];
    sem_wait(&shm->mutex_inbox[inbox_index]);
    if (shm->first_inbox[inbox_index] == shm->last_inbox[inbox_index]) {      // Nur eine Nachricht in der Inbox
        shm->first_inbox[inbox_index] = -1;
        shm->last_inbox[inbox_index] = -1;
    }
    else {
        shm->first_inbox[inbox_index] = shm->S[message_index].next_msg;
    }
    sem_post(&shm->mutex_inbox[inbox_index]);

    add_message_to_empty_slots(message_index);
    sem_post(&shm->full_inbox[inbox_index]);
    sem_wait(&shm->full_inbox[inbox_index]);
}





int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest) {
    printf("OSMP_Send called with params: count: %d dest: %d\n", count, dest);
    fflush(stdout);

    //Fehlerfälle
    if (check_message_len(count, datatype) == OSMP_FAIL) return OSMP_FAIL;
    if (check_non_reachable_rank(dest) == OSMP_FAIL) return OSMP_FAIL;

    int message_index;
    if (get_message_from_empty_slots(&message_index) == OSMP_NO_FREE_SLOTS) return OSMP_FAIL;
    debug_print("Hat sich freie Nachricht geholt", "");
    int index_last_message_of_dest = shm->last_inbox[dest];


    // Freie Nachrichtenfelder bearbeiten
    //TODO MAX 16 Nachrichten pro Inbox
    //Hier kein blockieren
    int rank;
    OSMP_Rank(&rank);
    write_into_message(message_index, rank, dest, datatype, buf, count);

    // Receiver Inbox neu verbinden
    sem_wait(&shm->empty_inbox[dest]);
    if (shm->first_inbox[dest] == -1) {     // Keine Nachrichten in der Inbox
        shm->first_inbox[dest] = message_index;
    }
    else {      // Nachrichten in der Inbox vorhanden
        change_next_message_index(index_last_message_of_dest, message_index);
    }
    change_next_message_index(message_index, NO_NEXT_MESSAGE);    //nicht unbedingt nötig - Zu besseren Leserlichkeit
    shm->last_inbox[dest] = message_index;
    sem_post(&shm->full_inbox[dest]);

    return OSMP_SUCCESS;
}



int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len) {
    // Rank besorgen
    int recv_index;
    OSMP_Rank(&recv_index);
    if (check_message_len(count, datatype) == OSMP_FAIL) return OSMP_FAIL;


    // Blockieren bei leerer Inbox, bis Nachricht kommt
    if (shm->first_inbox[recv_index] == -1) {
        printf("Prozess %d: Inbox ist noch leer \n", recv_index);
        return OSMP_INBOX_EMPTY;
    }

    int message_index = shm->first_inbox[recv_index];

    if (shm->S[message_index].type != datatype) {
        printf("Der Datentyp der Nachricht stimmt nicht mit dem angegebenen Datentypen überein");
        return OSMP_FAIL;
    }

    printf("####### Nachricht erhalten\n");
    printf("Sender: %d\n", shm->S[message_index].sender);
    printf("Empfänger: %d\n", shm->S[message_index].receiver);
    printf("Tatsächlicher Empfänger %d\n", recv_index);
//    printf("Nachricht: %s\n", (char*)shm->S[message_index].buf); //funktioniert

    *source = shm->S[message_index].sender;

    read_msg_to_buf(buf, message_index, count);

    // Inbox verwalten
    move_first_inbox_to_free_slots(recv_index);

    return OSMP_SUCCESS;
}


int OSMP_Bcast(void *buf, int count, OSMP_Datatype datatype, int root){
    int msg_index;


    int rank;
    OSMP_Rank(&rank);
    printf("Prozess %d:\tBcast opened\n", rank);

    if (rank == root) {
        debug_print("is root", "");
        sem_wait(&shm->mutex_is_free_to_read);

        //Fehlerfälle abgreifen
        shm->bcast_error_code = 0;
        if(shm->root_rank != NO_ROOT) shm->bcast_error_code = 1;
        if (get_message_from_empty_slots(&msg_index) == OSMP_NO_FREE_SLOTS) {
            shm->bcast_error_code = 2;
        }
        if (check_message_len(count, datatype) == OSMP_FAIL) shm->bcast_error_code = 3;

        //Nachricht erstellen, wenn es keine Fehler gab
        if (shm->bcast_error_code == 0) {

            //Neue message reinholen


            shm->root_rank = root;
            shm->bcast_msg_index = msg_index;

            int buf_len = (int) ((unsigned int) count * datatype);
            debug_print("msg:", (char*) buf);
            write_into_message(msg_index, root, OSMP_BROADCAST_RECEIVER, datatype, buf, buf_len);

        }

        sem_post(&shm->mutex_is_free_to_read);
        OSMP_Barrier();
        debug_print("1. Barrier überstanden", "");
        switch (shm->bcast_error_code) {
            case 1:
                printf("Broadcast wird gerade verwendet");
                return OSMP_FAIL;
            case 2:
                return OSMP_NO_FREE_SLOTS;
            case 3:
                return OSMP_FAIL;
            default:
                break;
        }
        OSMP_Barrier();
        debug_print("2. Barrier überstanden", "");


        //Bcast wieder cleanen
        sem_wait(&shm->mutex_is_free_to_read);
        shm->root_rank = NO_ROOT;
        sem_post(&shm->mutex_is_free_to_read);


        add_message_to_empty_slots(msg_index);

    }

    // Teil von Lesern
    else {
        debug_print("not root", "");
        OSMP_Barrier();
        //Sicherstellen, dass niemand zu früh anfängt zu lesen
        debug_print("1. Barrier überstanden", "");

        read_msg_to_buf(buf, shm->bcast_msg_index, count);
        debug_print("Buffer: ", buf);
        //printf("buffer: %s, buflen: %d ", (char*) shm->S[msg_index].buf, shm->S[msg_index].buf_len);
        debug_print("Lesen geschafft", "");

        OSMP_Barrier();
        debug_print("2. Barrier überstanden", "");

        /*Gucken dass nicht grade noch beschrieben wird
        FRAGE: Wofür braucht man count und datatype als Leser*/
    }
    return OSMP_SUCCESS;
}




int OSMP_CreateRequest(OSMP_Request* request){
    *request = malloc(sizeof(OSMP_Request_t));
    OSMP_Request_t *req = (OSMP_Request_t*) *request;
    req->status = OSMP_REQUEST_READY;
    sem_init(&req->mutex_is_working, 0, 1);

    return OSMP_SUCCESS;
}




int OSMP_RemoveRequest(OSMP_Request* request){
    OSMP_Request_t *req = (OSMP_Request_t*) *request;
    if (req->status == OSMP_REQUEST_WORKING) {
        return OSMP_FAIL;
    }
    free(req);
    return OSMP_SUCCESS;
}





void thread_set_working(OSMP_Request_t* request){
    request->status = OSMP_REQUEST_WORKING;
    sem_wait(&request->mutex_is_working);
}





void thread_set_finished(OSMP_Request_t* request, int return_value){
    if (return_value == OSMP_FAIL){
        request->status = OSMP_REQUEST_ERROR;
    }else{
        request->status = OSMP_REQUEST_FINISHED;
    }
    sem_post(&request->mutex_is_working);
}


void* thread_send(void* request) {

    printf("Request als Hex(in Funktion): %x\n", request);

    OSMP_Request_t* req = (OSMP_Request_t*) request;

    printf("Request als Hex(nach Cast): %x\n", req);

    printf("Thread send started\n");
    printf("\t\t\t request_dest: %d\n", req->dest);
    fflush(stdout);
    thread_set_working(req);

    int rv = OSMP_Send(req->buffer, req->count,req->datatype, req->dest);
    if(rv == OSMP_FAIL){
        printf("OSMP_Send in thread_send failed\n");
    }
    thread_set_finished(req, rv);
    return NULL;
}


void* thread_recv(void* request) {

    OSMP_Request_t* req = (OSMP_Request_t*) request;
    thread_set_working(req);

    int rv = OSMP_Recv(req->buffer, req->count, req->datatype, req->source, req->len);

    if(rv == OSMP_FAIL){
        printf("OSMP_Send in thread_send failed\n");
    }

    thread_set_finished(req, rv);
    return NULL;
}


int OSMP_Isend(const void *buf, int count, OSMP_Datatype datatype, int dest, OSMP_Request request) {

    OSMP_Request_t* req = (OSMP_Request_t*) request;

    if (check_request(request) == OSMP_FAIL) return OSMP_FAIL;
    if (check_message_len(count, datatype) == OSMP_FAIL) return OSMP_FAIL;
    if (check_non_reachable_rank(dest) == OSMP_FAIL) return OSMP_FAIL;

    //Request beschreiben
    req->buffer = buf;
    req->count = count;
    req->datatype = datatype;
    req->dest = dest;

    printf("Request als Hex: %x\n", req);

    printf("reqbuffer:%s buf:%s reqdest: %d dest: %d\n", (char*) req->buffer, (char*) buf, req->dest, dest);
    fflush(stdout);
    pthread_t tid;

    //thread_send(req);

    if (pthread_create(&tid, NULL, thread_send, req) != 0){
        printf("pthread_create macht Schwierigkeiten\n");
        fflush(stdout);
        return OSMP_FAIL;
    }
    req->tid = tid;

    //TODO linstert?
    return OSMP_SUCCESS;
}

int OSMP_Irecv(void *buf, int count, OSMP_Datatype datatype,int *source, int *len, OSMP_Request request){
    OSMP_Request_t* req = (OSMP_Request_t*) request;

    if (check_request(request) == OSMP_FAIL) return OSMP_FAIL;

    req->count = count;
    req->datatype = datatype;
    source = req->source;
    len = req->len;
    pthread_t tid;


    int return_value = pthread_create(&tid, NULL, thread_recv, req);
    if (return_value != 0) {
        fprintf(stderr, "pthread_create macht Schwierigkeiten\n");
        return OSMP_FAIL;
    }

    req->tid = tid;
    return OSMP_SUCCESS;

}

int remove_all_messages_from_inbox(int inbox_index) {
    int rv;
    //printf("Prozess %d: Vor dem Removen von allen Nachrichten ist die Inbox bei %d\n", inbox_index, shm->first_inbox[inbox_index]);

    while (shm->first_inbox[inbox_index] > -1) {
        rv = move_first_inbox_to_free_slots(inbox_index);
        if (rv != OSMP_SUCCESS) return rv;
    }
    //printf("Prozess %d: Nach dem Removen von allen Nachrichten ist die Inbox bei %d\n", inbox_index, shm->first_inbox[inbox_index]);


    return OSMP_SUCCESS;
}


int OSMP_Finalize(void) {
    int rank;
    OSMP_Rank(&rank);
//    remove_all_messages_from_inbox(rank);
    shm->process_ready[rank] = PROCESS_NOT_READY;
    int num_ready_procs;
    get_num_of_active_procs(&num_ready_procs);



    if(munmap(shm, TEMP_LENGTH) == -1){
        printf("Error child: munmap");
        return OSMP_FAIL;
    }
    //TODO Für Barrier anzahl an prozessen neuinitialisieren (pthread_barrier)
    printf("Prozess %d hat finalize aufgerufen\n", rank);
    return OSMP_SUCCESS;
}


int OSMP_Test(OSMP_Request request, int *flag){
    //todo status von request in flag schreiben und dirket returnen
    OSMP_Request_t* req = (OSMP_Request_t*) request;
    *flag = req->status;
    return OSMP_SUCCESS;
}

int OSMP_Wait (OSMP_Request request){
    //todo solange warten bis request.status finished ist und danach returnen
    OSMP_Request_t* req = (OSMP_Request_t*) request;
    sem_wait(&req->mutex_is_working);
    sem_post(&req->mutex_is_working);
    return OSMP_SUCCESS;
}


int OSMP_Barrier() {
    int rank;
    OSMP_Rank(&rank);
    printf("\t\tProzess %d started barrier\n", rank);

//    printf("barrier value: %s\n", pthread_barr);

    pthread_barrier_wait(&shm->barrier);

    printf("\t\t\t\tProzess %d finished barrier\n", rank);

    return OSMP_SUCCESS;
}


//---------------------------Hilfsfunktionen-Start------------------------------------------------------------------------------

struct message* new_message(){
    struct message* msg = malloc(sizeof(struct message));
    if(msg == NULL){
        printf("Error malloc/new_message");
        exit(-1);
    }
    return msg;
}

void change_next_message_index(int message_index, int new_index_of_next_message) {
    shm->S[message_index].next_msg = new_index_of_next_message;
}

void write_into_message(int message_index, int sender, int receiver, OSMP_Datatype type, const void* buf, int buf_len) {
    shm->S[message_index].sender = sender;
    shm->S[message_index].receiver = receiver;
    shm->S[message_index].type = type;
    shm->S[message_index].buf_len = buf_len;
//    shm->S[message_index].buf = (OSMP_Datatype*) buf;
    for (int i = 0; i < buf_len; i++) {
        shm->S[message_index].buf[i] = ((OSMP_Datatype*) buf)[i];
    }
    //shm->S[message_index].buf = buf;
}
/**
 * Darf nicht innerhalb des Mutex "mutex_empty_slots" da es sonst zum stocken kommt
 * @return Wahrheitswert der Funktion
 */
int depr_is_free_slots_used() {
    sem_wait(&shm->mutex_empty_slots);
    int message_index = shm->first_free;
    sem_post(&shm->mutex_empty_slots);
    if (message_index == -1) {      // Keine Nachrichten können versendet werden
        printf("Keine freien Nachrichtenslots mehr vorhanden");
        return 1;
    }
    return 0;
}


int get_num_of_active_procs(int *count){
    for(int i = 0; i < MAX_PROC; i++){
        *count += shm->process_ready[i];
    }
}


//---------------------------Hilfsfunktionen-Ende-------------------------------------------------------------------------------