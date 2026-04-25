#ifndef STRUCTS_H
#define STRUCTS_H

#define DB_OP_INSERT 1
#define DB_OP_UPDATE 2
#define DB_OP_DELETE 3
#define DB_OP_READ   4

#define DB_STATUS_SUCCESS       0
#define DB_STATUS_FAILURE       1
#define DB_STATUS_NOT_FOUND     2

#define DB_MAX_ORDER_ITEMS 50


#define SUCCESS 0
#define FAILURE -1
#define REC_NOT_FOUND 1


struct order_item {
    int menuItemID;
    int quantity;
};

struct orders {
    int               orderID;
    char              name[50];
    char              phone[16];
    char              email[50];
    int               tableID;
    struct order_item items[DB_MAX_ORDER_ITEMS];
    int               num_items;
    char              orderTime[9];   // HH:MM:SS
    char              orderDate[11];  // YYYY-MM-DD
    float             totalBill;
};

struct menu {
    int   itemID;
    char  itemName[50];
    float price;
};

struct tables {
    int tableID;
    int capacity;
    int isOccupied;
};

struct msg_request {
    long msg_type;    // always 1 — any worker picks it up
    long reply_to;    // unique request ID — worker copies this to reply msg_type
    int  operation;   // DB_OP_*
    int  db_id;     // 1=orders 2=menu 3=tables
    int  key;         // used by update/delete/read to identify the record
    union {
        struct orders  orders;
        struct menu    menu;
        struct tables  tables;
    } payload;        // used by insert/update
};

struct msg_response {
    long msg_type;
    int  status;
    int  assigned_key;   // ← populated on INSERT, -1 otherwise
    char error_msg[256];
    union {
        struct orders  orders;
        struct menu    menu;
        struct tables  tables;
    } payload;
};
#endif