#ifndef STRUCTS_H
#define STRUCTS_H

#define DB_OP_INSERT 1
#define DB_OP_UPDATE 2
#define DB_OP_DELETE 3
#define DB_OP_READ   4
#define DB_OP_FIND   5
#define DB_OP_FIND_AND_UPDATE 6

#define DB_STATUS_SUCCESS       0
#define DB_STATUS_FAILURE       1
#define DB_STATUS_NOT_FOUND     2

#define DB_MAX_ORDER_ITEMS 50


// Orders
#define ORDER_MATCH_ORDER_ID     (1 << 0)
#define ORDER_MATCH_NAME       (1 << 1)
#define ORDER_MATCH_PHONE      (1 << 2)
#define ORDER_MATCH_EMAIL      (1 << 3)
#define ORDER_MATCH_TABLE_ID   (1 << 4)
#define ORDER_MATCH_ORDER_TIME (1 << 5)
#define ORDER_MATCH_ORDER_DATE (1 << 6)
#define ORDER_MATCH_TOTAL_BILL (1 << 7)

// Menu
#define MENU_MATCH_ITEM_ID    (1 << 0)
#define MENU_MATCH_NAME        (1 << 1)
#define MENU_MATCH_PRICE       (1 << 2)

// Tables
#define TABLES_MATCH_TABLE_ID   (1 << 0)
#define TABLES_MATCH_CAPACITY   (1 << 1)
#define TABLES_MATCH_IS_OCCUPIED (1 << 2)

// Users
#define USERS_MATCH_USER_ID     (1 << 0)
#define USERS_MATCH_USERNAME    (1 << 1)
#define USERS_MATCH_PASSWORD    (1 << 2)

#define SUCCESS 0
#define FAILURE -1
#define REC_NOT_FOUND 1


struct menu {
    int   itemID;
    char  itemName[50];
    float price;
};

struct order_item {
    int orderItemID; // nothing but just the index in the order's items array (0 to DB_MAX_ORDER_ITEMS-1) 
    struct menu item;
    int quantity;
    int is_prepared;
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
    int               isBillGenerated;
};



struct tables {
    int tableID;
    int capacity; // can only be 2, 4, 8, or 12
    int isOccupied;
};

enum user_role {
    ROLE_ADMIN,
    ROLE_WAITER,
    ROLE_CHEF
};

struct users{
    int userID;
    char username[50];
    char password[50];
    enum user_role role;
};

struct msg_request {
    long msg_type;    // always 1 — any worker picks it up
    long reply_to;    // unique request ID — worker copies this to reply msg_type
    int  operation;   // DB_OP_*
    int  db_id;     // 1=orders 2=menu 3=tables 4=users
    int  key;         // used by update/delete/read to identify the record
    unsigned int match_mask; 
    union {
        struct orders  orders;
        struct menu menu;
        struct tables tables;
        struct users   users;
    } payload;        // used by insert/update

    union {
        struct orders  orders;
        struct menu menu;
        struct tables tables;
        struct users   users;
    } payload2;       // used by find-and-update's new_record

};

struct msg_response {
    long msg_type;
    int  status;
    int  assigned_key;   // ← populated on INSERT, -1 otherwise
    char error_msg[256];
    union {
        struct orders  orders;
        struct menu   menu;
        struct tables  tables;
        struct users users;
    } payload;
};
#endif