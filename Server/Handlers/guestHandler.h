#ifndef GUEST_HANDLER_H
#define GUEST_HANDLER_H

#include "../Server.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>




// what all I want 
// view menu
// place order
// generate bill
// view order status
// 
 
extern struct db_client DbClient;

void guest_handler(int client_fd);


#endif