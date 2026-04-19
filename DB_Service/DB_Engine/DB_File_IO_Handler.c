#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include "DB_File_IO_Handler.h"
#include "../structs.h"

extern pthread_mutex_t db_mutex_db_info; 


// =========================================================================================

int db_init(struct dbinfo* db_info){

    db_info->fd_orders = open("../Data/orders.db", O_RDWR | O_CREAT, 0666);
    if (db_info->fd_orders < 0) return FAILURE;


    db_info->fd_menu = open("../Data/menu.db", O_RDWR | O_CREAT, 0666);
    if (db_info->fd_menu < 0) return FAILURE;


    db_info->fd_tables = open("../Data/tables.db", O_RDWR | O_CREAT, 0666);
    if (db_info->fd_tables < 0) return FAILURE;


    db_info->fd_index_orders = open("../Data/orders.idx", O_RDWR | O_CREAT, 0666);
    if (db_info->fd_index_orders < 0) return FAILURE;


    db_info->fd_index_menu = open("../Data/menu.idx", O_RDWR | O_CREAT, 0666);
    if (db_info->fd_index_menu < 0) return FAILURE;


    db_info->fd_index_tables = open("../Data/tables.idx", O_RDWR | O_CREAT, 0666);
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



    return SUCCESS;
}







// =========================================================================================


int db_insert(struct dbinfo* db_info, int type, void* record){

    int target_fd;
    int target_record_size;
    struct index* target_index_array;
    int target_num_records;

    if(type == 1){
        target_fd = db_info->fd_orders;
        target_record_size = db_info->record_size_orders;
        target_index_array = db_info->index_orders;
        target_num_records = db_info->num_records_orders;
    }

    else if(type == 2){
        target_fd = db_info->fd_menu;
        target_record_size = db_info->record_size_menu;
        target_index_array = db_info->index_menu;
        target_num_records = db_info->num_records_menu;
    }

    else if(type == 3){
        target_fd = db_info->fd_tables;
        target_record_size = db_info->record_size_tables;
        target_index_array = db_info->index_tables;
        target_num_records = db_info->num_records_tables;

    }
    else{
        return FAILURE; // invalid type
    }


    // we need record locking for this
    // lock only the record we are writing to, since we are appending, we can just lock the end of the file
    struct flock lock;
    lock.l_type = F_WRLCK; // write lock
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = target_record_size;

    if (fcntl(target_fd, F_SETLKW, &lock) == -1) {
        perror("Failed to acquire lock");
        return FAILURE;
    }


    lseek(target_fd, 0, SEEK_END);
    if(write(target_fd, record, target_record_size) != target_record_size){
        return FAILURE; // write failed
    }
    target_num_records++;

    //relase the lock
    lock.l_type = F_UNLCK; // unlock
    if (fcntl(target_fd, F_SETLKW, &lock) == -1) {
        perror("Failed to release lock");
        return FAILURE;
    }



    //update the in memory index SINCE THIS IS SHARED BETWEEN THREADS, we need mutex lock here
    pthread_mutex_lock(&db_mutex_db_info);
        if(type == 1){
            struct orders* order_record = (struct orders*)record;
            target_index_array[target_num_records - 1].key = order_record->orderID;
            target_index_array[target_num_records - 1].offset = lseek(target_fd, 0, SEEK_END) - target_record_size;
            target_index_array[target_num_records - 1].is_deleted = 0; \
            db_info->num_records_orders = target_num_records;
        }
        else if(type == 2){
            struct menu* menu_record = (struct menu*)record;
            target_index_array[target_num_records - 1].key = menu_record->itemID;
            target_index_array[target_num_records - 1].offset = lseek(target_fd, 0, SEEK_END) - target_record_size;
            target_index_array[target_num_records - 1].is_deleted = 0;
            db_info->num_records_menu = target_num_records;
        }
        else if(type == 3){
            struct tables* table_record = (struct tables*)record;
            target_index_array[target_num_records - 1].key = table_record->tableID;
            target_index_array[target_num_records - 1].offset = lseek(target_fd, 0, SEEK_END) - target_record_size;
            target_index_array[target_num_records - 1].is_deleted = 0;
            db_info->num_records_tables = target_num_records;
        }
        else {
            return FAILURE; 
        }
    pthread_mutex_unlock(&db_mutex_db_info);

    return SUCCESS;
}


// =========================================================================================



