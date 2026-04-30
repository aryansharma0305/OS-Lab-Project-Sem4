#include "main.h"




int fd;


void* handle_exit_signal() {
    printf("Got Ctrl+C, exiting...\n");
    printf("%sClosing connection to server...%s\n", YELLOW, RESET);
    close(fd);
    sleep(1);
    exit(0);
}

int main() {

   
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket failed");
        return 1;
    }


    // sigfnalk handler with args
    signal(SIGINT,handle_exit_signal); 

   
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        return 1;
    }

 
    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(fd);
        return 1;
    }

    printf("Connected to server\n");

   
    print_landing_page(fd); 

    close(fd);
    return 0;
}
