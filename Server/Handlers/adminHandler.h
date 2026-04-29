#ifndef ADMIN_HANDLER_H
#define ADMIN_HANDLER_H

#include "../Server.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>


// REQ: GET_MENU
// RES: OK [ {itemID, itemName, price}, ... ]

// REQ: GET_TABLES
// RES: OK [ {tableID, capacity, isOccupied}, ... ]

// REQ: GET_ORDER
// RES: OK {orderID, tableID, name, phone, email, orderDate, orderTime, totalBill, isBillGenerated, items:[...]}

// REQ: BOOK_TABLE <table_id> <name> <phone> <email>
// RES: OK <order_id>

// REQ: ORDER_ITEM <item_id> <quantity>
// RES: OK ITEM_ADDED

// REQ: GENERATE_BILL
// RES: OK <total_amount>

// REQ: LOGOUT
// RES: OK Goodbye!

void admin_handler(int client_fd);


#endif