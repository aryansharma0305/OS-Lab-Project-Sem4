#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main () {


    char buffer[1024] = "AUTH GUEST";

    char* res = strtok(buffer, " ");

    printf("Token: %s\n", res);
    printf("buffer after strtok: %s\n", buffer);



}