int db_update(struct dbinfo* db_info, int type, int key, void* record_in){
    
    struct index* target_index_array;
    int target_record_size;
    int target_fd;


    if(type == 1){
        target_index_array = db_info->index_orders;
        target_record_size = db_info->record_size_orders;
        target_fd = db_info->fd_orders;
    }

    else if(type == 2){
        target_index_array = db_info->index_menu;
        target_record_size = db_info->record_size_menu;
        target_fd = db_info->fd_menu;
    }

    else if(type == 3){
        target_index_array = db_info->index_tables;
        target_record_size = db_info->record_size_tables;
        target_fd = db_info->fd_tables;
    }
    else{
        return FAILURE; // invalid type
    }



    // find the record in the index

    int found = 0;
    int target_offset = -1;
    for(int i = 0; i < 1000; i++){
        if(target_index_array[i].key == key && target_index_array[i].is_deleted == 0){
            found = 1;
            target_offset = target_index_array[i].offset;
        }
        if (found){
            break;
        }
    }    

    if(!found){
        return REC_NOT_FOUND; // record not found
    }






    // we need record locking for this
    struct flock lock;
    lock.l_type = F_WRLCK; // write lock
    lock.l_whence = SEEK_SET;
    lock.l_start = target_offset;
    lock.l_len = target_record_size;

    fcntl(target_fd, F_SETLKW, &lock); // acquire lock

    lseek(target_fd, target_offset, SEEK_SET);
    if(write(target_fd, record_in, target_record_size) != target_record_size){
        return FAILURE; // write failed
    }

    lock.l_type = F_UNLCK; // unlock
    if (fcntl(target_fd, F_SETLKW, &lock) == -1) {
        perror("Failed to release lock");
        return FAILURE; 
    }

    return SUCCESS;
}




// =========================================================================================




int db_delete(struct dbinfo* db_info, int type, int key){
    
    struct index* target_index_array;
    // pseudo delete - just mark as deleted.

    if(type == 1){
        target_index_array = db_info->index_orders;
    }

    else if(type == 2){
        target_index_array = db_info->index_menu;
    }

    else if(type == 3){
        target_index_array = db_info->index_tables;
    }
    else{
        return FAILURE; // invalid type
    }


    // find the record in the index
    int found = 0;
    int target_index_in_index_array = -1;
     for(int i = 0; i < 1000; i++){
        if(target_index_array[i].key == key && target_index_array[i].is_deleted == 0){
            found = 1;
            target_index_in_index_array = i;
        }
        if (found){
            break;
        }
     }

    if(!found){
        return REC_NOT_FOUND; // record not found
    }


    // mark as deleted in the in memory index
    pthread_mutex_lock(&db_mutex_db_info);
        target_index_array[target_index_in_index_array].is_deleted = 1;
    pthread_mutex_unlock(&db_mutex_db_info);

    return SUCCESS;
}



// =========================================================================================

int db_read(struct dbinfo* db_info, int type, int key, void* record_out){
    
    struct index* target_index_array;
    int target_record_size;
    int target_fd;

    if(type == 1){
        target_index_array = db_info->index_orders;
        target_record_size = db_info->record_size_orders;
        target_fd = db_info->fd_orders;
    }

    else if(type == 2){
        target_index_array = db_info->index_menu;
        target_record_size = db_info->record_size_menu;
        target_fd = db_info->fd_menu;
    }

    else if(type == 3){
        target_index_array = db_info->index_tables;
        target_record_size = db_info->record_size_tables;
        target_fd = db_info->fd_tables;
    }
    else{
        return FAILURE; // invalid type
    }


    // find the record in the index
    int found = 0;
    int target_offset = -1;
    for(int i = 0; i < 1000; i++){
        if(target_index_array[i].key == key && target_index_array[i].is_deleted == 0){
            found = 1;
            target_offset = target_index_array[i].offset;  
        }
        if (found){
            break;
        }
    }

    if(!found){
        return REC_NOT_FOUND; // record not found
    }


    // we need record locking for this
    struct flock lock;
    lock.l_type = F_RDLCK; // read lock
    lock.l_whence = SEEK_SET;
    lock.l_start = target_offset;
    lock.l_len = target_record_size;

    if (fcntl(target_fd, F_SETLKW, &lock) == -1) {
        perror("Failed to acquire lock");
        return FAILURE;
    }


    lseek(target_fd, target_offset, SEEK_SET);
    if(read(target_fd, record_out, target_record_size) != target_record_size){
        return FAILURE; // read failed 
    }

    lock.l_type = F_UNLCK; // unlock
    if (fcntl(target_fd, F_SETLKW, &lock) == -1) {
        perror("Failed to release lock");
        return FAILURE;
    }

    return SUCCESS;
}