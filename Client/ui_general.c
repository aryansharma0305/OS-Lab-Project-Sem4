#include "ui_general.h"
#include "ui_guest.h"
#include "ui_chef.h"
#include "ui_admin.h"

#include <stdio.h>


void ClearScreen() {
    printf("\033[2J\033[H");
}



void print_landing_page(int fd){

    ClearScreen();

    printf(BLUE"***************************************************************************\n");
    printf("*                                                                         *\n");
    printf("*%s      ____  _____ ____ _____  _    _   _  ____      _    _   _ _____     %s*\n", RED, BLUE);
    printf("*%s     |  _ \\| ____/ ___|_   _|/ \\  | | | ||  _ \\    / \\  | \\ | |_   _|    %s*\n", RED, BLUE);
    printf("*%s     | |_) |  _| \\___ \\ | | / _ \\ | | | || |_) |  / _ \\ |  \\| | | |      %s*\n", RED, BLUE);
    printf("*%s     |  _ <| |___ ___) || |/ ___ \\| |_| ||  _ <  / ___ \\| |\\  | | |      %s*\n", RED, BLUE);
    printf("*%s     |_| \\_\\_____|____/ |_/_/   \\_\\|___/ |_| \\_\\/_/   \\_\\_| \\_| |_|      %s*\n", RED, BLUE);

    printf("*             %s  ________________________________                          %s*\n",YELLOW,BLUE);
    printf("*             %s |                                |                         %s*\n",YELLOW,BLUE);
    printf("*             %s |      1. LOGIN AS GUEST         |                         %s*\n",YELLOW,BLUE);
    printf("*             %s |________________________________|                         %s*\n",YELLOW,BLUE);

    printf("*                                                                         *\n");

    printf("*            %s  _____________        _____________                         %s*\n",YELLOW,BLUE);
    printf("*            %s |             |      |             |                        %s*\n",YELLOW,BLUE);
    printf("*            %s |  2. ADMIN   |      |  3. CHEF    |                        %s*\n",YELLOW,BLUE);
    printf("*            %s |_____________|      |_____________|                        %s*\n",YELLOW,BLUE);

    printf("*                                                                         *\n");

    printf("*                                                                         *\n");
    printf("*            %sPress 9 to Exit%s                                              *\n",BLINKING_YELLOW,BLUE);

    printf("*                                                                         *\n");
    printf("***************************************************************************\n");
    printf("\n\033[0m");

    printf("%s Your Choice : %s %s",BLINKING_YELLOW,RESET,RED);
    fflush(stdout);

    int choice;
    scanf("%d",&choice);

    int c;
    while ((c = getchar()) != '\n');

    if(choice == 9){
        printf("%s Exiting... %s\n",BLINKING_YELLOW,RESET);
        exit(0);
    }
    else if(choice ==1){
        if (send(fd, "AUTH GUEST", strlen("AUTH GUEST "), 0) < 0) {
            perror("send failed");
        }
        //now wait for resp
        char buffer[1024];
        int bytes_received = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0) {
            perror("recv failed");
        } else {
            buffer[bytes_received] = '\0';
            if (strncmp(buffer, "OK", 2) == 0) {
                print_customer_details_page(fd,0);
            } else {
                printf("%s Authentication Failed %s\n",RED,RESET);
            }

        }
    
    }

    else if(choice ==2){
        print_admin_login_page(fd);
    }
    else if (choice ==3){
        print_chef_login_page(fd);
    }
    else{
        printf("%s Invalid Choice %s\n",RED,RESET);
        print_landing_page(fd);
    }
}
