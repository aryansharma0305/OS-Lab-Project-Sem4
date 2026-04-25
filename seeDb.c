#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "DB_Service/structs.h"
#include "DB_Service/DB_Engine/DB_File_IO_Handler.h"

void print_orders() {
    int fd_data = open("DB_Service/Data/orders.db", O_RDONLY);
    int fd_idx  = open("DB_Service/Data/orders.idx", O_RDONLY);

    if (fd_data < 0 || fd_idx < 0) {
        perror("orders open failed");
        return;
    }

    struct index idx;
    struct orders rec;

    printf("\n==================== ORDERS ====================\n");
    printf("%-8s %-12s %-15s %-20s %-6s %-10s %-12s %-10s\n",
           "ID", "Name", "Phone", "Email", "Tbl", "Time", "Date", "Bill");

    while (read(fd_idx, &idx, sizeof(idx)) == sizeof(idx)) {
        if (idx.is_deleted) continue;

        if (pread(fd_data, &rec, sizeof(rec), idx.offset) != sizeof(rec)) {
            perror("orders read failed");
            continue;
        }

        printf("%-8d %-12s %-15s %-20s %-6d %-10s %-12s %-10.2f\n",
               rec.orderID,
               rec.name,
               rec.phone,
               rec.email,
               rec.tableID,
               rec.orderTime,
               rec.orderDate,
               rec.totalBill);
    }

    close(fd_data);
    close(fd_idx);
}

void print_menu() {
    int fd_data = open("DB_Service/Data/menu.db", O_RDONLY);
    int fd_idx  = open("DB_Service/Data/menu.idx", O_RDONLY);

    if (fd_data < 0 || fd_idx < 0) {
        perror("menu open failed");
        return;
    }

    struct index idx;
    struct menu rec;

    printf("\n==================== MENU ====================\n");
    printf("%-8s %-20s %-10s\n", "ID", "Name", "Price");

    while (read(fd_idx, &idx, sizeof(idx)) == sizeof(idx)) {
        if (idx.is_deleted) continue;

        if (pread(fd_data, &rec, sizeof(rec), idx.offset) != sizeof(rec)) {
            perror("menu read failed");
            continue;
        }

        printf("%-8d %-20s %-10.2f\n",
               rec.itemID,
               rec.itemName,
               rec.price);
    }

    close(fd_data);
    close(fd_idx);
}

void print_tables() {
    int fd_data = open("DB_Service/Data/tables.db", O_RDONLY);
    int fd_idx  = open("DB_Service/Data/tables.idx", O_RDONLY);

    if (fd_data < 0 || fd_idx < 0) {
        perror("tables open failed");
        return;
    }

    struct index idx;
    struct tables rec;

    printf("\n==================== TABLES ====================\n");
    printf("%-8s %-10s %-10s\n", "ID", "Capacity", "Occupied");

    while (read(fd_idx, &idx, sizeof(idx)) == sizeof(idx)) {
        if (idx.is_deleted) continue;

        if (pread(fd_data, &rec, sizeof(rec), idx.offset) != sizeof(rec)) {
            perror("tables read failed");
            continue;
        }

        printf("%-8d %-10d %-10d\n",
               rec.tableID,
               rec.capacity,
               rec.isOccupied);
    }

    close(fd_data);
    close(fd_idx);
}

int main() {
    print_orders();
    print_menu();
    print_tables();
    return 0;
}