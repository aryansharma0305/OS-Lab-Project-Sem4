#ifndef SERVER_H
#define SERVER_H



#include "Utils/Auth.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../DB_Service/DB_Client/DB_Client.h"
#include "../DB_Service/structs.h"
#include "./Handlers/guestHandler.h"
#include "./Handlers/adminHandler.h"
#include "./Handlers/chefHandler.h"

#include <pthread.h>



#define PORT            8080
#define MAX_CONNECTIONS 128
#define BACKLOG         16



struct client_args {
    int         client_fd;
    struct      sockaddr_in client_addr;
};



#endif