#include "guestHandler.h"


int get_all_tables(struct db_client* client, struct tables* tables_array, int* count_out) {

    int count = 0;

    for (int key = 0; key < 1000; key++) {

        struct tables t;
        int status = db_client_read_table(client, key, &t);

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


int get_all_menu_items(struct db_client* client, struct menu* menu_array, int* count_out) {

    int count = 0;

    for (int key = 0; key < 1000; key++) {

        struct menu m;
        int status = db_client_read_menu(client, key, &m);

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


void guest_handler(int client_fd) {

    printf("guest handler callerd\n");

    while(1){
        char buffer[1024];
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0) {
            printf("Client disconnected or error occurred\n");
            break;
        }

        buffer[bytes_received] = '\0'; // Null-terminate the received data

        printf("Received from client: %s\n", buffer);

        // send back the same message to the client with OK
        char response[1050];
        snprintf(response, sizeof(response), "OK %s", buffer);
        send(client_fd, response, strlen(response), 0);
    }
}