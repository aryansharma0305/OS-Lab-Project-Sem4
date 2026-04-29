


#include <stdio.h>
#include <string.h>

#include "../DB_Service/DB_Client/DB_Client.h"
#include "../DB_Service/structs.h"

struct db_client DbClient;


void add_users() {
    struct users u;

    u.role = ROLE_ADMIN;
    strcpy(u.username, "admin1"); strcpy(u.password, "admin123");
    db_client_insert_user(&DbClient, &u, NULL);

    strcpy(u.username, "admin2"); strcpy(u.password, "admin456");
    db_client_insert_user(&DbClient, &u, NULL);

    u.role = ROLE_CHEF;
    strcpy(u.username, "chef1"); strcpy(u.password, "chef123");
    db_client_insert_user(&DbClient, &u, NULL);

    strcpy(u.username, "chef2"); strcpy(u.password, "chef456");
    db_client_insert_user(&DbClient, &u, NULL);

    u.role = ROLE_WAITER;
    strcpy(u.username, "waiter1"); strcpy(u.password, "waiter123");
    db_client_insert_user(&DbClient, &u, NULL);

    strcpy(u.username, "waiter2"); strcpy(u.password, "waiter456");
    db_client_insert_user(&DbClient, &u, NULL);

    printf("Users: 2 admins, 2 chefs, 2 waiters\n");
}


void add_menu() {
    struct menu items[] = {
        { .itemName = "Margherita Pizza",  .price = 12.99 },
        { .itemName = "Pepperoni Pizza",   .price = 14.99 },
        { .itemName = "Pasta Carbonara",   .price = 11.99 },
        { .itemName = "Pasta Arrabiata",   .price = 10.99 },
        { .itemName = "Caesar Salad",      .price =  8.99 },
        { .itemName = "Garlic Bread",      .price =  4.99 },
        { .itemName = "Grilled Chicken",   .price = 16.99 },
        { .itemName = "Beef Burger",       .price = 13.99 },
        { .itemName = "French Fries",      .price =  3.99 },
        { .itemName = "Chocolate Lava Cake",.price =  6.99 },
        { .itemName = "Tiramisu",          .price =  5.99 },
        { .itemName = "Coke",              .price =  2.49 },
        { .itemName = "Fresh Juice",       .price =  3.49 },
        { .itemName = "Water",             .price =  1.00 },
    };

    int n = sizeof(items) / sizeof(items[0]);
    for (int i = 0; i < n; i++) {
        int key = -1;
        db_client_insert_menu(&DbClient, &items[i], &key);
    }

    printf("Menu: %d items added\n", n);
}


void add_tables() {
    struct tables t;
    int capacities[] = { 2, 2, 4, 4, 4, 8, 8, 12 };
    int n = sizeof(capacities) / sizeof(capacities[0]);

    for (int i = 0; i < n; i++) {
        t.capacity   = capacities[i];
        t.isOccupied = 0;
        int key = -1;
        db_client_insert_table(&DbClient, &t, &key);
    }

    printf("Tables: %d tables added\n", n);
}


