#include <uuid/uuid.h>
#include <stdio.h>


int main () {
    uuid_t uuid;
    char uuid_str[37]; // UUIDs are 36 characters plus a null terminator

    // Generate a random UUID
    uuid_generate_random(uuid);

    // Convert the UUID to a string
    uuid_unparse(uuid, uuid_str);

    // Print the generated UUID
    printf("Generated UUID: %s\n", uuid_str);

    return 0;
}


// gcc file.c -o uuid_demo -luuid
// ./uuid_demo