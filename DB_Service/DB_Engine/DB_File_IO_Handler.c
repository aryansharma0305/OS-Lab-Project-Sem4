#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "DB_File_IO_Handler.h"
#include "../structs.h"

extern pthread_mutex_t db_mutex_db_info; 


// =========================================================================================

int db_init(struct dbinfo* db_info){


    //getting mutex lock for db_info since we are writing to it
    pthread_mutex_lock(&db_mutex_db_info);


    db_info->fd_orders = open("DB_Service/Data/orders.db", O_RDWR | O_CREAT, 0666);
    if (db_info->fd_orders < 0) return FAILURE;


    db_info->fd_menu = open("DB_Service/Data/menu.db", O_RDWR | O_CREAT, 0666);
    if (db_info->fd_menu < 0) return FAILURE;


    db_info->fd_tables = open("DB_Service/Data/tables.db", O_RDWR | O_CREAT, 0666);
    if (db_info->fd_tables < 0) return FAILURE;


    db_info->fd_index_orders = open("DB_Service/Data/orders.idx", O_RDWR | O_CREAT, 0666);
    if (db_info->fd_index_orders < 0) return FAILURE;


    db_info->fd_index_menu = open("DB_Service/Data/menu.idx", O_RDWR | O_CREAT, 0666);
    if (db_info->fd_index_menu < 0) return FAILURE;


    db_info->fd_index_tables = open("DB_Service/Data/tables.idx", O_RDWR | O_CREAT, 0666);
    if (db_info->fd_index_tables < 0) return FAILURE;





    db_info->db_is_open_orders = DB_OPEN;
    db_info->db_is_open_menu = DB_OPEN;
    db_info->db_is_open_tables = DB_OPEN;
    db_info->db_is_open_index_orders = DB_OPEN;
    db_info->db_is_open_index_menu = DB_OPEN;
    db_info->db_is_open_index_tables = DB_OPEN;




    db_info->record_size_orders = sizeof(struct orders);
    db_info->record_size_menu = sizeof(struct menu);
    db_info->record_size_tables = sizeof(struct tables);


    for(int i = 0; i < 1000; i++){
    db_info->index_orders[i].is_deleted = 1;
    db_info->index_menu[i].is_deleted = 1;
    db_info->index_tables[i].is_deleted = 1;

    pthread_rwlock_init(&db_info->orders_rwlock[i], NULL);
    pthread_rwlock_init(&db_info->menu_rwlock[i], NULL);
    pthread_rwlock_init(&db_info->tables_rwlock[i], NULL);
    }


    // creating in memeory index from index file
    struct index temp;

    db_info->num_records_orders = 0;
    lseek(db_info->fd_index_orders, 0, SEEK_SET);
    while (read(db_info->fd_index_orders, &temp, sizeof(struct index)) == sizeof(struct index)) {
        db_info->index_orders[db_info->num_records_orders++] = temp;
    }



    db_info->num_records_menu = 0;
    lseek(db_info->fd_index_menu, 0, SEEK_SET);
    while (read(db_info->fd_index_menu, &temp, sizeof(struct index)) == sizeof(struct index)) {
        db_info->index_menu[db_info->num_records_menu++] = temp;
    }



    db_info->num_records_tables = 0;
    lseek(db_info->fd_index_tables, 0, SEEK_SET);
    while (read(db_info->fd_index_tables, &temp, sizeof(struct index)) == sizeof(struct index)) {
        db_info->index_tables[db_info->num_records_tables++] = temp;
    }



    //release the lock
    pthread_mutex_unlock(&db_mutex_db_info);

    return SUCCESS;
}







// =========================================================================================




// int db_insert(struct dbinfo* db_info, int type, void* record){

//     int target_fd;
//     int target_fd_index;
//     int target_record_size;
//     struct index* target_index_array;
//     pthread_rwlock_t* target_rwlock;
//     int my_index;

//     // Reserve index and lock db_info
//     pthread_mutex_lock(&db_mutex_db_info);

