#include "chefHandler.h"    



void broadcast_to_chefs(const char* message) {   // something like NEW_UPDATE <orderID> <ItemORDERID>  <itemName> <tableID> <status>
    pthread_mutex_lock(&chef_fds_mutex);
    for (int i = 0; i < num_chefs; i++) {
        send(chef_fds[i], message, strlen(message), MSG_DONTWAIT);
    }
    pthread_mutex_unlock(&chef_fds_mutex);
}



static void send_chef_dashboard(int fd){

    char response[16384];
    int offset = 0;

    offset += sprintf(response + offset, "OK [");

    int first = 1;

    for(int key = 0; key < 1000; key++){

        struct orders o;

        int status = db_client_read_order(&DbClient, key, &o);

        if(status == FAILURE){
            return;
        }

        if(status != SUCCESS) continue;

        for(int i = 0; i < o.num_items; i++){

            if(o.items[i].is_prepared) continue;

            struct menu m;
            int menu_status = db_client_read_menu(&DbClient, o.items[i].item.itemID, &m);

            if(menu_status != SUCCESS) continue;

            if(!first){
                offset += sprintf(response + offset, ",");
            }
            first = 0;

            offset += sprintf(response + offset,
                "{\"orderID\":%d,"
                "\"orderItemID\":%d,"
                "\"itemName\":\"%s\","
                "\"tableID\":%d}",
                o.orderID,
                o.items[i].orderItemID,
                m.itemName,
                o.tableID
            );
        }
    }

    offset += sprintf(response + offset, "]\n");

    send(fd, response, strlen(response), 0);
}




static void mark_prepared(int orderID, int orderItemID){

    struct orders o;

    int status = db_client_read_order(&DbClient, orderID, &o);

    if(status != SUCCESS){
        fprintf(stderr, "Failed to read order in mark_prepared\n");
        return;
    }

    int found = 0;

    for(int i = 0; i < o.num_items; i++){
        if(o.items[i].orderItemID == orderItemID){
            o.items[i].is_prepared = 1;
            found = 1;
            break;
        }
    }

    if(!found){
        fprintf(stderr, "Order item not found in mark_prepared\n");
        return;
    }

    status = db_client_update_order(&DbClient, orderID, &o);

    if(status != SUCCESS){
        fprintf(stderr, "Failed to update order in mark_prepared\n");
        return;
    }

    char message[256];
    snprintf(message, sizeof(message), "NEW_UPDATE %d %d %s %d PREPARED\n",
             orderID, orderItemID, o.items[orderItemID].item.itemName, o.tableID);
    
    broadcast_to_chefs(message);
}





void chef_handler(int client_fd) {

    printf("chef handler called\n");

    while(1){
        char buffer[1024];
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0) {
            printf("Client disconnected or error occurred\n");
            break;
        }

        buffer[bytes_received] = '\0'; // Null-terminate the received data

        printf("Received from client: %s\n", buffer);

        if(strncmp(buffer, "GET_DASHBOARD", 13) == 0){
            send_chef_dashboard(client_fd);
        }
        else if (strncmp(buffer, "MARK_PREPARED",13)==0){  // MARK_PREPARED <orderID> <orderItemID>


            char* token = strtok(buffer, " ");
            token = strtok(NULL, " ");
            if(token == NULL){
                send(client_fd, "ERROR Missing orderID\n", 22, 0);
                continue;
            }
            int orderID = atoi(token);

            token = strtok(NULL, " ");
            if(token == NULL){
                send(client_fd, "ERROR Missing orderItemID\n", 26, 0);
                continue;
            }
            int orderItemID = atoi(token);

           mark_prepared(orderID, orderItemID);

        }
        else{
            send(client_fd, "ERROR Unknown command\n", 22, 0);
        }
    }
}