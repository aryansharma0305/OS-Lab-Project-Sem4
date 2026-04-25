#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <pthread.h>

#include "DB_Client.h"
#include "../structs.h"
#include "../DB_Engine/DB_File_IO_Handler.h"


// =============================================================================
// Internal helpers
// =============================================================================

static long make_request_id(struct db_client* client) {
    pthread_mutex_lock(&client->counter_mutex);
    long id = client->client_id * 100000L + (++(client->request_counter));
    pthread_mutex_unlock(&client->counter_mutex);
    return id;
}

// Core gateway for INSERT / UPDATE / DELETE / READ
static int db_client_call(struct db_client* client,
                          int               operation,
                          int               db_type,
                          int               key,
                          void*             payload_in,
                          size_t            payload_size,
                          void*             payload_out,
                          int*              assigned_key)
{
    struct msg_request  req;
    struct msg_response reply;

    memset(&req, 0, sizeof(req));

    req.msg_type  = 1;
    req.reply_to  = make_request_id(client);
    req.operation = operation;
    req.db_id     = db_type;
    req.key       = key;
    req.match_mask = 0;   // not used for non-find ops

    if (payload_in != NULL)
        memcpy(&req.payload, payload_in, payload_size);

    if (msgsnd(client->msgid_request, &req,
               sizeof(struct msg_request) - sizeof(long), 0) == -1) {
        perror("DB_CLIENT: msgsnd failed");
        return FAILURE;
    }

    if (msgrcv(client->msgid_reply, &reply,
               sizeof(struct msg_response) - sizeof(long),
               req.reply_to, 0) == -1) {
        perror("DB_CLIENT: msgrcv failed");
        return FAILURE;
    }

    if (operation == DB_OP_READ &&
        reply.status == SUCCESS &&
        payload_out != NULL) {
        memcpy(payload_out, &reply.payload, payload_size);
    }

    if (operation == DB_OP_INSERT &&
        reply.status == SUCCESS &&
        assigned_key != NULL) {
        *assigned_key = reply.assigned_key;
    }

    if (reply.status != SUCCESS && reply.error_msg[0] != '\0')
        fprintf(stderr, "DB_CLIENT: Server error: %s\n", reply.error_msg);

    return reply.status;
}

// Separate gateway for FIND — needs match_mask and always populates payload_out
static int db_client_call_find(struct db_client* client,
                               int               db_type,
                               void*             partial,
                               size_t            payload_size,
                               unsigned int      match_mask,
                               void*             record_out)
{
    struct msg_request  req;
    struct msg_response reply;

    memset(&req, 0, sizeof(req));

    req.msg_type   = 1;
    req.reply_to   = make_request_id(client);
    req.operation  = DB_OP_FIND;
    req.db_id      = db_type;
    req.key        = 0;
    req.match_mask = match_mask;

    memcpy(&req.payload, partial, payload_size);

    if (msgsnd(client->msgid_request, &req,
               sizeof(struct msg_request) - sizeof(long), 0) == -1) {
        perror("DB_CLIENT: msgsnd failed");
        return FAILURE;
    }

    if (msgrcv(client->msgid_reply, &reply,
               sizeof(struct msg_response) - sizeof(long),
               req.reply_to, 0) == -1) {
        perror("DB_CLIENT: msgrcv failed");
        return FAILURE;
    }

    if (reply.status == SUCCESS && record_out != NULL)
        memcpy(record_out, &reply.payload, payload_size);

    if (reply.status != SUCCESS && reply.error_msg[0] != '\0')
        fprintf(stderr, "DB_CLIENT: Server error: %s\n", reply.error_msg);

    return reply.status;
}


// =============================================================================
// Lifecycle
// =============================================================================

