#ifndef UI_GUEST_H
#define UI_GUEST_H



#include <stdlib.h>

void print_customer_details_page(int fd, int flag);
void show_tables_menu(int fd, char* name, char* phone, char* email);
void print_customer_dashboard(int fd, int orderID);
void print_order_items_page(fd, orderID);
void print_generate_bill_page(int fd, int orderID);
void print_view_order_details_page(int fd, int orderID);


#endif