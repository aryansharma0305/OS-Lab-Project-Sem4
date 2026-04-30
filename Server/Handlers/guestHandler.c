#include "guestHandler.h"

extern struct db_client DbClient;

#include "chefHandler.h"


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
    printf("Sent tables data to client (fd=%d) %s\n", fd, json);
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
                        "{\"orderItemID\":%d,\"itemID\":%d,\"itemName\":\"%s\","
                        "\"price\":%.2f,\"quantity\":%d,\"isPrepared\":%d}%s",
                        o.items[i].orderItemID,
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




static void order_item(int fd, int order_id, int item_id, int quantity) {

    if (quantity <= 0) {
        send(fd, "ERROR Invalid quantity\n", 23, 0);
        return;
    }

    struct orders o;
    int status = db_client_read_order(&DbClient, order_id, &o);
    
    if (status != SUCCESS) {
        send(fd, "ERROR ORDER_NOT_FOUND\n", 22, 0);
        return;
    }

    if (o.num_items >= DB_MAX_ORDER_ITEMS) {
        send(fd, "ERROR Order has maximum items already\n", 38, 0);
        return;
    }

    struct menu m;
    status = db_client_read_menu(&DbClient, item_id, &m);
    if (status != SUCCESS) {
        send(fd, "ERROR ITEM_NOT_FOUND\n", 21, 0);
        return;
    }

    int idx = o.num_items;  

    o.items[idx].orderItemID = idx;
    o.items[idx].item        = m;
    o.items[idx].quantity    = quantity;
    o.items[idx].is_prepared = 0;

    o.num_items++;
    o.totalBill += (m.price * quantity);

    status = db_client_update_order(&DbClient, order_id, &o);



    if (status != SUCCESS) {
        send(fd, "ERROR Failed to update order\n", 29, 0);
        return;
    }


    //broadcast to chefs that a new item has been added (so they can update dashboard)
    char message[256];
    snprintf(message, sizeof(message), "NEW_UPDATE %d %d %s %d PENDING\n",
             o.orderID, idx, m.itemName, o.tableID);
    broadcast_to_chefs(message);

    send(fd, "OK ITEM_ADDED\n", 14, 0);
}

static void generate_bill(int fd, int order_id) {

    struct orders o;
    int status = db_client_read_order(&DbClient, order_id, &o);

    if (status != SUCCESS) {
        send(fd, "ERROR ORDER_NOT_FOUND\n", 22, 0);
        return;
    }

    if (o.isBillGenerated) {
        send(fd, "ERROR Bill already generated\n", 29, 0);
        return;
    }

    float total = 0.0f;

    for (int i = 0; i < o.num_items; i++) {
        total += o.items[i].item.price * o.items[i].quantity;
    }

    // update order
    o.totalBill = total;
    o.isBillGenerated = 1;


    // also broadcast to chefs that these items are now prepared (so they can remove from dashboard)
    char message[256];
    for (int i = 0; i < o.num_items; i++) {
        if(o.items[i].is_prepared) continue; 
        o.items[i].is_prepared = 1; // mark as prepared before broadcasting
        snprintf(message, sizeof(message), "NEW_UPDATE %d %d %s %d PREPARED\n",
                 o.orderID, o.items[i].orderItemID, o.items[i].item.itemName, o.tableID);
        broadcast_to_chefs(message);
        
    }


    status = db_client_update_order(&DbClient, order_id, &o);
    if (status != SUCCESS) {
        send(fd, "ERROR Failed to update order\n", 29, 0);
        return;
    }

    // free table
    struct tables match  = { .tableID = o.tableID, .isOccupied = 1 };
    struct tables update = { .tableID = o.tableID, .isOccupied = 0 };

    status = db_client_find_and_update_table(
        &DbClient,
        &match,
        TABLES_MATCH_TABLE_ID | TABLES_MATCH_IS_OCCUPIED,
        &update
    );
 

    if (status != SUCCESS) {
        send(fd, "ERROR Failed to free table\n", 27, 0);
        return;
    }

    // send minimal response
    char response[64];
    snprintf(response, sizeof(response), "OK %.2f\n", total);
    send(fd, response, strlen(response), 0);
}




void guest_handler(int client_fd) {

    printf("guest handler callerd\n");

    int orderID=-1;

    while(1){
        char buffer[1024];
        int recv_output = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if(recv_output == -1) {
            close(client_fd);
            if(orderID != -1) generate_bill(client_fd, orderID);
            printf("SERVER: recv Failed ---> Connection closed (fd=%d)\n", client_fd);
            return;
        }
        if(recv_output == 0) {
            if(orderID != -1) generate_bill(client_fd, orderID);
            printf("SERVER:: Client disconnected (fd=%d)\n", client_fd);
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
            
            send_order(client_fd,orderID);

        }

        else if (strncmp(buffer,"BOOK_TABLE",10) == 0) {

            if(orderID != -1){
                send(client_fd,"ERROR You already have an active order. Cannot book another table.\n",70,0);
                continue;
            }
           
            char* token = strtok(buffer," ");
            token = strtok(NULL," ");
            
            if(token == NULL){
                send(client_fd,"ERROR Missing table_id\n",23,0);
                continue;
            }

            int table_id = atoi(token);

            token = strtok(NULL," ");
            if(token == NULL){
                send(client_fd,"ERROR Missing name\n",20,0);
                continue;
            }
            char* name = token;

            token = strtok(NULL," ");
            if(token == NULL){
                send(client_fd,"ERROR Missing phone\n",21,0);
                continue;
            }
            char* phone = token;

            token = strtok(NULL," ");
            if(token == NULL){
                send(client_fd,"ERROR Missing email\n",21,0);
                continue;
            }
            char* email = token;

            int assigned_order_id=-1;
            int status = book_table_and_create_order(table_id,name,phone,email,&assigned_order_id);

            if(status != SUCCESS){
                send(client_fd,"ERROR Failed to book table and create order\n",45,0);
                continue;
            }


            orderID=assigned_order_id;

            char response[64];
            snprintf(response, sizeof(response), "OK %d\n", assigned_order_id);
            send(client_fd, response, strlen(response), 0);


        }


        else if(strncmp(buffer,"ORDER_ITEM",10) == 0){  // ORDER_ITEM <item_id> <quantity>
            
            if(orderID == -1){
                send(client_fd,"ERROR No active order. Please book a table first.\n",50,0);
                continue;
            }

            char* token = strtok(buffer," ");
            token = strtok(NULL," ");
            
            if(token == NULL){
                send(client_fd,"ERROR Missing item_id\n",22,0);
                continue;
            }

            int item_id = atoi(token);

            token = strtok(NULL," ");
            if(token == NULL){
                send(client_fd,"ERROR Missing quantity\n",24,0);
                continue;
            }
            int quantity = atoi(token);
            
            order_item(client_fd,orderID,item_id,quantity);

        }

        else if (strncmp(buffer, "GENERATE_BILL", 13) == 0) {
            if (orderID == -1) {
                send(client_fd, "ERROR No active order. Please book a table first.\n", 52, 0);
                continue;
            }

            generate_bill(client_fd, orderID);
        }
        else if (strcmp(buffer, "LOGOUT") == 0) {
            send(client_fd, "OK Goodbye!\n", 12, 0);
            if(orderID != -1){
                generate_bill(client_fd, orderID);
            }
            close(client_fd);
            printf("SERVER : Client requested to logout (fd=%d)\n", client_fd);
            return;
        }
        else
        {
            char* response = "ERROR Unknown command\n";
            send(client_fd, response, strlen(response), 0);
   
        }
    }
}