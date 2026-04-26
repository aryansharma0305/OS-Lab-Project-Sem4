#ifndef AUTH_H
#define AUTH_H

#define ROOT_USERNAME "root"
#define ROOT_PASSWORD "root"

#define AUTH_SUCCESS 0
#define AUTH_FAILURE 1

int authenticate_admin(const char* username, const char* password);
int add_admin_user(const char* username, const char* password);
int remove_admin_user(const char* username);

int authenticate_waiter(const char* username, const char* password);
int add_waiter_user(const char* username, const char* password);
int remove_waiter_user(const char* username);

int authenticate_chef(const char* username, const char* password);
int add_chef_user(const char* username, const char* password);
int remove_chef_user(const char* username);

#endif