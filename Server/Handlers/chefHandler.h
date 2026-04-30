#ifndef CHEF_HANDLER_H
#define CHEF_HANDLER_H

#include "../Server.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

extern pthread_mutex_t chef_fds_mutex;
extern int chef_fds[1000];
extern int num_chefs;
extern struct db_client DbClient;

void chef_handler(int client_fd);


#endif