void add_orders() {

    // --- Order 1: Table 0, 2 items ---
    struct orders o1;
    memset(&o1, 0, sizeof(o1));
    strcpy(o1.name,      "Alice Johnson");
    strcpy(o1.phone,     "9876543210");
    strcpy(o1.email,     "alice@example.com");
    o1.tableID         = 0;
    o1.isBillGenerated = 0;
    strcpy(o1.orderDate, "2024-01-15");
    strcpy(o1.orderTime, "12:30:00");

    struct menu m;
    db_client_read_menu(&DbClient, 0, &m);  // Margherita Pizza
    o1.items[0].item     = m;
    o1.items[0].quantity = 1;
    o1.totalBill        += m.price * 1;

    db_client_read_menu(&DbClient, 11, &m); // Coke
    o1.items[1].item     = m;
    o1.items[1].quantity = 2;
    o1.totalBill        += m.price * 2;

    o1.num_items = 2;

    int key1 = -1;
    db_client_insert_order(&DbClient, &o1, &key1);

    // Mark table 0 as occupied
    struct tables match0  = { .tableID = 0, .isOccupied = 0 };
    struct tables update0 = { .tableID = 0, .capacity = 2, .isOccupied = 1 };
    db_client_find_and_update_table(&DbClient, &match0,
                                    TABLES_MATCH_TABLE_ID | TABLES_MATCH_IS_OCCUPIED,
                                    &update0);

    // --- Order 2: Table 1, 3 items ---
    struct orders o2;
    memset(&o2, 0, sizeof(o2));
    strcpy(o2.name,      "Bob Smith");
    strcpy(o2.phone,     "1234567890");
    strcpy(o2.email,     "bob@example.com");
    o2.tableID         = 1;
    o2.isBillGenerated = 0;
    strcpy(o2.orderDate, "2024-01-15");
    strcpy(o2.orderTime, "13:00:00");

    db_client_read_menu(&DbClient, 6, &m);  // Grilled Chicken
    o2.items[0].item     = m;
    o2.items[0].quantity = 1;
    o2.totalBill        += m.price * 1;

    db_client_read_menu(&DbClient, 8, &m);  // French Fries
    o2.items[1].item     = m;
    o2.items[1].quantity = 1;
    o2.totalBill        += m.price * 1;

    db_client_read_menu(&DbClient, 12, &m); // Fresh Juice
    o2.items[2].item     = m;
    o2.items[2].quantity = 1;
    o2.totalBill        += m.price * 1;

    o2.num_items = 3;

    int key2 = -1;
    db_client_insert_order(&DbClient, &o2, &key2);

    struct tables match1  = { .tableID = 1, .isOccupied = 0 };
    struct tables update1 = { .tableID = 1, .capacity = 2, .isOccupied = 1 };
    db_client_find_and_update_table(&DbClient, &match1,
                                    TABLES_MATCH_TABLE_ID | TABLES_MATCH_IS_OCCUPIED,
                                    &update1);

    // --- Order 3: Table 2, bill already generated ---
    struct orders o3;
    memset(&o3, 0, sizeof(o3));
    strcpy(o3.name,      "Carol White");
    strcpy(o3.phone,     "5554443333");
    strcpy(o3.email,     "carol@example.com");
    o3.tableID         = 2;
    o3.isBillGenerated = 1;   // already paid
    strcpy(o3.orderDate, "2024-01-15");
    strcpy(o3.orderTime, "11:00:00");

    db_client_read_menu(&DbClient, 2, &m);  // Pasta Carbonara
    o3.items[0].item     = m;
    o3.items[0].quantity = 2;
    o3.totalBill        += m.price * 2;

    db_client_read_menu(&DbClient, 9, &m);  // Chocolate Lava Cake
    o3.items[1].item     = m;
    o3.items[1].quantity = 2;
    o3.totalBill        += m.price * 2;

    o3.num_items = 2;

    int key3 = -1;
    db_client_insert_order(&DbClient, &o3, &key3);
    // table 2 stays free — bill was already generated

    printf("Orders: 3 orders added (keys %d, %d, %d)\n", key1, key2, key3);
    printf("  Order %d: Alice  — table 0 — $%.2f — active\n",   key1, o1.totalBill);
    printf("  Order %d: Bob    — table 1 — $%.2f — active\n",   key2, o2.totalBill);
    printf("  Order %d: Carol  — table 2 — $%.2f — paid\n",     key3, o3.totalBill);
}


int main() {

    if (db_client_init(&DbClient) != SUCCESS) {
        printf("Failed to connect to DB service\n");
        return 1;
    }

    printf("\n=== Populating Database ===\n\n");

    add_users();
    add_menu();
    add_tables();
    add_orders();

    printf("\n=== Done ===\n");

    db_client_destroy(&DbClient);
    return 0;
}