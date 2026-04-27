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
// flow --> user can VIEW_MENU or BOOKTABLE. 
// BOOKTABLE --> system asks for name phone email
//           --> show all table and the book. --> booking will do two things, create the order if the table selected is available.
//     something like BOOKTABLE <table_id> <name> <phone> <email>
// we also need to store the orderID in the thread while it lives. 
// after placing order, user can do VIEW_ORDER_STATUS which will show the status of the order.
// or it can order more
// generate bill --> show the bill and free the table.
//


// C :  VIEWMENU 
// S :  OK <json menu array>
// C :  BOOKTABLE <table_id> <name> <phone> <email>
// S :  OK <assigned_order_id> or ERROR <error_msg>
// C :  VIEW_ORDER_STATUS 
// S :  OK <json order details> or ERROR <error_msg>
// C :  GENERATE_BILL <order_id>
// S :  OK <bill_amount> or ERROR <error_msg>

 
extern struct db_client DbClient;

void guest_handler(int client_fd);


#endif