//     if(type == 1){
//         my_index = db_info->num_records_orders++;
//         target_fd = db_info->fd_orders;
//         target_fd_index = db_info->fd_index_orders;
//         target_record_size = db_info->record_size_orders;
//         target_index_array = db_info->index_orders;
//         target_rwlock = &db_info->orders_rwlock[my_index];
//         pthread_rwlock_init(&db_info->orders_rwlock[my_index], NULL);
//         db_info->index_orders[my_index].is_deleted = 1;
//     }
//     else if(type == 2){
//         my_index = db_info->num_records_menu++;
//         target_fd = db_info->fd_menu;
//         target_fd_index = db_info->fd_index_menu;
//         target_record_size = db_info->record_size_menu;
//         target_index_array = db_info->index_menu;
//         target_rwlock = &db_info->menu_rwlock[my_index];
//         pthread_rwlock_init(&db_info->menu_rwlock[my_index], NULL);
//         db_info->index_menu[my_index].is_deleted = 1;
//     }
//     else if(type == 3){
//         my_index = db_info->num_records_tables++;
//         target_fd = db_info->fd_tables;
//         target_fd_index = db_info->fd_index_tables;
//         target_record_size = db_info->record_size_tables;
//         target_index_array = db_info->index_tables;
//         target_rwlock = &db_info->tables_rwlock[my_index];
//         pthread_rwlock_init(&db_info->tables_rwlock[my_index], NULL);
//         db_info->index_tables[my_index].is_deleted = 1;
//     }
//     else{
//         pthread_mutex_unlock(&db_mutex_db_info);
//         return FAILURE;
//     }

//     pthread_mutex_unlock(&db_mutex_db_info);

//     // Get write record lock
//     pthread_rwlock_wrlock(target_rwlock);

//     off_t offset = (off_t)target_record_size * my_index;

//     // Write the record to the data file
//     if(pwrite(target_fd, record, target_record_size, offset) != target_record_size){
//         pthread_rwlock_unlock(target_rwlock);
//         return FAILURE;
//     }

//     // Update in-memory index under mutex, then copy and release before disk write
//     pthread_mutex_lock(&db_mutex_db_info);

//     if(type == 1){
//         struct orders* r = (struct orders*)record;
//         db_info->index_orders[my_index].key = r->orderID;
//         db_info->index_orders[my_index].offset = offset;
//         db_info->index_orders[my_index].is_deleted = 0;
//     }
//     else if(type == 2){
//         struct menu* r = (struct menu*)record;
//         db_info->index_menu[my_index].key = r->itemID;
//         db_info->index_menu[my_index].offset = offset;
//         db_info->index_menu[my_index].is_deleted = 0;
//     }
//     else{
//         struct tables* r = (struct tables*)record;
//         db_info->index_tables[my_index].key = r->tableID;
//         db_info->index_tables[my_index].offset = offset;
//         db_info->index_tables[my_index].is_deleted = 0;
//     }

//     // Take a local copy, then release mutex before hitting disk.
//     // Safe because target_rwlock is still held — no other thread
//     // can touch this slot until we release it at the very end.
//     struct index index_copy = target_index_array[my_index];

//     pthread_mutex_unlock(&db_mutex_db_info);

//     // Persist index entry to .idx file outside the mutex
//     off_t idx_offset = (off_t)my_index * sizeof(struct index);
//     if(pwrite(target_fd_index, &index_copy,
//               sizeof(struct index), idx_offset) != sizeof(struct index)){
//         pthread_rwlock_unlock(target_rwlock);
//         return FAILURE;
//     }

//     pthread_rwlock_unlock(target_rwlock);
//     return SUCCESS;
// }





