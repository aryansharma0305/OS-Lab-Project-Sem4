#include "guestHandler.h"

extern struct db_client DbClient;


int get_all_tables(struct tables* tables_array, int* count_out) {

    int count = 0;

    for (int key = 0; key < 1000; key++) {

        struct tables t;
        int status = db_client_read_table(&DbClient, key, &t);

        if (status == SUCCESS) {
            tables_array[count++] = t;
        }
        else if (status == FAILURE) {
            return FAILURE;
        }
    }

    *count_out = count;
    return SUCCESS;
}


int get_all_menu_items( struct menu* menu_array, int* count_out) {

    int count = 0;

    for (int key = 0; key < 1000; key++) {

        struct menu m;
        int status = db_client_read_menu(&DbClient, key, &m);

        if (status == SUCCESS) {
            menu_array[count++] = m;
        }
        else if (status == FAILURE) {
            return FAILURE;
        }
    }

    *count_out = count;
    return SUCCESS;
}


int book_table_and_create_order(int table_id, char* name, char* phone, char* email, int* assigned_order_id) {




    //first check if such table even exists
    struct tables t;
    int status = db_client_read_table(&DbClient, table_id, &t);

    if (status == REC_NOT_FOUND) return FAILURE;
    if (status == FAILURE)       return FAILURE;
    if (t.isOccupied == 1) {
        fprintf(stderr, "Table %d is already occupied\n", table_id);
        return FAILURE;
    }



    // if yes then match and update atomically.
    struct tables match  = { .tableID = table_id, .isOccupied = 0 };
    struct tables update = { .tableID = table_id, .capacity = t.capacity, .isOccupied = 1 };

    status = db_client_find_and_update_table(&DbClient, &match,TABLES_MATCH_TABLE_ID | TABLES_MATCH_IS_OCCUPIED,&update);
    if (status != SUCCESS) {
        fprintf(stderr, "Table %d could not be claimed\n", table_id);
        return FAILURE;
    }



    // Now create the order.
    struct orders order;
    memset(&order, 0, sizeof(order));
    strncpy(order.name,  name,  sizeof(order.name)  - 1);
    strncpy(order.phone, phone, sizeof(order.phone) - 1);
    strncpy(order.email, email, sizeof(order.email) - 1);
    order.tableID         = table_id;
    order.num_items       = 0;
    order.totalBill       = 0.0f;
    order.isBillGenerated = 0;

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(order.orderTime, sizeof(order.orderTime), "%H:%M:%S", tm_info);
    strftime(order.orderDate, sizeof(order.orderDate), "%Y-%m-%d", tm_info);

    int order_id = -1;
    status = db_client_insert_order(&DbClient, &order, &order_id);

    if (status != SUCCESS) {
        fprintf(stderr, "Order insert failed — rolling back table %d\n", table_id);

        struct tables rb_match  = { .tableID = table_id, .isOccupied = 1 };
        struct tables rb_update = { .tableID = table_id, .capacity = t.capacity, .isOccupied = 0 };
        db_client_find_and_update_table(&DbClient, &rb_match, TABLES_MATCH_TABLE_ID | TABLES_MATCH_IS_OCCUPIED, &rb_update);
        return FAILURE;
    }

    *assigned_order_id = order_id;
    printf("Table %d booked — order %d created for %s\n", table_id, order_id, name);
    return SUCCESS;
}




static void send_menu(int fd) {
    struct menu items[1000];
    int count = 0;

    if (get_all_menu_items(items, &count) != SUCCESS) {
        send(fd, "ERROR Failed to fetch menu\n", 27, 0);
        return;
    }

    char json[65536];
    int  pos = 0;

    pos += snprintf(json + pos, sizeof(json) - pos, "OK [");

    for (int i = 0; i < count; i++) {
        pos += snprintf(json + pos, sizeof(json) - pos,
                        "{\"itemID\":%d,\"itemName\":\"%s\",\"price\":%.2f}%s",
                        items[i].itemID,
                        items[i].itemName,
                        items[i].price,
                        i < count - 1 ? "," : "");
    }

    pos += snprintf(json + pos, sizeof(json) - pos, "]\n");

    send(fd, json, pos, 0);
}





