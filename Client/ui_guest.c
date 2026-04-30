#include <stdio.h>
#include "ui_guest.h"
#include "ui_general.h"



void print_customer_details_page(int fd, int flag) {

    ClearScreen();
    char name[100], email[100], phone[20];

    printf("%s****************************************************\n",BLUE);
    printf("*                                                  *\n");
    printf("*            %sCustomer Details Page%s                 *\n",YELLOW,BLUE);
    printf("*                                                  *\n");
    printf("*     %sPlease provide the following details:%s        *\n",YELLOW,BLUE);

    if(flag==1){
        printf("*            %sEnter Correct Details%s                 *\n",RED,BLUE);
    }

    printf("*                                                  *\n");
    printf("****************************************************\n");

    // NAME
    printf("*                                                  *\n");
    printf("*                                                  *\n");
    printf("*                                                  *\n");
    printf("****************************************************\033[F\033[F*  %s Name: %s ",YELLOW,RED);
    scanf("%[^\n]", name);

    // EMAIL
    printf("\n\n%s*                                                  *\n",BLUE);
    printf("*                                                  *\n");
    printf("*                                                  *\n");
    printf("****************************************************\033[F\033[F*  %s Email: %s",YELLOW,RED);
    scanf("%s", email);

    // PHONE
    printf("\n\n%s*                                                  *\n",BLUE);
    printf("*                                                  *\n");
    printf("*                                                  *\n");
    printf("****************************************************\033[F\033[F*  %s Phone Number: %s",YELLOW,RED);
    scanf("%s", phone);

    printf("\n\n\n%s Enter 1 to confirm Details , 0 to Re-enter : %s %s",
           BLINKING_YELLOW, RESET, RED);

    int choice;
    scanf("%d",&choice);

    int c;
    while ((c = getchar()) != '\n');

    if(choice == 1){

        int name_flag = 0;
        for (int i = 0; name[i] != '\0'; i++) {
            if (name[i] >= '0' && name[i] <= '9') name_flag = 1;
        }

        int phone_flag = 0, len = 0;
        for (int i = 0; phone[i] != '\0'; i++) {
            len++;
            if (phone[i] < '0' || phone[i] > '9') phone_flag = 1;
        }
        if(len != 10) phone_flag = 1;

        int email_flag = 0;
        int at = 0, dot = 0;
        for(int i = 0; email[i] != '\0'; i++){
            if(email[i] == '@') at = 1;
            if(email[i] == '.') dot = 1;
        }
        if(!(at && dot)) email_flag = 1;

        if(phone_flag == 0 && name_flag == 0 && email_flag == 0){


            name[strcspn(name, "\n")] = '\0';
            int len_name = strlen(name);
            while(len_name > 0 && name[len_name - 1] == ' ') {
                name[--len_name] = '\0';
            }
            for(int i = 0; name[i]; i++){
                if(name[i] == ' ') name[i] = '-';
            }
            email[strcspn(email, "\n")] = '\0';
            char clean_phone[11];
            int j = 0;

            for(int i = 0; phone[i] && j < 10; i++){
                if(phone[i] >= '0' && phone[i] <= '9'){
                    clean_phone[j++] = phone[i];
                }
            }
            clean_phone[j] = '\0';

            strcpy(phone, clean_phone);
            show_tables_menu(fd, name, phone, email);
        }
        else{
            print_customer_details_page(fd,1);
        }
    }
    else{
        print_customer_details_page(fd,0);
    }
}