int db_insert(struct dbinfo* db_info, int type, void* record, int* assigned_key, char* error_msg) {

    int target_fd;
    int target_fd_index;
    int target_record_size;
    struct index* target_index_array;
    pthread_rwlock_t* target_rwlock;
    int my_index;

    
    pthread_mutex_lock(&db_mutex_db_info);

    if (type == 1) {
        if (db_info->num_records_orders >= 1000) {
            pthread_mutex_unlock(&db_mutex_db_info);
            strcpy(error_msg, "Orders table full");
            return FAILURE;
        }
        my_index = db_info->num_records_orders++;
        target_fd = db_info->fd_orders;
        target_fd_index = db_info->fd_index_orders;
        target_record_size = db_info->record_size_orders;
        target_index_array = db_info->index_orders;
        target_rwlock = &db_info->orders_rwlock[my_index];

        ((struct orders*)record)->orderID = my_index;
        db_info->index_orders[my_index].is_deleted = 1;
    }
    else if (type == 2) {
        if (db_info->num_records_menu >= 1000) {
            pthread_mutex_unlock(&db_mutex_db_info);
            strcpy(error_msg, "Menu table full");
            return FAILURE;
        }
        my_index = db_info->num_records_menu++;
        target_fd = db_info->fd_menu;
        target_fd_index  = db_info->fd_index_menu;
        target_record_size = db_info->record_size_menu;
        target_index_array = db_info->index_menu;
        target_rwlock = &db_info->menu_rwlock[my_index];

        ((struct menu*)record)->itemID = my_index;
        db_info->index_menu[my_index].is_deleted = 1;
    }
    else if (type == 3) {
        if (db_info->num_records_tables >= 1000) {
            pthread_mutex_unlock(&db_mutex_db_info);
            strcpy(error_msg, "Tables table full");
            return FAILURE;
        }
        my_index = db_info->num_records_tables++;
        target_fd = db_info->fd_tables;
        target_fd_index = db_info->fd_index_tables;
        target_record_size = db_info->record_size_tables;
        target_index_array = db_info->index_tables;
        target_rwlock = &db_info->tables_rwlock[my_index];

        ((struct tables*)record)->tableID = my_index;
        db_info->index_tables[my_index].is_deleted = 1;
    }
    else {
        pthread_mutex_unlock(&db_mutex_db_info);
        strcpy(error_msg, "Invalid type");
        return FAILURE;
    }

    pthread_mutex_unlock(&db_mutex_db_info);


    pthread_rwlock_wrlock(target_rwlock);

    off_t offset = (off_t)target_record_size * my_index;


    if (pwrite(target_fd, record, target_record_size, offset) != target_record_size) {
        pthread_rwlock_unlock(target_rwlock);
        strcpy(error_msg, "Failed to write record");
        return FAILURE;
    }
    fsync(target_fd);


    pthread_mutex_lock(&db_mutex_db_info);

    target_index_array[my_index].key = my_index;  // key == slot number
    target_index_array[my_index].offset = offset;
    target_index_array[my_index].is_deleted = 0;

    struct index index_copy = target_index_array[my_index];

    pthread_mutex_unlock(&db_mutex_db_info);


    off_t idx_offset = (off_t)my_index * sizeof(struct index);
    if (pwrite(target_fd_index, &index_copy,
               sizeof(struct index), idx_offset) != sizeof(struct index)) {
        pthread_rwlock_unlock(target_rwlock);
        strcpy(error_msg, "Failed to write index");
        return FAILURE;
    }
    fsync(target_fd_index);

    pthread_rwlock_unlock(target_rwlock);


    *assigned_key = my_index;
    return SUCCESS;
}








// =========================================================================================


