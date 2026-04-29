#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../DB_Service/DB_Client/DB_Client.h"
#include "../DB_Service/structs.h"

#define OUT_BUF_SIZE (1024 * 1024)  // 1 MB buffer

static void append(char *buf, size_t *pos, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    *pos += vsnprintf(buf + *pos, OUT_BUF_SIZE - *pos, fmt, args);
    va_end(args);
}

void print_orders(struct db_client* client, char *buf, size_t *pos) {
    append(buf, pos, "\n==================== ORDERS ====================\n");
    append(buf, pos, "%-8s %-12s %-15s %-20s %-6s %-10s %-12s %-10s\n",
           "ID", "Name", "Phone", "Email", "Tbl", "Time", "Date", "Bill");

    for (int key = 0; key < 1000; key++) {
        struct orders rec;
        if (db_client_read_order(client, key, &rec) != SUCCESS)
            continue;

        append(buf, pos, "%-8d %-12s %-15s %-20s %-6d %-10s %-12s %-10.2f\n",
               rec.orderID, rec.name, rec.phone, rec.email,
               rec.tableID, rec.orderTime, rec.orderDate, rec.totalBill);
    }
}

void print_menu(struct db_client* client, char *buf, size_t *pos) {
    append(buf, pos, "\n==================== MENU ====================\n");
    append(buf, pos, "%-8s %-20s %-10s\n", "ID", "Name", "Price");

    for (int key = 0; key < 1000; key++) {
        struct menu rec;
        if (db_client_read_menu(client, key, &rec) != SUCCESS)
            continue;

        append(buf, pos, "%-8d %-20s %-10.2f\n",
               rec.itemID, rec.itemName, rec.price);
    }
}

void print_tables(struct db_client* client, char *buf, size_t *pos) {
    append(buf, pos, "\n==================== TABLES ====================\n");
    append(buf, pos, "%-8s %-10s %-10s\n", "ID", "Capacity", "Occupied");

    for (int key = 0; key < 1000; key++) {
        struct tables rec;
        if (db_client_read_table(client, key, &rec) != SUCCESS)
            continue;

        append(buf, pos, "%-8d %-10d %-10d\n",
               rec.tableID, rec.capacity, rec.isOccupied);
    }
}

void print_users(struct db_client* client, char *buf, size_t *pos) {
    append(buf, pos, "\n==================== USERS ====================\n");
    append(buf, pos, "%-8s %-20s %-20s %-10s\n", "ID", "Username", "Password", "Role");

    for (int key = 0; key < 1000; key++) {
        struct users rec;
        if (db_client_read_user(client, key, &rec) != SUCCESS)
            continue;

        const char* role_str;
        switch (rec.role) {
            case ROLE_ADMIN:  role_str = "ADMIN"; break;
            case ROLE_WAITER: role_str = "WAITER"; break;
            case ROLE_CHEF:   role_str = "CHEF"; break;
            default:          role_str = "UNKNOWN";
        }

        append(buf, pos, "%-8d %-20s %-20s %-10s\n",
               rec.userID, rec.username, rec.password, role_str);
    }
}

int main() {
    struct db_client client;

    if (db_client_init(&client) != SUCCESS) {
        printf("Failed to connect to DB service\n");
        return 1;
    }

    char *buffer = malloc(OUT_BUF_SIZE);
    size_t pos = 0;

    print_orders(&client, buffer, &pos);
    print_menu(&client, buffer, &pos);
    print_tables(&client, buffer, &pos);
    print_users(&client, buffer, &pos);

    // 🔥 print once at the end
    printf("%s", buffer);

    free(buffer);
    db_client_destroy(&client);
    return 0;
}