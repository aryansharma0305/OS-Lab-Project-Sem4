#ifndef UI_GENERAL_H
#define UI_GENERAL_H


#define BLUE "\033[96m"
#define RED "\033[91m"
#define YELLOW "\033[33m"
#define BLINKING_YELLOW "\033[93m"
#define GREEN "\033[92m"
#define RESET "\033[0m"


#include <stdio.h>

#include <sys/socket.h>

#include "ui_guest.h"
#include "ui_chef.h"


#include <stdlib.h>

void ClearScreen();
void print_landing_page(int fd);


#endif
