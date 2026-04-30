#include "adminHandler.h"

extern struct db_client DbClient;

// ======================= HELPERS =======================

static void send_tables(int fd) {
    char json[65536];
    int pos = 0;

    pos += snprintf(json + pos, sizeof(json) - pos, "OK [");

    int first = 1;

    for (int key = 0; key < 1000; key++) {
        struct tables t;
        int status = db_client_read_table(&DbClient, key, &t);

        if (status == FAILURE) return;
        if (status != SUCCESS) continue;

        if (!first) pos += snprintf(json + pos, sizeof(json) - pos, ",");
        first = 0;

        pos += snprintf(json + pos, sizeof(json) - pos,
            "{\"tableID\":%d,\"capacity\":%d,\"isOccupied\":%d}",
            t.tableID, t.capacity, t.isOccupied);
    }

    pos += snprintf(json + pos, sizeof(json) - pos, "]\n");
    send(fd, json, pos, 0);
}

static void send_menu(int fd) {
    char json[65536];
    int pos = 0;

    pos += snprintf(json + pos, sizeof(json) - pos, "OK [");

    int first = 1;

    for (int key = 0; key < 1000; key++) {
        struct menu m;
        int status = db_client_read_menu(&DbClient, key, &m);

        if (status == FAILURE) return;
        if (status != SUCCESS) continue;

        if (!first) pos += snprintf(json + pos, sizeof(json) - pos, ",");
        first = 0;

        pos += snprintf(json + pos, sizeof(json) - pos,
            "{\"itemID\":%d,\"itemName\":\"%s\",\"price\":%.2f}",
            m.itemID, m.itemName, m.price);
    }

    pos += snprintf(json + pos, sizeof(json) - pos, "]\n");
    send(fd, json, pos, 0);
}

static void send_orders(int fd) {
    char json[65536];
    int pos = 0;

    pos += snprintf(json + pos, sizeof(json) - pos, "OK [");

    int first = 1;

    for (int key = 0; key < 1000; key++) {
        struct orders o;
        int status = db_client_read_order(&DbClient, key, &o);

        if (status == FAILURE) return;
        if (status != SUCCESS) continue;

        if (!first) pos += snprintf(json + pos, sizeof(json) - pos, ",");
        first = 0;

        pos += snprintf(json + pos, sizeof(json) - pos,
            "{\"orderID\":%d,\"tableID\":%d,\"totalBill\":%.2f,\"items\":%d}",
            o.orderID, o.tableID, o.totalBill, o.num_items);
    }

    pos += snprintf(json + pos, sizeof(json) - pos, "]\n");
    send(fd, json, pos, 0);
}

// ======================= DELETE OPS =======================

static void delete_table(int fd, int table_id) {
    int status = db_client_delete_table(&DbClient, table_id);

    if (status != SUCCESS) {
        send(fd, "ERROR Failed to delete table\n", 30, 0);
        return;
    }

    send(fd, "OK TABLE_DELETED\n", 17, 0);
}

static void delete_menu(int fd, int item_id) {
    int status = db_client_delete_menu(&DbClient, item_id);

    if (status != SUCCESS) {
        send(fd, "ERROR Failed to delete menu item\n", 34, 0);
        return;
    }

    send(fd, "OK MENU_DELETED\n", 16, 0);
}










#include "chefHandler.h"   // IMPORTANT for broadcast_to_chefs

static void delete_order(int fd, int order_id) {

    struct orders o;

    int status = db_client_read_order(&DbClient, order_id, &o);

    if (status != SUCCESS) {
        send(fd, "ERROR ORDER_NOT_FOUND\n", 22, 0);
        return;
    }

    char message[256];

    for (int i = 0; i < o.num_items; i++) {

        if (o.items[i].is_prepared) continue;

        snprintf(message, sizeof(message),
                 "NEW_UPDATE %d %d %s %d PREPARED\n",
                 o.orderID,
                 o.items[i].orderItemID,
                 o.items[i].item.itemName,
                 o.tableID);

        broadcast_to_chefs(message);
    }

    // 🗑️ DELETE FROM DB
    status = db_client_delete_order(&DbClient, order_id);

    if (status != SUCCESS) {
        send(fd, "ERROR Failed to delete order\n", 30, 0);
        return;
    }

    send(fd, "OK ORDER_DELETED\n", 17, 0);
}


