static void send_tables(int fd) {
    struct tables t[1000];
    int count = 0;

    if (get_all_tables(t, &count) != SUCCESS) {
        send(fd, "ERROR Failed to fetch tables\n", 29, 0);
        return;
    }

    char json[65536];
    int  pos = 0;

    pos += snprintf(json + pos, sizeof(json) - pos, "OK [");
    for (int i = 0; i < count; i++) {
        pos += snprintf(json + pos, sizeof(json) - pos,
                        "{\"tableID\":%d,\"capacity\":%d,\"isOccupied\":%d}%s",
                        t[i].tableID,
                        t[i].capacity,
                        t[i].isOccupied,
                        i < count - 1 ? "," : "");
    }
    pos += snprintf(json + pos, sizeof(json) - pos, "]\n");
    send(fd, json, pos, 0);
}




static void send_order(int fd, int order_id) {
    struct orders o;

    if (db_client_read_order(&DbClient, order_id, &o) != SUCCESS) {
        send(fd, "ERROR ORDER_NOT_FOUND\n", 22, 0);
        return;
    }

    char json[65536];
    int  pos = 0;

    pos += snprintf(json + pos, sizeof(json) - pos,
                    "OK {\"orderID\":%d,\"tableID\":%d,\"name\":\"%s\","
                    "\"phone\":\"%s\",\"email\":\"%s\","
                    "\"orderDate\":\"%s\",\"orderTime\":\"%s\","
                    "\"totalBill\":%.2f,\"isBillGenerated\":%d,\"items\":[",
                    o.orderID, o.tableID, o.name,
                    o.phone,   o.email,
                    o.orderDate, o.orderTime,
                    o.totalBill, o.isBillGenerated);

    for (int i = 0; i < o.num_items; i++) {
        pos += snprintf(json + pos, sizeof(json) - pos,
                        "{\"itemID\":%d,\"itemName\":\"%s\","
                        "\"price\":%.2f,\"quantity\":%d,\"isPrepared\":%d}%s",
                        o.items[i].item.itemID,
                        o.items[i].item.itemName,
                        o.items[i].item.price,
                        o.items[i].quantity,
                        o.items[i].is_prepared,
                        i < o.num_items - 1 ? "," : "");
    }

    pos += snprintf(json + pos, sizeof(json) - pos, "]}\n");
    send(fd, json, pos, 0);
}



















void guest_handler(int client_fd) {

    printf("guest handler callerd\n");

    int orderID=-1;

    while(1){
        char buffer[1024];
        int recv_output = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if(recv_output == -1) {
            close(client_fd);
            printf("SERVER: recv Failed ---> Connection closed (fd=%d)\n", client_fd);
            return;
        }
        if(recv_output == 0) {
            printf("SERVER: Client disconnected (fd=%d)\n", client_fd);
            close(client_fd);
            return;
        }
        buffer[recv_output-1] = '\0';

        printf("Received from client (fd=%d): %s\n", client_fd, buffer);


        if(strcmp(buffer, "GET_MENU") == 0) {
            send_menu(client_fd);
        }
        
        
        else if(strcmp(buffer, "GET_TABLES") == 0) {
            send_tables(client_fd);
        }


        else if(strncmp(buffer,"GET_ORDER",9) == 0){
            
            char* token = strtok(buffer," ");
            token = strtok(NULL," ");
            
            if(token == NULL){
                send(client_fd,"ERROR Missing order_id\n",23,0);
                continue;
            }

            int order_id = atoi(token);
            
            send_order(client_fd,order_id);

        }
        else
        {
            char* response = "ERROR Unknown command\n";
            send(client_fd, response, strlen(response), 0);
   
        }
    }
}