int db_update(struct dbinfo* db_info, int type, int key, void* record_in, char* error_msg) {

    struct index*       target_index_array;
    pthread_rwlock_t*   target_rwlock_array;
    int                 target_record_size;
    int                 target_fd;
    int                 target_num_records;

    //accquire dbinfo lock

    pthread_mutex_lock(&db_mutex_db_info);

    if (type == 1) {
        target_index_array   = db_info->index_orders;
        target_rwlock_array  = db_info->orders_rwlock;
        target_record_size   = db_info->record_size_orders;
        target_fd            = db_info->fd_orders;
        target_num_records   = db_info->num_records_orders;
    }
    else if (type == 2) {
        target_index_array   = db_info->index_menu;
        target_rwlock_array  = db_info->menu_rwlock;
        target_record_size   = db_info->record_size_menu;
        target_fd            = db_info->fd_menu;
        target_num_records   = db_info->num_records_menu;
    }
    else if (type == 3) {
        target_index_array   = db_info->index_tables;
        target_rwlock_array  = db_info->tables_rwlock;
        target_record_size   = db_info->record_size_tables;
        target_fd            = db_info->fd_tables;
        target_num_records   = db_info->num_records_tables;
    }
    else {
         pthread_mutex_unlock(&db_mutex_db_info);
        strcpy(error_msg, "Invalid type");
        return FAILURE;
    }


    int    found         = 0;
    int    target_slot   = -1;
    off_t  target_offset = -1;

    for (int i = 0; i < target_num_records; i++) {
        if (target_index_array[i].key == key &&
            target_index_array[i].is_deleted == 0) {
            found         = 1;
            target_slot   = i;
            target_offset = target_index_array[i].offset;
            break;
        }
    }

    if (!found) {
        pthread_mutex_unlock(&db_mutex_db_info);
        strcpy(error_msg, "Record not found");
        return REC_NOT_FOUND;
    }

    // Acquire the per-record write lock WHILE still holding db_mutex_db_info,
    // consistent with the lock order used in db_insert's index-update phase.
    pthread_rwlock_wrlock(&target_rwlock_array[target_slot]);

    // Re-validate: the record may have been deleted between our scan
    // and acquiring the write lock.
    if (target_index_array[target_slot].is_deleted == 1) {
        pthread_rwlock_unlock(&target_rwlock_array[target_slot]);
        pthread_mutex_unlock(&db_mutex_db_info);
        strcpy(error_msg, "Record not found");
        return REC_NOT_FOUND;
    }

    pthread_mutex_unlock(&db_mutex_db_info);

    // --- Write the record (atomic positional write, no seek needed) ---
    if (pwrite(target_fd, record_in, target_record_size, target_offset)
            != target_record_size) {
        pthread_rwlock_unlock(&target_rwlock_array[target_slot]);
        strcpy(error_msg, "Failed to write record");
        return FAILURE;
    }
    fsync(target_fd);

    pthread_rwlock_unlock(&target_rwlock_array[target_slot]);
    return SUCCESS;
}


// =========================================================================================

int db_delete(struct dbinfo* db_info, int type, int key, char* error_msg) {

    struct index*       target_index_array;
    pthread_rwlock_t*   target_rwlock_array;
    int                 target_num_records;
    int                 target_fd_index;

    pthread_mutex_lock(&db_mutex_db_info);

    if (type == 1) {
        target_index_array  = db_info->index_orders;
        target_rwlock_array = db_info->orders_rwlock;
        target_num_records  = db_info->num_records_orders;
        target_fd_index     = db_info->fd_index_orders;
    }
    else if (type == 2) {
        target_index_array  = db_info->index_menu;
        target_rwlock_array = db_info->menu_rwlock;
        target_num_records  = db_info->num_records_menu;
        target_fd_index     = db_info->fd_index_menu;
    }
    else if (type == 3) {
        target_index_array  = db_info->index_tables;
        target_rwlock_array = db_info->tables_rwlock;
        target_num_records  = db_info->num_records_tables;
        target_fd_index     = db_info->fd_index_tables;
    }
    else {
        pthread_mutex_unlock(&db_mutex_db_info);
        strcpy(error_msg, "Invalid type");
        return FAILURE;
    }

    int found       = 0;
    int target_slot = -1;

    for (int i = 0; i < target_num_records; i++) {
        if (target_index_array[i].key == key &&
            target_index_array[i].is_deleted == 0) {
            found       = 1;
            target_slot = i;
            break;
        }
    }

    if (!found) {
        pthread_mutex_unlock(&db_mutex_db_info);
        strcpy(error_msg, "Record not found");
        return REC_NOT_FOUND;
    }

    // Acquire per-record write lock while still holding mutex.
    pthread_rwlock_wrlock(&target_rwlock_array[target_slot]);

    // Re-validate: another thread may have deleted this slot
    // between our scan and acquiring the write lock.
    if (target_index_array[target_slot].is_deleted == 1) {
        pthread_rwlock_unlock(&target_rwlock_array[target_slot]);
        pthread_mutex_unlock(&db_mutex_db_info);
        strcpy(error_msg, "Record not found");
        return REC_NOT_FOUND;
    }

    // Mark deleted in memory, take a local copy, then release mutex.
    // Safe because target_rwlock is still held — no other thread
    // can touch this slot until we release it at the very end.
    target_index_array[target_slot].is_deleted = 1;
    struct index index_copy = target_index_array[target_slot];

    pthread_mutex_unlock(&db_mutex_db_info);

    // Persist the updated index entry to .idx file outside the mutex
    off_t idx_offset = (off_t)target_slot * sizeof(struct index);
    if (pwrite(target_fd_index, &index_copy,
               sizeof(struct index), idx_offset) != sizeof(struct index)) {
        pthread_rwlock_unlock(&target_rwlock_array[target_slot]);
        strcpy(error_msg, "Failed to update index");
        return FAILURE;
    }
    fsync(target_fd_index);

    pthread_rwlock_unlock(&target_rwlock_array[target_slot]);
    return SUCCESS;
}