void show_tables_menu(int fd, char* name, char* phone, char* email){

    char buffer[4096];

    // --- REQUEST TABLES ---
    send(fd, "GET_TABLES", strlen("GET_TABLES "), 0);

    int bytes = recv(fd, buffer, sizeof(buffer)-1, 0);
    if(bytes <= 0){
        perror("recv failed");
        return;
    }
    buffer[bytes] = '\0';

    if(strncmp(buffer, "ERROR", 5) == 0){
        printf("%s[ERROR] %s%s\n", RED, buffer + 6, RESET);
        printf("%sPress Enter to retry...%s", BLINKING_YELLOW, RESET);
        getchar();
        show_tables_menu(fd, name, phone, email);
        return;
    }




    ClearScreen();

    printf("%s================ AVAILABLE TABLES ================%s\n\n", YELLOW, RESET);

    int tableID, capacity, isOccupied;

    printf("%sTableID   Capacity   Status%s\n", BLUE, RESET);
    printf("-----------------------------------\n");

    char *ptr = strchr(buffer, '{');

    while(ptr){
        if(sscanf(ptr,
            "{\"tableID\":%d,\"capacity\":%d,\"isOccupied\":%d}",
            &tableID, &capacity, &isOccupied) == 3){

            printf("%s%-8d %-10d %s%s\n",
                BLUE,
                tableID,
                capacity,
                isOccupied ? RED "Occupied" : GREEN "Available",
                RESET);
        }

        ptr = strchr(ptr + 1, '{');
    }
    // --- USER INPUT ---
    printf("\n%sEnter Table ID to book (or -1 to go back): %s", BLINKING_YELLOW, RESET);

    int table_id;
    scanf("%d", &table_id);

    int c;
    while((c = getchar()) != '\n');

    if(table_id == -1){
        print_landing_page(fd);
        return;
    }

    char cmd[256];
    snprintf(cmd, sizeof(cmd),
             "BOOK_TABLE %d %s %s %s\n",
             table_id, name, phone, email);

    send(fd, cmd, strlen(cmd), 0);

    bytes = recv(fd, buffer, sizeof(buffer)-1, 0);
    if(bytes <= 0){
        perror("recv failed");
        return;
    }
    buffer[bytes] = '\0';

    if(strncmp(buffer, "ERROR", 5) == 0){
        printf("%s[ERROR] %s%s\n", RED, buffer + 6, RESET);
        printf("%sPress Enter to retry...%s", BLINKING_YELLOW, RESET);
        getchar();
        show_tables_menu(fd, name, phone, email);
        return;
    }

    int order_id;
    sscanf(buffer, "OK %d", &order_id);

    printf("%s\nTable booked successfully!%s\n", GREEN, RESET);
    printf("%sOrder ID: %d%s\n", GREEN, order_id, RESET);

    printf("%sPress Enter to continue...%s", BLINKING_YELLOW, RESET);
    getchar();

    print_customer_dashboard(fd, order_id);
}






