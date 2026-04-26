#include <stdio.h>
#include <string.h>

#include "../../DB_Service/DB_Client/DB_Client.h"
#include "../../DB_Service/structs.h"
#include "../Server.h"
#include "Auth.h"

extern struct db_client DbClient;


int authenticate_admin(const char* username, const char* password) {

    if(username == NULL || password == NULL) {
        return AUTH_FAILURE;
    }

    if(strcmp(username, ROOT_USERNAME) == 0 && strcmp(password, ROOT_PASSWORD) == 0) {
        return AUTH_SUCCESS;
    }

    if(strcmp(username, "") == 0 || strcmp(password, "") == 0) {
        return AUTH_FAILURE;
    }

    struct users user_record;

    struct users user_to_search = {
        .username = "",
        .password = "",
        .role = ROLE_ADMIN
    };
    strcpy(user_to_search.username, username);
    strcpy(user_to_search.password, password);

    int stat = db_client_find_user(&DbClient, &user_to_search,
        USERS_MATCH_PASSWORD | USERS_MATCH_USERNAME, &user_record);

    if (stat == SUCCESS) {
        if (user_record.role == ROLE_ADMIN) {
            return AUTH_SUCCESS;
        }
        else {
            return AUTH_FAILURE;
        }
    }
    return AUTH_FAILURE;
}


int add_admin_user(const char* username, const char* password) {
    if(username == NULL || password == NULL) {
        printf("AUTH ERROR: Username and password cannot be NULL.\n");
        return AUTH_FAILURE;
    }

    if(strcmp(username, "") == 0 || strcmp(password, "") == 0) {
        printf("AUTH ERROR: Username and password cannot be empty.\n");
        return AUTH_FAILURE;
    }

    if(strcmp(username, ROOT_USERNAME) == 0) {
        printf("AUTH ERROR: Cannot use root credentials for new admin user.\n");
        return AUTH_FAILURE;
    }

    // first check if user already exists
    struct users existing_user;
    struct users user_to_search = {
        .username = "",
        .password = "",
        .role = ROLE_ADMIN
    };
    strcpy(user_to_search.username, username);
    strcpy(user_to_search.password, password);

    int stat = db_client_find_user(&DbClient, &user_to_search,
        USERS_MATCH_USERNAME, &existing_user);

    if (stat == SUCCESS) {
        printf("AUTH ERROR: User '%s' already exists.\n", username);
        return AUTH_FAILURE;
    }

    struct users new_user = {
        .role = ROLE_ADMIN
    };
    strcpy(new_user.username, username);
    strcpy(new_user.password, password);

    stat = db_client_insert_user(&DbClient, &new_user, NULL);

    if (stat == SUCCESS) {
        return AUTH_SUCCESS;
    }
    return AUTH_FAILURE;
}



int authenticate_chef(const char* username, const char* password) {
    if(username == NULL || password == NULL) {
        return AUTH_FAILURE;
    }

    if(strcmp(username, "") == 0 || strcmp(password, "") == 0) {
        return AUTH_FAILURE;
    }

    struct users user_record;

    struct users user_to_search = {
        .username = "",
        .password = "",
        .role = ROLE_CHEF
    };
    strcpy(user_to_search.username, username);
    strcpy(user_to_search.password, password);

    int stat = db_client_find_user(&DbClient, &user_to_search,
        USERS_MATCH_PASSWORD | USERS_MATCH_USERNAME, &user_record);

    if (stat == SUCCESS) {
        if (user_record.role == ROLE_CHEF) {
            return AUTH_SUCCESS;
        }
        else {
            return AUTH_FAILURE;
        }
    }
    return AUTH_FAILURE;
}



int add_chef_user(const char* username, const char* password) {
    if(username == NULL || password == NULL) {
        printf("AUTH ERROR: Username and password cannot be NULL.\n");
        return AUTH_FAILURE;
    }

    if(strcmp(username, "") == 0 || strcmp(password, "") == 0) {
        printf("AUTH ERROR: Username and password cannot be empty.\n");
        return AUTH_FAILURE;
    }

    // first check if user already exists
    struct users existing_user;
    struct users user_to_search = {
        .username = "",
        .password = "",
        .role = ROLE_CHEF
    };
    strcpy(user_to_search.username, username);
    strcpy(user_to_search.password, password);

    int stat = db_client_find_user(&DbClient, &user_to_search,
        USERS_MATCH_USERNAME, &existing_user);

    if (stat == SUCCESS) {
        printf("AUTH ERROR: User '%s' already exists.\n", username);
        return AUTH_FAILURE;
    }

    struct users new_user = {
        .role = ROLE_CHEF
    };
    strcpy(new_user.username, username);
    strcpy(new_user.password, password);

    stat = db_client_insert_user(&DbClient, &new_user, NULL);

    if (stat == SUCCESS) {
        return AUTH_SUCCESS;
    }
    return AUTH_FAILURE;
}