int db_client_init(struct db_client* client) {

    key_t key = ftok("DB_Service/structs.h", 65);
    if (key == -1) {
        perror("DB_CLIENT: ftok failed");
        return FAILURE;
    }

    client->msgid_request = msgget(key, 0666);
    if (client->msgid_request == -1) {
        perror("DB_CLIENT: msgget request queue failed — is DB_engine running?");
        return FAILURE;
    }

    client->msgid_reply = msgget(key + 1, 0666);
    if (client->msgid_reply == -1) {
        perror("DB_CLIENT: msgget reply queue failed");
        return FAILURE;
    }

    client->request_counter = 0;
    client->client_id       = (long)getpid();

    if (pthread_mutex_init(&client->counter_mutex, NULL) != 0) {
        perror("DB_CLIENT: pthread_mutex_init failed");
        return FAILURE;
    }

    printf("DB_CLIENT: Connected to DB service (pid=%ld)\n", client->client_id);
    return SUCCESS;
}

void db_client_destroy(struct db_client* client) {
    pthread_mutex_destroy(&client->counter_mutex);
    memset(client, 0, sizeof(struct db_client));
}


// =============================================================================
// Orders
// =============================================================================

int db_client_insert_order(struct db_client* client, struct orders* record, int* assigned_key) {
    return db_client_call(client, DB_OP_INSERT, 1, 0,
                          record, sizeof(struct orders), NULL, assigned_key);
}

int db_client_update_order(struct db_client* client, int key, struct orders* record) {
    return db_client_call(client, DB_OP_UPDATE, 1, key,
                          record, sizeof(struct orders), NULL, NULL);
}

int db_client_delete_order(struct db_client* client, int key) {
    return db_client_call(client, DB_OP_DELETE, 1, key,
                          NULL, 0, NULL, NULL);
}

int db_client_read_order(struct db_client* client, int key, struct orders* record_out) {
    return db_client_call(client, DB_OP_READ, 1, key,
                          NULL, sizeof(struct orders), record_out, NULL);
}

int db_client_find_order(struct db_client* client, struct orders* partial,
                         unsigned int match_mask, struct orders* record_out) {
    return db_client_call_find(client, 1,
                               partial, sizeof(struct orders),
                               match_mask, record_out);
}


// =============================================================================
// Menu
// =============================================================================

int db_client_insert_menu(struct db_client* client, struct menu* record, int* assigned_key) {
    return db_client_call(client, DB_OP_INSERT, 2, 0,
                          record, sizeof(struct menu), NULL, assigned_key);
}

int db_client_update_menu(struct db_client* client, int key, struct menu* record) {
    return db_client_call(client, DB_OP_UPDATE, 2, key,
                          record, sizeof(struct menu), NULL, NULL);
}

int db_client_delete_menu(struct db_client* client, int key) {
    return db_client_call(client, DB_OP_DELETE, 2, key,
                          NULL, 0, NULL, NULL);
}

int db_client_read_menu(struct db_client* client, int key, struct menu* record_out) {
    return db_client_call(client, DB_OP_READ, 2, key,
                          NULL, sizeof(struct menu), record_out, NULL);
}

int db_client_find_menu(struct db_client* client, struct menu* partial,
                        unsigned int match_mask, struct menu* record_out) {
    return db_client_call_find(client, 2,
                               partial, sizeof(struct menu),
                               match_mask, record_out);
}


// =============================================================================
// Tables
// =============================================================================

int db_client_insert_table(struct db_client* client, struct tables* record, int* assigned_key) {
    return db_client_call(client, DB_OP_INSERT, 3, 0,
                          record, sizeof(struct tables), NULL, assigned_key);
}

int db_client_update_table(struct db_client* client, int key, struct tables* record) {
    return db_client_call(client, DB_OP_UPDATE, 3, key,
                          record, sizeof(struct tables), NULL, NULL);
}

int db_client_delete_table(struct db_client* client, int key) {
    return db_client_call(client, DB_OP_DELETE, 3, key,
                          NULL, 0, NULL, NULL);
}

int db_client_read_table(struct db_client* client, int key, struct tables* record_out) {
    return db_client_call(client, DB_OP_READ, 3, key,
                          NULL, sizeof(struct tables), record_out, NULL);
}

int db_client_find_table(struct db_client* client, struct tables* partial,
                         unsigned int match_mask, struct tables* record_out) {
    return db_client_call_find(client, 3,
                               partial, sizeof(struct tables),
                               match_mask, record_out);
}