void print_customer_dashboard(int fd, int orderID){

    char buffer[4096];

    // --- SEND REQUEST ---
    send(fd, "GET_ORDER\n", strlen("GET_ORDER\n"), 0);

    int bytes = recv(fd, buffer, sizeof(buffer)-1, 0);
    if(bytes <= 0){
        perror("recv failed");
        return;
    }
    buffer[bytes] = '\0';

    // --- ERROR HANDLING ---
    if(strncmp(buffer, "ERROR", 5) == 0){
        printf("%s[ERROR] %s%s\n", RED, buffer + 6, RESET);
        printf("%sPress Enter to retry...%s", BLINKING_YELLOW, RESET);
        getchar();
        print_customer_dashboard(fd, orderID);
        return;
    }

    // --- PARSE JSON ---
    char name[100], phone[20];
    int tableID;
    float totalBill;

    if(sscanf(buffer,
        "OK {\"orderID\":%d,\"tableID\":%d,\"name\":\"%[^\"]\",\"phone\":\"%[^\"]\",\"email\":\"%*[^\"]\",\"orderDate\":\"%*[^\"]\",\"orderTime\":\"%*[^\"]\",\"totalBill\":%f",
        &orderID, &tableID, name, phone, &totalBill) != 5){
        
        printf("Parse error\n");
    }

    // --- UI ---
    ClearScreen();

    printf(BLUE"***************************************************************************\n");
    printf("*                                                                         *\n");
    printf("*              %sCUSTOMER DASHBOARD%s                                         *\n", YELLOW, BLUE);
    printf("*                                                                         *\n");
    printf("*   %sTable : %d%s                                                               *\n", YELLOW, tableID, BLUE);
    printf("*   %sOrder ID : %d%s                                                            *\n", YELLOW, orderID, BLUE);
    printf("*   %sName : %s%-20s%s                                              *\n", YELLOW, RESET, name, BLUE);
    printf("*   %sPhone: %s%-20s%s                                              *\n", YELLOW, RESET, phone, BLUE);
    printf("*   %sBill : %s₹ %.2f%s                                                         *\n", YELLOW, RESET, totalBill, BLUE);

    printf("*                                                                         *\n");

   
    printf("*  %s  ____________________   ____________________   ____________________   %s*\n",YELLOW,BLUE);
    printf("*  %s |                    | |                    | |                    |  %s*\n",YELLOW,BLUE);
    printf("*  %s |   1. ORDER ITEMS   | |   2. GENERATE BILL | |  3. VIEW ORDER     |  %s*\n",YELLOW,BLUE);
    printf("*  %s |                    | |                    | |     DETAILS        |  %s*\n",YELLOW,BLUE);
    printf("*  %s |____________________| |____________________| |____________________|  %s*\n",YELLOW,BLUE);
    printf("*                                                                         *\n");
    printf("*              %sPress 9 to Exit%s                                            *\n",BLINKING_YELLOW,BLUE);
    printf("*                                                                         *\n");
    printf("***************************************************************************\n");
    printf("\n\033[0m");

    printf("%s Your Choice : %s %s", BLINKING_YELLOW, RESET, RED);

    int choice;
    scanf("%d", &choice);

    int c;
    while((c = getchar()) != '\n');

    // --- HANDLE CHOICE ---
    if(choice == 1){

        print_order_items_page(fd, orderID);

    }
    else if(choice == 2){

        print_generate_bill_page(fd, orderID);
      
    }
    else if(choice == 3){
        print_view_order_details_page(fd, orderID);
    }
    else if(choice == 9){
        
    }
    else{
        print_customer_dashboard(fd, orderID);
    }
}















void print_order_items_page(int fd, int orderID){

    char buffer[8192];

    while(1){

        // --- GET MENU ---
        send(fd, "GET_MENU\n", strlen("GET_MENU\n"), 0);

        int bytes = recv(fd, buffer, sizeof(buffer)-1, 0);
        if(bytes <= 0){
            perror("recv failed");
            return;
        }
        buffer[bytes] = '\0';

        // --- ERROR ---
        if(strncmp(buffer, "ERROR", 5) == 0){
            printf("%s[ERROR] %s%s\n", RED, buffer + 6, RESET);
            printf("%sPress Enter to retry...%s", BLINKING_YELLOW, RESET);
            getchar();
            continue;
        }

        // --- UI ---
        ClearScreen();
        printf("%s==================== MENU ====================%s\n\n", YELLOW, RESET);

        printf("%sID     Name                      Price%s\n", BLUE, RESET);
        printf("------------------------------------------------\n");

        int itemID;
        char itemName[100];
        float price;

        // --- PARSE ---
        char *ptr = strchr(buffer, '{');

        while(ptr){
            if(sscanf(ptr,
                "{\"itemID\":%d,\"itemName\":\"%[^\"]\",\"price\":%f}",
                &itemID, itemName, &price) == 3){

                printf("%s%-6d %-25s ₹ %.2f%s\n",
                       BLUE, itemID, itemName, price, RESET);
            }

            ptr = strchr(ptr + 1, '{');
        }

        // --- INPUT ---
        printf("\n%sEnter Item ID (-1 to go back): %s", BLINKING_YELLOW, RESET);

        int item_id;
        scanf("%d", &item_id);

        int c;
        while((c = getchar()) != '\n');

        if(item_id == -1){
            print_customer_dashboard(fd, orderID);
            return;
        }

        printf("%sEnter Quantity: %s", BLINKING_YELLOW, RESET);

        int quantity;
        scanf("%d", &quantity);

        while((c = getchar()) != '\n');

        if(quantity <= 0){
            printf("%sInvalid quantity%s\n", RED, RESET);
            getchar();
            continue;
        }

        // --- SEND ORDER ---
        char cmd[256];
        snprintf(cmd, sizeof(cmd),
                 "ORDER_ITEM %d %d\n", item_id, quantity);

        send(fd, cmd, strlen(cmd), 0);

        bytes = recv(fd, buffer, sizeof(buffer)-1, 0);
        if(bytes <= 0){
            perror("recv failed");
            return;
        }
        buffer[bytes] = '\0';

        // --- HANDLE RESPONSE ---
        if(strncmp(buffer, "ERROR", 5) == 0){
            printf("%s[ERROR] %s%s\n", RED, buffer + 6, RESET);
        } else {
            printf("%sItem added successfully!%s\n", GREEN, RESET);
        }

        printf("\n%s1. Continue Ordering    2. Go Back to Dashboard%s\n",
       BLINKING_YELLOW, RESET);

        printf("%sYour Choice: %s", BLINKING_YELLOW, RESET);

        int choice;
        scanf("%d", &choice);

        
        while((c = getchar()) != '\n');

        if(choice == 1){
            continue;   // loop again → same function
        }
        else if(choice == 2){
            print_customer_dashboard(fd, orderID);
            return;
        }
        else{
            printf("%sInvalid choice%s\n", RED, RESET);
            getchar();
            continue;
        }
    }
}