int authenticate_waiter(const char* username, const char* password) {
    if(username == NULL || password == NULL) {
        return AUTH_FAILURE;
    }

    if(strcmp(username, "") == 0 || strcmp(password, "") == 0) {
        return AUTH_FAILURE;
    }

    struct users user_record;

    struct users user_to_search = {
        .username = "",
        .password = "",
        .role = ROLE_WAITER
    };
    strcpy(user_to_search.username, username);
    strcpy(user_to_search.password, password);

    int stat = db_client_find_user(&DbClient, &user_to_search,
        USERS_MATCH_USERNAME | USERS_MATCH_PASSWORD, &user_record);

    if (stat == SUCCESS) {
        if (user_record.role == ROLE_WAITER) {
            return AUTH_SUCCESS;
        }
        else {
            return AUTH_FAILURE;
        }
    }
    return AUTH_FAILURE;
}


int add_waiter_user(const char* username, const char* password) {
    if(username == NULL || password == NULL) {
        printf("AUTH ERROR: Username and password cannot be NULL.\n");
        return AUTH_FAILURE;
    }

    if(strcmp(username, "") == 0 || strcmp(password, "") == 0) {
        printf("AUTH ERROR: Username and password cannot be empty.\n");
        return AUTH_FAILURE;
    }

    // first check if user already exists
    struct users existing_user;
    struct users user_to_search = {
        .username = "",
        .password = "",
        .role = ROLE_WAITER
    };
    strcpy(user_to_search.username, username);
    strcpy(user_to_search.password, password);

    int stat = db_client_find_user(&DbClient, &user_to_search,
        USERS_MATCH_USERNAME, &existing_user);

    if (stat == SUCCESS) {
        printf("AUTH ERROR: User '%s' already exists.\n", username);
        return AUTH_FAILURE;
    }

    struct users new_user = {
        .role = ROLE_WAITER
    };
    strcpy(new_user.username, username);
    strcpy(new_user.password, password);

    stat = db_client_insert_user(&DbClient, &new_user, NULL);

    if (stat == SUCCESS) {
        return AUTH_SUCCESS;
    }
    return AUTH_FAILURE;
}

int remove_admin_user(const char* username) {
    if(username == NULL) {
        printf("AUTH ERROR: Username cannot be NULL.\n");
        return AUTH_FAILURE;
    }

    if(strcmp(username, "") == 0) {
        printf("AUTH ERROR: Username cannot be empty.\n");
        return AUTH_FAILURE;
    }

    if(strcmp(username, ROOT_USERNAME) == 0) {
        printf("AUTH ERROR: Cannot remove root admin user.\n");
        return AUTH_FAILURE;
    }

    struct users user_to_search = {
        .username = "",
        .password = "",
        .role = ROLE_ADMIN
    };
    strcpy(user_to_search.username, username);

    struct users existing_user;

    int stat = db_client_find_user(&DbClient, &user_to_search,
        USERS_MATCH_USERNAME, &existing_user);

    if (stat != SUCCESS) {
        printf("AUTH ERROR: User '%s' does not exist.\n", username);
        return AUTH_FAILURE;
    }

    stat = db_client_delete_user(&DbClient, existing_user.userID);

    if (stat == SUCCESS) {
        return AUTH_SUCCESS;
    }
    return AUTH_FAILURE;
}


int remove_chef_user(const char* username) {
    if(username == NULL) {
        printf("AUTH ERROR: Username cannot be NULL.\n");
        return AUTH_FAILURE;
    }

    if(strcmp(username, "") == 0) {
        printf("AUTH ERROR: Username cannot be empty.\n");
        return AUTH_FAILURE;
    }

    struct users user_to_search = {
        .username = "",
        .password = "",
        .role = ROLE_CHEF
    };
    strcpy(user_to_search.username, username);

    struct users existing_user;

    int stat = db_client_find_user(&DbClient, &user_to_search,
        USERS_MATCH_USERNAME, &existing_user);

    if (stat != SUCCESS) {
        printf("AUTH ERROR: User '%s' does not exist.\n", username);
        return AUTH_FAILURE;
    }

    stat = db_client_delete_user(&DbClient, existing_user.userID);

    if (stat == SUCCESS) {
        return AUTH_SUCCESS;
    }
    return AUTH_FAILURE;
}




int remove_waiter_user(const char* username) {
    if(username == NULL) {
        printf("AUTH ERROR: Username cannot be NULL.\n");
        return AUTH_FAILURE;
    }

    if(strcmp(username, "") == 0) {
        printf("AUTH ERROR: Username cannot be empty.\n");
        return AUTH_FAILURE;
    }

    struct users user_to_search = {
        .username = "",
        .password = "",
        .role = ROLE_WAITER
    };
    strcpy(user_to_search.username, username);

    struct users existing_user;

    int stat = db_client_find_user(&DbClient, &user_to_search,
        USERS_MATCH_USERNAME, &existing_user);

    if (stat != SUCCESS) {
        printf("AUTH ERROR: User '%s' does not exist.\n", username);
        return AUTH_FAILURE;
    }

    stat = db_client_delete_user(&DbClient, existing_user.userID);

    if (stat == SUCCESS) {
        return AUTH_SUCCESS;
    }
    return AUTH_FAILURE;
}