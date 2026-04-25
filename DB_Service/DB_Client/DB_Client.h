#ifndef DB_CLIENT_H
#define DB_CLIENT_H

#include <pthread.h>
#include "../structs.h"
#include "../DB_Engine/DB_File_IO_Handler.h"

struct db_client {
    int             msgid_request;
    int             msgid_reply;
    long            request_counter;
    long            client_id;
    pthread_mutex_t counter_mutex;
};

// ---------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------
int  db_client_init   (struct db_client* client);
void db_client_destroy(struct db_client* client);

// ---------------------------------------------------------------------
// Orders
// ---------------------------------------------------------------------
int db_client_insert_order(struct db_client* client, struct orders* record, int* assigned_key);
int db_client_update_order(struct db_client* client, int key, struct orders* record);
int db_client_delete_order(struct db_client* client, int key);
int db_client_read_order  (struct db_client* client, int key, struct orders* record_out);
int db_client_find_order  (struct db_client* client, struct orders* partial,
                           unsigned int match_mask, struct orders* record_out);

// ---------------------------------------------------------------------
// Menu
// ---------------------------------------------------------------------
int db_client_insert_menu(struct db_client* client, struct menu* record, int* assigned_key);
int db_client_update_menu(struct db_client* client, int key, struct menu* record);
int db_client_delete_menu(struct db_client* client, int key);
int db_client_read_menu  (struct db_client* client, int key, struct menu* record_out);
int db_client_find_menu  (struct db_client* client, struct menu* partial,
                          unsigned int match_mask, struct menu* record_out);

// ---------------------------------------------------------------------
// Tables
// ---------------------------------------------------------------------
int db_client_insert_table(struct db_client* client, struct tables* record, int* assigned_key);
int db_client_update_table(struct db_client* client, int key, struct tables* record);
int db_client_delete_table(struct db_client* client, int key);
int db_client_read_table  (struct db_client* client, int key, struct tables* record_out);
int db_client_find_table  (struct db_client* client, struct tables* partial,
                           unsigned int match_mask, struct tables* record_out);

#endif