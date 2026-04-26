#ifndef AUTH_H
#define AUTH_H

#define ROOT_USERNAME "admin"
#define ROOT_PASSWORD "admin123"

#define AUTH_SUCCESS 1
#define AUTH_FAILURE 0

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