void print_view_order_details_page(int fd, int orderID){

    char buffer[8192];

    send(fd, "GET_ORDER\n", strlen("GET_ORDER\n"), 0);

    int bytes = recv(fd, buffer, sizeof(buffer)-1, 0);
    if(bytes <= 0){
        perror("recv failed");
        return;
    }
    buffer[bytes] = '\0';

    if(strncmp(buffer, "ERROR", 5) == 0){
        printf("%s[ERROR] %s%s\n", RED, buffer+6, RESET);
        getchar();
        print_customer_dashboard(fd, orderID);
        return;
    }

    int tableID;
    char name[100], phone[20], date[20], time[20];
    float totalBill;

    sscanf(buffer,
        "OK {\"orderID\":%*d,\"tableID\":%d,\"name\":\"%[^\"]\",\"phone\":\"%[^\"]\",\"email\":\"%*[^\"]\",\"orderDate\":\"%[^\"]\",\"orderTime\":\"%[^\"]\",\"totalBill\":%f",
        &tableID, name, phone, date, time, &totalBill
    );

    ClearScreen();

    printf("%s================ ORDER DETAILS ================%s\n\n", YELLOW, RESET);

    printf("%sName : %s%s\n", BLUE, RESET, name);
    printf("%sPhone: %s%s\n", BLUE, RESET, phone);
    printf("%sTable: %d\n", BLUE, tableID);
    printf("%sDate : %s  Time: %s\n", BLUE, date, time);
    printf("%sTotal: ₹ %.2f%s\n\n", BLUE, totalBill, RESET);

    printf("%sID   Item                      Qty   Price   Status%s\n", YELLOW, RESET);
    printf("------------------------------------------------------------\n");

    int orderItemID, itemID, quantity, isPrepared;
    char itemName[100];
    float price;

    char *ptr = strstr(buffer, "\"items\":[");
    if(ptr) ptr = strchr(ptr, '{');

    while(ptr){
        if(sscanf(ptr,
            "{\"orderItemID\":%d,\"itemID\":%d,\"itemName\":\"%[^\"]\",\"price\":%f,\"quantity\":%d,\"isPrepared\":%d}",
            &orderItemID, &itemID, itemName, &price, &quantity, &isPrepared) == 6){

            printf("%s%-4d %-25s %-5d %-7.2f %s%s\n",
                   BLUE,
                   itemID,
                   itemName,
                   quantity,
                   price,
                   isPrepared ? GREEN"Prepared" : RED"Pending",
                   RESET);
        }

        ptr = strchr(ptr+1, '{');
    }

    printf("\n%sPress Enter to go back...%s", BLINKING_YELLOW, RESET);
    getchar();

    print_customer_dashboard(fd, orderID);
}











