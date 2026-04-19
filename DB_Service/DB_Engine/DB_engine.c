#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

#include "DB_engine.h"
#include "../structs.h"
#include "DB_File_IO_Handler.h"

int msgid_request, msgid_reply;
struct dbinfo db_info;

pthread_mutex_t db_mutex_db_info = PTHREAD_MUTEX_INITIALIZER;





void* worker_thread(void* args){
    
    int my_id = *((int*)args); 
    printf("DB_SERVICE: Worker thread %d started\n", my_id);

}



int main(){
    
    // get the msg q
    key_t key = ftok("DB_engine.c", 65);
    
    msgid_request = msgget(key, 0666 | IPC_CREAT);
    if (msgid_request == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }
    
    msgid_reply = msgget(key + 1, 0666 | IPC_CREAT);
    if (msgid_reply == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }



    // initialize the db
    if(db_init(&db_info) == FAILURE){
        fprintf(stderr, "Failed to initialize the database\n");
        exit(EXIT_FAILURE);
    }



    pthread_t threads[DB_MAX_THREADS];
    int ids_threads[DB_MAX_THREADS];

    for(int i = 0; i < DB_MAX_THREADS; i++){
        ids_threads[i] = i+1;
        if(pthread_create(&threads[i], NULL, worker_thread, &ids_threads[i]) != 0){
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }


    for(int i = 0; i < DB_MAX_THREADS; i++){
        pthread_join(threads[i], NULL);
    }


    return 0;
}
