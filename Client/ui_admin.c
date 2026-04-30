#include "ui_admin.h"
#include "ui_general.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

// ---------- HELPERS ----------

static void wait_enter() {
    printf("\n%sPress Enter to continue...%s", BLINKING_YELLOW, RESET);
    getchar();
}

static int recv_response(int fd, char *buffer) {
    int bytes = recv(fd, buffer, 8192 - 1, 0);
    if (bytes <= 0) {
        printf("Disconnected from server\n");
        return -1;
    }
    buffer[bytes] = '\0';
    return 0;
}

// ---------- VIEW TABLES ----------

static void view_tables(int fd) {
    char buffer[8192];

    send(fd, "GET_TABLES\n", strlen("GET_TABLES\n"), 0);

    if (recv_response(fd, buffer) < 0) return;

    ClearScreen();

    printf("%s================ ALL TABLES ================%s\n\n", YELLOW, RESET);

    int tableID, capacity, isOccupied;

    printf("%sTableID   Capacity   Status%s\n", BLUE, RESET);
    printf("-----------------------------------\n");

    char *ptr = strchr(buffer, '{');

    while (ptr) {
        if (sscanf(ptr,
            "{\"tableID\":%d,\"capacity\":%d,\"isOccupied\":%d}",
            &tableID, &capacity, &isOccupied) == 3) {

            printf("%s%-8d %-10d %s%s\n",
                BLUE,
                tableID,
                capacity,
                isOccupied ? RED "Occupied" : GREEN "Available",
                RESET);
        }

        ptr = strchr(ptr + 1, '{');
    }

    wait_enter();
}

// ---------- VIEW MENU ----------

static void view_menu(int fd) {
    char buffer[8192];

    send(fd, "GET_MENU\n", strlen("GET_MENU\n"), 0);

    if (recv_response(fd, buffer) < 0) return;

    ClearScreen();

    printf("%s================ MENU =================%s\n\n", YELLOW, RESET);

    int itemID;
    char itemName[100];
    float price;

    printf("%sID     Name                      Price%s\n", BLUE, RESET);
    printf("------------------------------------------------\n");

    char *ptr = strchr(buffer, '{');

    while (ptr) {
        if (sscanf(ptr,
            "{\"itemID\":%d,\"itemName\":\"%[^\"]\",\"price\":%f}",
            &itemID, itemName, &price) == 3) {

            printf("%s%-6d %-25s ₹ %.2f%s\n",
                   BLUE, itemID, itemName, price, RESET);
        }

        ptr = strchr(ptr + 1, '{');
    }

    wait_enter();
}

// ---------- VIEW ORDERS ----------

static void view_orders(int fd) {
    char buffer[8192];

    send(fd, "GET_ORDERS\n", strlen("GET_ORDERS\n"), 0);

    if (recv_response(fd, buffer) < 0) return;

    ClearScreen();

    printf("%s================ ALL ORDERS ================%s\n\n", YELLOW, RESET);

    int orderID, tableID, items;
    float total;

    printf("%sOrderID   TableID   Items   Total%s\n", BLUE, RESET);
    printf("-------------------------------------------\n");

    char *ptr = strchr(buffer, '{');

    while (ptr) {
        if (sscanf(ptr,
            "{\"orderID\":%d,\"tableID\":%d,\"totalBill\":%f,\"items\":%d}",
            &orderID, &tableID, &total, &items) == 4) {

            printf("%s%-9d %-9d %-7d ₹ %.2f%s\n",
                   BLUE, orderID, tableID, items, total, RESET);
        }

        ptr = strchr(ptr + 1, '{');
    }

    wait_enter();
}

// ---------- DELETE HELPERS ----------

static void delete_table_ui(int fd) {
    int id;
    printf("Enter Table ID: ");
    scanf("%d", &id);
    while(getchar()!='\n');

    char cmd[128], buffer[1024];

    snprintf(cmd, sizeof(cmd), "DELETE_TABLE %d\n", id);
    send(fd, cmd, strlen(cmd), 0);

    if (recv_response(fd, buffer) < 0) return;

    printf("%s%s%s\n", GREEN, buffer, RESET);
    wait_enter();
}

static void delete_menu_ui(int fd) {
    int id;
    printf("Enter Item ID: ");
    scanf("%d", &id);
    while(getchar()!='\n');

    char cmd[128], buffer[1024];

    snprintf(cmd, sizeof(cmd), "DELETE_MENU %d\n", id);
    send(fd, cmd, strlen(cmd), 0);

    if (recv_response(fd, buffer) < 0) return;

    printf("%s%s%s\n", GREEN, buffer, RESET);
    wait_enter();
}

static void delete_order_ui(int fd) {
    int id;
    printf("Enter Order ID: ");
    scanf("%d", &id);
    while(getchar()!='\n');

    char cmd[128], buffer[1024];

    snprintf(cmd, sizeof(cmd), "DELETE_ORDER %d\n", id);
    send(fd, cmd, strlen(cmd), 0);

    if (recv_response(fd, buffer) < 0) return;

    printf("%s%s%s\n", GREEN, buffer, RESET);
    wait_enter();
}





