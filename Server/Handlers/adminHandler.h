#ifndef ADMIN_HANDLER_H
#define ADMIN_HANDLER_H

#include "../Server.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>


void admin_handler(int client_fd);


#endif