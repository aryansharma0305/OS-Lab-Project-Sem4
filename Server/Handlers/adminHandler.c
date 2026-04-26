#include "adminHandler.h"





void admin_handler(int client_fd) {

    printf("admin handler called\n");

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