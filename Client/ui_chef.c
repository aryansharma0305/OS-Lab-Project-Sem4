#include "ui_chef.h"
#include "ui_general.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_ITEMS 1000

struct ChefItem {
    int orderID;
    int orderItemID;
    int tableID;
    char itemName[100];
    int isPrepared;
};

static struct ChefItem items[MAX_ITEMS];
static int item_count = 0;
static pthread_mutex_t items_mutex = PTHREAD_MUTEX_INITIALIZER;

static void populate_dashboard(char *buffer);
static void apply_update(char *buffer);
static void render_dashboard(void);
static void *chef_listener(void *arg);

void print_chef_login_page(int fd) {
    ClearScreen();

    printf("%s***************************************************\n", BLUE);
    printf("*                                                 *\n");
    printf("*            %sChef Login Page%s                    *\n", YELLOW, BLUE);
    printf("*                                                 *\n");
    printf("*     %sPlease enter the password to continue:%s    *\n", YELLOW, BLUE);
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
    snprintf(cmd, sizeof(cmd), "AUTH CHEF %s %s\n", username, password);

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
        print_chef_dashboard(fd);
    } else {
        printf("%s Authentication Failed %s\n", RED, RESET);
        printf("%sPress Enter to Retry...%s", BLINKING_YELLOW, RESET);
        getchar();
        print_chef_login_page(fd);
    }
}

static void populate_dashboard(char *buffer) {
    pthread_mutex_lock(&items_mutex);

    item_count = 0;

    int orderID, orderItemID, tableID;
    char itemName[100];

    char *ptr = strchr(buffer, '{');
    while (ptr) {
        if (sscanf(ptr,
                   "{\"orderID\":%d,\"orderItemID\":%d,\"itemName\":\"%[^\"]\",\"tableID\":%d}",
                   &orderID, &orderItemID, itemName, &tableID) == 4) {
            items[item_count].orderID = orderID;
            items[item_count].orderItemID = orderItemID;
            items[item_count].tableID = tableID;
            strncpy(items[item_count].itemName, itemName, sizeof(items[item_count].itemName) - 1);
            items[item_count].itemName[sizeof(items[item_count].itemName) - 1] = '\0';
            items[item_count].isPrepared = 0;
            item_count++;
            if (item_count >= MAX_ITEMS) break;
        }

        ptr = strchr(ptr + 1, '{');
    }

    pthread_mutex_unlock(&items_mutex);
}















void apply_update(char *buffer){

    int orderID, orderItemID, tableID;
    char itemName[100], status[20];

    sscanf(buffer,
           "NEW_UPDATE %d %d %[^0-9\n] %d %s",
           &orderID,
           &orderItemID,
           itemName,
           &tableID,
           status);

    // trim spaces
    int len = strlen(itemName);
    while(len > 0 && itemName[len-1] == ' ') {
        itemName[len-1] = '\0';
        len--;
    }

    pthread_mutex_lock(&items_mutex);

    if(strcmp(status, "PENDING") == 0){

        // 🔥 ADD ITEM
        items[item_count].orderID = orderID;
        items[item_count].orderItemID = orderItemID;
        items[item_count].tableID = tableID;
        strcpy(items[item_count].itemName, itemName);
        items[item_count].isPrepared = 0;

        item_count++;
    }

    else if(strcmp(status, "PREPARED") == 0){

      
        for(int i = 0; i < item_count; i++){
            if(items[i].orderID == orderID &&
               items[i].orderItemID == orderItemID){

                for(int j = i; j < item_count - 1; j++){
                    items[j] = items[j + 1];
                }
                item_count--;
                break;
            }
        }
    }

    pthread_mutex_unlock(&items_mutex);
}











static void render_dashboard(void) {
    ClearScreen();

    printf("%s================ CHEF DASHBOARD ================%s\n\n", YELLOW, RESET);

    pthread_mutex_lock(&items_mutex);

    if (item_count == 0) {
        printf("%sNo pending items.%s\n", GREEN, RESET);
        pthread_mutex_unlock(&items_mutex);
        return;
    }

    int per_row = 3;

    for (int i = 0; i < item_count; i += per_row) {

        // ===== TOP BORDER =====
        for (int k = 0; k < per_row && i + k < item_count; k++)
            printf("+----------------------+   ");
        printf("\n");

        // ===== ORDER + ITEM =====
        for (int k = 0; k < per_row && i + k < item_count; k++) {
            struct ChefItem *x = &items[i + k];
            char line[32];
            snprintf(line, sizeof(line), "Order:%d  Item:%d", x->orderID, x->orderItemID);
            printf("| %s%-20.20s%s |   ", BLUE, line, RESET);
        }
        printf("\n");

        // ===== TABLE =====
        for (int k = 0; k < per_row && i + k < item_count; k++) {
            struct ChefItem *x = &items[i + k];
            char line[32];
            snprintf(line, sizeof(line), "Table: %d", x->tableID);
            printf("| %-20.20s |   ", line);   // ← No cursor tricks, just print directly
        }
        printf("\n");

        // ===== ITEM NAME =====
        for (int k = 0; k < per_row && i + k < item_count; k++) {
            struct ChefItem *x = &items[i + k];
            printf("| %-20.20s |   ", x->itemName);
        }
        printf("\n");

        // ===== STATUS =====
        for (int k = 0; k < per_row && i + k < item_count; k++)
            printf("| %s%-20.20s%s |   ", GREEN, "PENDING", RESET);
        printf("\n");

        // ===== BOTTOM BORDER =====
        for (int k = 0; k < per_row && i + k < item_count; k++)
            printf("+----------------------+   ");
        printf("\n\n");
    }

    printf("%sEnter orderID orderItemID to mark prepared, or r to refresh: %s",
           BLINKING_YELLOW, RESET);

    fflush(stdout);
    pthread_mutex_unlock(&items_mutex);
}











static void *chef_listener(void *arg) {
    int fd = *(int *)arg;
    char buffer[4096];

    while (1) {
        int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            printf("\nDisconnected\n");
            exit(0);
        }

        buffer[bytes] = '\0';

        if (strncmp(buffer, "OK", 2) == 0) {
            populate_dashboard(buffer);
            render_dashboard();
        } else if (strncmp(buffer, "NEW_UPDATE", 10) == 0) {
            printf("\n%sUpdate received: %s%s\n", YELLOW, buffer, RESET);
            apply_update(buffer);
            render_dashboard();
        } else if (strncmp(buffer, "ERROR", 5) == 0) {
            printf("\n%s%s%s\n", RED, buffer, RESET);
        }
    }

    return NULL;
}

void print_chef_dashboard(int fd) {
    send(fd, "GET_DASHBOARD\n", strlen("GET_DASHBOARD\n"), 0);

    pthread_t t;
    pthread_create(&t, NULL, chef_listener, &fd);
    pthread_detach(t);

    while (1) {
        printf("%sEnter orderID orderItemID to mark prepared, or r to refresh: %s",
               BLINKING_YELLOW, RESET);

        char input[128];
        if (!fgets(input, sizeof(input), stdin)) {
            continue;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "r") == 0) {
            send(fd, "GET_DASHBOARD\n", strlen("GET_DASHBOARD\n"), 0);
            continue;
        }

        int oid, iid;
        if (sscanf(input, "%d %d", &oid, &iid) == 2) {
            char cmd[128];
            snprintf(cmd, sizeof(cmd), "MARK_PREPARED %d %d\n", oid, iid);
            send(fd, cmd, strlen(cmd), 0);
        }
    }
}