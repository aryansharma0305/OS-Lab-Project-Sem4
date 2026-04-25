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


// =========================================================================================

void* worker_thread(void* args) {

    int my_id = *((int*)args);
    printf("DB_SERVICE: Worker thread %d started\n", my_id);

    struct msg_request  req;
    struct msg_response reply;

    while (1) {

        // Block waiting for any request (mtype=1)
        if (msgrcv(msgid_request, &req,
                   sizeof(struct msg_request) - sizeof(long), 1, 0) == -1) {
            perror("DB_SERVICE: msgrcv failed");
            continue;
        }

        printf("DB_SERVICE: Thread %d picked up request reply_to=%ld op=%d db_id=%d\n",
               my_id, req.reply_to, req.operation, req.db_id);

        // Clear reply and set mtype so client can filter its own reply
        memset(&reply, 0, sizeof(reply));
        reply.msg_type = req.reply_to;

        char error_msg[256] = {0};

        switch (req.operation) {

            case DB_OP_INSERT: {
                int assigned_key = -1;
                reply.status = db_insert(&db_info, req.db_id, &req.payload,
                                        &assigned_key, error_msg);
                reply.assigned_key = assigned_key;
                if (reply.status == FAILURE)
                    strncpy(reply.error_msg, error_msg, sizeof(reply.error_msg) - 1);
            break;
}

            case DB_OP_UPDATE:
                reply.status = db_update(&db_info, req.db_id, req.key, &req.payload, error_msg  );
                if (reply.status == FAILURE)
                    strncpy(reply.error_msg, error_msg, sizeof(reply.error_msg) - 1);
                else if (reply.status == REC_NOT_FOUND)
                    strncpy(reply.error_msg, "Record not found", sizeof(reply.error_msg) - 1);
                break;

            case DB_OP_DELETE:
                reply.status = db_delete(&db_info, req.db_id, req.key, error_msg);
                if (reply.status == FAILURE)
                    strncpy(reply.error_msg, error_msg, sizeof(reply.error_msg) - 1);
                else if (reply.status == REC_NOT_FOUND)
                    strncpy(reply.error_msg, "Record not found", sizeof(reply.error_msg) - 1);
                break;

            case DB_OP_READ:
                reply.status = db_read(&db_info, req.db_id, req.key, &reply.payload);
                if (reply.status == FAILURE)
                    strncpy(reply.error_msg, error_msg, sizeof(reply.error_msg) - 1);
                else if (reply.status == REC_NOT_FOUND)
                    strncpy(reply.error_msg, "Record not found", sizeof(reply.error_msg) - 1);
                break;

            default:
                fprintf(stderr, "DB_SERVICE: Thread %d received unknown op %d\n",
                        my_id, req.operation);
                reply.status = FAILURE;
                strncpy(reply.error_msg, "Unknown operation", sizeof(reply.error_msg) - 1);
                break;
        }

        // Send reply — client is blocking on msgrcv with mtype=reply_to
        if (msgsnd(msgid_reply, &reply,
                   sizeof(struct msg_response) - sizeof(long), 0) == -1) {
            perror("DB_SERVICE: msgsnd failed");
        }
    }

    return NULL;
}


// =========================================================================================

int main() {

    key_t key = ftok("DB_Service/structs.h", 65);

    msgid_request = msgget(key, 0666 | IPC_CREAT);
    if (msgid_request == -1) {
        perror("msgget request queue failed");
        exit(EXIT_FAILURE);
    }

    msgid_reply = msgget(key + 1, 0666 | IPC_CREAT);
    if (msgid_reply == -1) {
        perror("msgget reply queue failed");
        exit(EXIT_FAILURE);
    }

    if (db_init(&db_info) == FAILURE) {
        fprintf(stderr, "DB_SERVICE: Failed to initialize the database\n");
        exit(EXIT_FAILURE);
    }

    printf("DB_SERVICE: Database initialized. Starting %d worker threads.\n",
           DB_MAX_THREADS);

    pthread_t threads[DB_MAX_THREADS];
    int       thread_ids[DB_MAX_THREADS];

    for (int i = 0; i < DB_MAX_THREADS; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]) != 0) {
            perror("DB_SERVICE: Failed to create worker thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < DB_MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}