// =========================================================================================

int db_read(struct dbinfo* db_info, int type, int key, void* record_out) {

    struct index*       target_index_array;
    pthread_rwlock_t*   target_rwlock_array;
    int                 target_record_size;
    int                 target_fd;
    int                 target_num_records;


    pthread_mutex_lock(&db_mutex_db_info);

    if (type == 1) {
        target_index_array  = db_info->index_orders;
        target_rwlock_array = db_info->orders_rwlock;
        target_record_size  = db_info->record_size_orders;
        target_fd           = db_info->fd_orders;
        target_num_records  = db_info->num_records_orders;
    }
    else if (type == 2) {
        target_index_array  = db_info->index_menu;
        target_rwlock_array = db_info->menu_rwlock;
        target_record_size  = db_info->record_size_menu;
        target_fd           = db_info->fd_menu;
        target_num_records  = db_info->num_records_menu;
    }
    else if (type == 3) {
        target_index_array  = db_info->index_tables;
        target_rwlock_array = db_info->tables_rwlock;
        target_record_size  = db_info->record_size_tables;
        target_fd           = db_info->fd_tables;
        target_num_records  = db_info->num_records_tables;
    }
    else {
         pthread_mutex_unlock(&db_mutex_db_info);
        return FAILURE;
    }


    int   found         = 0;
    int   target_slot   = -1;
    off_t target_offset = -1;

    for (int i = 0; i < target_num_records; i++) {
        if (target_index_array[i].key == key &&
            target_index_array[i].is_deleted == 0) {
            found         = 1;
            target_slot   = i;
            target_offset = target_index_array[i].offset;
            break;
        }
    }

    if (!found) {
        pthread_mutex_unlock(&db_mutex_db_info);
        return REC_NOT_FOUND;
    }

    // Acquire per-record READ lock while still holding mutex.
    // Same lock-order as db_update and db_delete.
    pthread_rwlock_rdlock(&target_rwlock_array[target_slot]);

    // Re-validate: slot could have been deleted between scan and lock.
    if (target_index_array[target_slot].is_deleted == 1) {
        pthread_rwlock_unlock(&target_rwlock_array[target_slot]);
        pthread_mutex_unlock(&db_mutex_db_info);
        return REC_NOT_FOUND;
    }

    pthread_mutex_unlock(&db_mutex_db_info);

    // --- Read the record ---
    if (pread(target_fd, record_out, target_record_size, target_offset)
            != target_record_size) {
        pthread_rwlock_unlock(&target_rwlock_array[target_slot]);
        return FAILURE;
    }

    pthread_rwlock_unlock(&target_rwlock_array[target_slot]);
    return SUCCESS;
}