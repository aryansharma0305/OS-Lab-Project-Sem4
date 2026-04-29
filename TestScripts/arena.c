#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DB_Service/DB_Client/DB_Client.h"
#include "DB_Service/structs.h"


int main () {

    struct db_client DbClient;

    if (db_client_init(&DbClient) != SUCCESS) {
        printf("Failed to connect to DB service\n");
        return 1;
    }



    struct orders o1;

    db_client_read_order(&DbClient, 2, &o1);

    // print all details of o1
    printf("OrderID: %d\n", o1.orderID);
    printf("Name: %s\n", o1.name);
    printf("Phone: %s\n", o1.phone);
    printf("Email: %s\n", o1.email);
    printf("TableID: %d\n", o1.tableID);
    printf("Order Date: %s\n", o1.orderDate);
    printf("Order Time: %s\n", o1.orderTime);
    printf("Total Bill: %.2f\n", o1.totalBill);
    printf("Is Bill Generated: %d\n", o1.isBillGenerated);  
    printf("Items:\n");
    for (int i = 0; i < o1.num_items; i++) {
        printf("  Item %d:\n", i + 1);
        printf("    ItemID: %d\n", o1.items[i].item.itemID);
        printf("    Item Name: %s\n", o1.items[i].item.itemName);
        printf("    Price: %.2f\n", o1.items[i].item.price);
        printf("    Quantity: %d\n", o1.items[i].quantity);
        printf("    Is Prepared: %d\n", o1.items[i].is_prepared);
    }
 

}