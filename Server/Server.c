#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "../DB_Service/DB_Client/DB_Client.h"
#include "../DB_Service/structs.h"
#include "Server.h"


#include "Utils/Auth.h"





struct db_client DbClient;


int main () {

    
    int stat = db_client_init(&DbClient);
    if (stat != SUCCESS) {
        fprintf(stderr, "Failed to initialize DB client. Exiting.\n");
        return EXIT_FAILURE;
    }


    printf("1........\n");
        stat = authenticate_admin("admin", "admin123");
    if (stat == AUTH_SUCCESS) {
        printf("Default admin user authenticated successfully.\n");
    } else {
        printf("Failed to authenticate default admin user. Exiting.\n");
        return EXIT_FAILURE;
    }


    printf("2........\n");
    stat = add_admin_user("admin2", "password2");
    if (stat == AUTH_SUCCESS) {
        printf("Admin user 'admin2' added successfully.\n");
    } else {
        printf("Failed to add admin user 'admin2'. Exiting.\n");
        return EXIT_FAILURE;
    }



    printf("3........\n");
    stat = authenticate_admin("admin2", "password2");
    if (stat == AUTH_SUCCESS) {
        printf("Admin user 'admin2' authenticated successfully.\n");
    } else {
        printf("Failed to authenticate admin user 'admin2'. Exiting.\n");
        return EXIT_FAILURE;
    }



    printf("4........\n");
    stat = add_chef_user("chef1", "chefpass");
    if(stat==AUTH_FAILURE){
        printf("Failed to add chef user 'chef1'. Exiting.\n");
        return EXIT_FAILURE;
    }
    stat = add_waiter_user("waiter1", "waiterpass");
    if(stat==AUTH_FAILURE){
        printf("Failed to add waiter user 'waiter1'. Exiting.\n");
        return EXIT_FAILURE;
    }
    else


    stat = authenticate_chef("chef1", "chefpass");
    if(stat==AUTH_FAILURE){
        printf("Failed to authenticate chef user 'chef1'. Exiting.\n");
        return EXIT_FAILURE;
    }
    stat = authenticate_waiter("waiter1", "waiterpass");
    if(stat==AUTH_FAILURE){
        printf("Failed to authenticate waiter user 'waiter1'. Exiting.\n");
        return EXIT_FAILURE;
    }

}
    