// ---------- ADD TABLE ----------
static void add_table(int fd, int tableID, int capacity) {

    struct tables t;

    t.tableID = tableID;
    t.capacity = capacity;
    t.isOccupied = 0;

    int a;

    int status = db_client_insert_table(&DbClient, &t,&a);

    if (status != SUCCESS) {
        send(fd, "ERROR Failed to add table\n", 27, 0);
        return;
    }

    send(fd, "OK TABLE_ADDED\n", 15, 0);
}

// ---------- ADD MENU ----------
static void add_menu(int fd, int itemID, char *itemName, float price) {

    struct menu m;

    m.itemID = itemID;
    strncpy(m.itemName, itemName, sizeof(m.itemName) - 1);
    m.itemName[sizeof(m.itemName) - 1] = '\0';
    m.price = price;

    int a;

    int status = db_client_insert_menu(&DbClient, &m,&a);

    if (status != SUCCESS) {
        send(fd, "ERROR Failed to add menu item\n", 31, 0);
        return;
    }

    send(fd, "OK MENU_ADDED\n", 14, 0);
}




// ======================= MAIN HANDLER =======================

void admin_handler(int client_fd) {

    printf("admin handler called\n");

    while (1) {
        char buffer[1024];

        int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes <= 0) {
            printf("Admin disconnected\n");
            close(client_fd);
            return;
        }

        buffer[bytes] = '\0';

        printf("ADMIN CMD: %s\n", buffer);

        // ---------- READ ----------
        if (strncmp(buffer, "GET_TABLES", 10) == 0) {
            send_tables(client_fd);
        }
        else if (strncmp(buffer, "GET_MENU", 8) == 0) {
            send_menu(client_fd);
        }
        else if (strncmp(buffer, "GET_ORDERS", 10) == 0) {
            send_orders(client_fd);
        }

        // ---------- DELETE ----------
        else if (strncmp(buffer, "DELETE_TABLE", 12) == 0) {
            int id;
            if (sscanf(buffer, "DELETE_TABLE %d", &id) != 1) {
                send(client_fd, "ERROR Invalid format\n", 22, 0);
                continue;
            }
            delete_table(client_fd, id);
        }
        else if (strncmp(buffer, "DELETE_MENU", 11) == 0) {
            int id;
            if (sscanf(buffer, "DELETE_MENU %d", &id) != 1) {
                send(client_fd, "ERROR Invalid format\n", 22, 0);
                continue;
            }
            delete_menu(client_fd, id);
        }
        else if (strncmp(buffer, "DELETE_ORDER", 12) == 0) {
            int id;
            if (sscanf(buffer, "DELETE_ORDER %d", &id) != 1) {
                send(client_fd, "ERROR Invalid format\n", 22, 0);
                continue;
            }
            delete_order(client_fd, id);
        }

        else if (strncmp(buffer, "ADD_TABLE", 9) == 0) {
            int id, capacity;

            if (sscanf(buffer, "ADD_TABLE %d %d", &id, &capacity) != 2) {
                send(client_fd, "ERROR Invalid format\n", 22, 0);
                continue;
            }

            add_table(client_fd, id, capacity);
        }

        else if (strncmp(buffer, "ADD_MENU", 8) == 0) {
            int id;
            float price;
            char name[100];

            if (sscanf(buffer, "ADD_MENU %d %[^0-9\n] %f", &id, name, &price) != 3) {
                send(client_fd, "ERROR Invalid format\n", 22, 0);
                continue;
            }

            // trim trailing spaces
            int len = strlen(name);
            while (len > 0 && name[len - 1] == ' ') {
                name[--len] = '\0';
            }

            add_menu(client_fd, id, name, price);
        }

        else {
            send(client_fd, "ERROR Unknown command\n", 22, 0);
        }
    }
}