static void add_table_ui(int fd) {
    int id=0, capacity;


    printf("Enter Capacity: ");
    scanf("%d", &capacity);

    while(getchar()!='\n');

    char cmd[128], buffer[1024];

    snprintf(cmd, sizeof(cmd), "ADD_TABLE %d %d\n", id, capacity);

    send(fd, cmd, strlen(cmd), 0);

    int bytes = recv(fd, buffer, sizeof(buffer)-1, 0);
    if(bytes <= 0) return;

    buffer[bytes] = '\0';

    printf("%s%s%s\n", GREEN, buffer, RESET);
    printf("\nPress Enter...");
    getchar();
}













static void add_menu_ui(int fd) {
    int id=0;
    float price;
    char name[100];


    printf("Enter Item Name: ");
    scanf("%[^\n]", name);

    while(getchar()!='\n');

    printf("Enter Price: ");
    scanf("%f", &price);

    while(getchar()!='\n');

    char cmd[256], buffer[1024];

    snprintf(cmd, sizeof(cmd),
             "ADD_MENU %d %s %.2f\n", id, name, price);

    send(fd, cmd, strlen(cmd), 0);

    int bytes = recv(fd, buffer, sizeof(buffer)-1, 0);
    if(bytes <= 0) return;

    buffer[bytes] = '\0';

    printf("%s%s%s\n", GREEN, buffer, RESET);
    printf("\nPress Enter...");
    getchar();
}



void print_admin_login_page(int fd) {
    ClearScreen();

    printf("%s***************************************************\n", BLUE);
    printf("*                                                 *\n");
    printf("*            %sAdmin Login Page%s                   *\n", YELLOW, BLUE);
    printf("*                                                 *\n");
    printf("*     %sPlease enter credentials to continue:%s     *\n", YELLOW, BLUE);
    printf("*                                                 *\n");
    printf("***************************************************\n");

    printf("%s Your Username : %s %s", BLINKING_YELLOW, RESET, RED);
    fflush(stdout);

    char username[100];
    scanf("%99s", username);

    int c;
    while ((c = getchar()) != '\n');

    printf("%s Your Password : %s %s", BLINKING_YELLOW, RESET, RED);
    fflush(stdout);

    char password[100];
    scanf("%99s", password);
    while ((c = getchar()) != '\n');

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "AUTH ADMIN %s %s\n", username, password);

    printf("Authenticating...\n");

    if (send(fd, cmd, strlen(cmd), 0) < 0) {
        perror("send failed");
        return;
    }

    char buffer[1024];
    int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes < 0) {
        perror("recv failed");
        exit(0);
    }

    buffer[bytes] = '\0';

    printf("Server Response: %s\n", buffer);

    if (strncmp(buffer, "OK", 2) == 0) {
        print_admin_dashboard(fd);   // 🔥 jump to admin UI
    } else {
        printf("%s Authentication Failed %s\n", RED, RESET);
        printf("%sPress Enter to Retry...%s", BLINKING_YELLOW, RESET);
        getchar();
        print_admin_login_page(fd);
    }
}




// ---------- MAIN DASHBOARD ----------

void print_admin_dashboard(int fd) {

    while (1) {

        ClearScreen();

        printf(BLUE"***************************************************************************\n");
        printf("*                                                                         *\n");
        printf("*              %sADMIN DASHBOARD%s                                           *\n", YELLOW, BLUE);
        printf("*                                                                         *\n");

        printf("*  %s  ____________________   ____________________   ____________________   %s*\n",YELLOW,BLUE);
        printf("*  %s |                    | |                    | |                    |  %s*\n",YELLOW,BLUE);
        printf("*  %s |   1. VIEW TABLES   | |   2. VIEW MENU     | |   3. VIEW ORDERS   |  %s*\n",YELLOW,BLUE);
        printf("*  %s |____________________| |____________________| |____________________|  %s*\n",YELLOW,BLUE);

        printf("*                                                                         *\n");

        printf("*  %s  ____________________   ____________________   ____________________   %s*\n",YELLOW,BLUE);
        printf("*  %s |                    | |                    | |                    |  %s*\n",YELLOW,BLUE);
        printf("*  %s |  4. DELETE TABLE   | |  5. DELETE MENU    | |  6. DELETE ORDER   |  %s*\n",YELLOW,BLUE);
        printf("*  %s |____________________| |____________________| |____________________|  %s*\n",YELLOW,BLUE);
        printf("*  %s  ____________________   ____________________                          %s*\n",YELLOW,BLUE);
        printf("*  %s |                    | |                    |                         %s*\n",YELLOW,BLUE);
        printf("*  %s |   7. ADD TABLE     | |  8. ADD MENU ITEM  |                         %s*\n",YELLOW,BLUE);
        printf("*  %s |____________________| |____________________|                         %s*\n",YELLOW,BLUE);

        printf("*                                                                         *\n");
        printf("*              %sPress 9 to Exit%s                                            *\n",BLINKING_YELLOW,BLUE);
        printf("*                                                                         *\n");
        printf("***************************************************************************\n");
        printf("\n\033[0m");

        printf("%s Your Choice : %s %s", BLINKING_YELLOW, RESET, RED);

        int choice;
        scanf("%d", &choice);

        int c;
        while((c=getchar())!='\n');

        if (choice == 1) view_tables(fd);
        else if (choice == 2) view_menu(fd);
        else if (choice == 3) view_orders(fd);
        else if (choice == 4) delete_table_ui(fd);
        else if (choice == 5) delete_menu_ui(fd);
        else if (choice == 6) delete_order_ui(fd);
        else if(choice == 7) add_table_ui(fd);
        else if(choice == 8) add_menu_ui(fd);   
        else if (choice == 9) return;
    }
}