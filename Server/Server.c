
#include "Server.h"


struct db_client DbClient;



void* handle_client(void* arg) {

    struct client_args* ca = (struct client_args*)arg;
    
    int client_fd          = ca->client_fd;
    char client_ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &ca->client_addr.sin_addr, client_ip, sizeof(client_ip));
    int client_port = ntohs(ca->client_addr.sin_port);
    
    free(ca);   //freeing the CA

    printf("SERVER: New connection from %s:%d (fd=%d)\n", client_ip, client_port, client_fd);


    // recv

    char buffer [1024];


    int recv_output = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if(recv_output == -1) {
        close(client_fd);
        printf("SERVER: recv Failed ---> Connection closed %s:%d (fd=%d)\n",client_ip, client_port, client_fd);
        return NULL;
    }

    if(recv_output == 0) {
        printf("SERVER: Client disconnected %s:%d (fd=%d)\n",client_ip, client_port, client_fd);
        close(client_fd);
        return NULL;
    }

    buffer[recv_output-1] = '\0';

    printf("SERVER: Received from %s:%d (fd=%d): %s\n",client_ip, client_port, client_fd, buffer);
    
    // now I am expecting AUTH <role> <username> <password>    { if role == GUEST no need for username and password }

    char* token = strtok(buffer, " ");

    if(token == NULL) {
        char* response = "ERROR Invalid command format\n";
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
        return NULL;
        
    }

    if(strcmp(token, "AUTH") == 0) {

        char* role = strtok(NULL, " ");
        char* username = strtok(NULL, " ");
        char* password = strtok(NULL, " ");

        if(role == NULL) {
            char* response = "ERROR Missing role\n";
            send(client_fd, response, strlen(response), 0);
            close(client_fd);
            return NULL;
            
        }

        if(strcmp(role, "GUEST") == 0) {
            char* response = "OK Authenticated as GUEST\n";
            send(client_fd, response, strlen(response), 0);
            guest_handler(client_fd);
        }


        
        if(username == NULL || password == NULL) {
            char* response = "ERROR Missing username or password\n";
            send(client_fd, response, strlen(response), 0);
            close(client_fd);
            return NULL;
            
        }

        if(strcmp(role, "ADMIN") == 0) {
            if(authenticate_admin(username, password) == 0) {
                char* response = "OK Authenticated as ADMIN\n";
                send(client_fd, response, strlen(response), 0);
                admin_handler(client_fd);
            } else {
                char* response = "ERROR Authentication failed\n";
                send(client_fd, response, strlen(response), 0);
                close(client_fd);
                return NULL;
            }
        }
        else if(strcmp(role, "CHEF") == 0) {
            if(authenticate_chef(username, password) == 0) {
                char* response = "OK Authenticated as CHEF\n";
                send(client_fd, response, strlen(response), 0);
                chef_handler(client_fd);
            } else {
                char* response = "ERROR Authentication failed\n";
                send(client_fd, response, strlen(response), 0);
                close(client_fd);
                return NULL;
            }
        }
        else {
            char* response = "ERROR Unknown role\n";
            send(client_fd, response, strlen(response), 0);
            close(client_fd);
            return NULL;
        }



    } else {
        char* response = "ERROR Unknown command\n";
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
        return NULL;

    }


    close(client_fd);
    return NULL;
}













int main() {

    
    if (db_client_init(&DbClient) != SUCCESS) {
        fprintf(stderr, "SERVER: Failed to initialize DB client"
                        " — is DB engine running?\n");
        return EXIT_FAILURE;
    }

    printf("SERVER: DB client initialized\n");

    

    // main listening socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("SERVER: socket failed");
        return EXIT_FAILURE;
    }

    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("SERVER: setsockopt failed");
        close(server_fd);
        return EXIT_FAILURE;
    }




    struct sockaddr_in server_addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port        = htons(PORT)
    };


    // binding basically attachiung the socket to a port 
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("SERVER: bind failed");
        close(server_fd);
        return EXIT_FAILURE;
    }




    // m,aking the socket listen to backlog of 16 connections in Q    
    if (listen(server_fd, BACKLOG) == -1) {
        perror("SERVER: listen failed");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("SERVER: Listening on port %d\n", PORT);





    // accepting connections and spawining threads.
    while (1) {

        struct sockaddr_in client_addr;

        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd,(struct sockaddr*)&client_addr,&client_len);
        
        if (client_fd == -1) {
            perror("SERVER: accept failed");
            continue;  
        }




        // maing client args
        struct client_args* ca = malloc(sizeof(struct client_args));

        if (ca == NULL) {
            perror("SERVER: malloc failed for client_args");
            close(client_fd);
            continue;
        }

        ca->client_fd   = client_fd;
        ca->client_addr = client_addr;






        // BASICALLY if I dont do the detached state, the thread will wait for PthreadJOIN on main thread and sit there in memeory wasting resources. 

        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create(&tid, &attr, handle_client, ca) != 0) {
            perror("SERVER: pthread_create failed");
            free(ca);
            close(client_fd);
        }

        pthread_attr_destroy(&attr);

    }

    close(server_fd);
    db_client_destroy(&DbClient);
    return 0;
}