void print_generate_bill_page(int fd, int orderID){

    char buffer[8192];

    // --- STEP 1: GENERATE BILL ---
    send(fd, "GENERATE_BILL\n", strlen("GENERATE_BILL\n"), 0);

    int bytes = recv(fd, buffer, sizeof(buffer)-1, 0);
    if(bytes <= 0){
        perror("recv failed");
        return;
    }
    buffer[bytes] = '\0';

    if(strncmp(buffer, "ERROR", 5) == 0){
        printf("%s[ERROR] %s%s\n", RED, buffer+6, RESET);
        getchar();
        print_customer_dashboard(fd, orderID);
        return;
    }

    float finalAmount;
    sscanf(buffer, "OK %f", &finalAmount);

    // --- STEP 2: GET FULL ORDER ---
    send(fd, "GET_ORDER\n", strlen("GET_ORDER\n"), 0);

    bytes = recv(fd, buffer, sizeof(buffer)-1, 0);
    if(bytes <= 0){
        perror("recv failed");
        return;
    }
    buffer[bytes] = '\0';

    // --- BASIC INFO ---
    int tableID;
    char name[100], date[20], time[20];

    sscanf(buffer,
        "OK {\"orderID\":%*d,\"tableID\":%d,\"name\":\"%[^\"]\",\"phone\":\"%*[^\"]\",\"email\":\"%*[^\"]\",\"orderDate\":\"%[^\"]\",\"orderTime\":\"%[^\"]\",\"totalBill\":%*f",
        &tableID, name, date, time
    );

    // --- UI START ---
    ClearScreen();

    printf(BLUE"**********************************************************\n");
    printf("                                                          \n");
    printf("                %s FINAL BILL RECEIPT %s                   \n", YELLOW, BLUE);
    printf("                                                          \n");

    printf("   %sCustomer:%s %-30s%s \n", YELLOW, RESET, name, BLUE);
    printf("   %sTable   :%s %-30d%s \n", YELLOW, RESET, tableID, BLUE);
    printf("   %sDate    :%s %-12s %-12s%s \n", YELLOW, RESET, date, time, BLUE);

    printf("                                                          \n");
    printf("----------------------------------------------------------\n");

    printf("   %-20s %-6s %-8s %-10s *\n", "Item", "Qty", "Price", "Subtotal");
    printf("----------------------------------------------------------\n");

    // --- PARSE ITEMS ---
    int itemID, quantity, isPrepared;
    char itemName[100];
    float price;

    char *ptr = strstr(buffer, "\"items\":[");
    if(ptr) ptr = strchr(ptr, '{');

    while(ptr){
        if(sscanf(ptr,
            "{\"orderItemID\":%*d,\"itemID\":%d,\"itemName\":\"%[^\"]\",\"price\":%f,\"quantity\":%d,\"isPrepared\":%d}",
            &itemID, itemName, &price, &quantity, &isPrepared) == 5){

            float subtotal = price * quantity;

            printf("   %-20s %-6d %-8.2f %-10.2f \n",
                   itemName, quantity, price, subtotal);
        }

        ptr = strchr(ptr+1, '{');
    }

    printf("----------------------------------------------------------\n");

    printf("   %sTOTAL:%s ₹ %-20.2f%s                        \n",
           BLINKING_YELLOW, RESET, finalAmount, BLUE);

    printf("----------------------------------------------------------\n");
    printf("                                                          \n");
    printf("        %sThank you for dining with us!%s                  \n", GREEN, BLUE);
    printf("                                                          \n");
    printf("**********************************************************\n");

    printf("\n%sPress Enter to Exit...%s", BLINKING_YELLOW, RESET);
    getchar();

    exit(0);
}


































