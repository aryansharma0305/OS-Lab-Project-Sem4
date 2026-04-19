#ifndef DB_FILE_IO_HANDLER_H
#define DB_FILE_IO_HANDLER_H


#define SUCCESS 0
#define FAILURE -1
#define REC_NOT_FOUND 1

#define DB_OPEN 0
#define DB_CLOSE 1


struct index{
    int key;
    int offset;
    int is_deleted; 
    
};


// orders -> type 1 menu -> type 2 tables -> type 3

struct dbinfo{
    int fd_orders;    
    int fd_menu;
    int fd_tables;

    int fd_index_orders;
    int fd_index_menu;
    int fd_index_tables;

    int db_is_open_orders;
    int db_is_open_menu;
    int db_is_open_tables;
    int db_is_open_index_orders;
    int db_is_open_index_menu;
    int db_is_open_index_tables;

    int record_size_orders;
    int record_size_menu;
    int record_size_tables;

    int num_records_orders;
    int num_records_menu;
    int num_records_tables;


    // ASSUMING max of 1000 record in each table;
    struct index index_orders[1000]; 
    struct index index_menu[1000];
    struct index index_tables[1000];
};



int db_init(struct dbinfo* db_info);

int db_insert(struct dbinfo* db_info, int type, void* record);
int db_update(struct dbinfo* db_info, int type, int key, void* record_in);
int db_delete(struct dbinfo* db_info, int type, int key);
int db_read(struct dbinfo* db_info, int type, int key, void* record_out);

#endif