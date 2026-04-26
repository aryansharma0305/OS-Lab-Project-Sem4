#ifndef CHEF_HANDLER_H
#define CHEF_HANDLER_H

#include "../Server.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>


void chef_handler(int client_fd);


#endif