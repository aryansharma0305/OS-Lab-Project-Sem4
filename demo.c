// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include <sys/time.h>

// #include "DB_Service/DB_Client/DB_Client.h"
// #include "DB_Service/structs.h"

// #define NUM_THREADS 300

// struct db_client client;

// void* worker_thread(void* arg) {
//     int id = *((int*)arg);

//     // Do NOT set orderID — it is auto-assigned by the DB engine
//     struct orders order = {
//         .orderID = 100,
//         .tableID = id % 50,
//         .num_items = 1,
//         .totalBill = 10.0 + id
//     };

//     // INSERT — assigned_key receives the auto-assigned ID
//     int assigned_key = -1;
//     int status = db_client_insert_order(&client, &order, &assigned_key);
//     if (status != SUCCESS) {
//         printf("Insert failed for thread %d\n", id);
//         return NULL;
//     }
//     printf("Thread %d: Inserted order with assigned key %d\n", id, assigned_key);

//     // UPDATE — use assigned_key, not order.orderID
//     order.totalBill += 100;
//     status = db_client_update_order(&client, assigned_key, &order);
//     if (status != SUCCESS) {
//         printf("Update failed for thread %d (key=%d)\n", id, assigned_key);
//         return NULL;
//     }
//     printf("Thread %d: Updated order %d\n", id, assigned_key);

//     // DELETE — use assigned_key
//     status = db_client_delete_order(&client, assigned_key);
//     if (status != SUCCESS) {
//         printf("Delete failed for thread %d (key=%d)\n", id, assigned_key);
//         return NULL;
//     }
//     printf("Thread %d: Deleted order %d\n", id, assigned_key);

//     return NULL;
// }

// int main() {

//     if (db_client_init(&client) != SUCCESS) {
//         printf("Failed to connect to DB service\n");
//         return 1;
//     }

//     pthread_t threads[NUM_THREADS];
//     int ids[NUM_THREADS];

//     struct timeval start, end;
//     gettimeofday(&start, NULL);

//     for (int i = 0; i < NUM_THREADS; i++) {
//         ids[i] = i + 1;
//         if (pthread_create(&threads[i], NULL, worker_thread, &ids[i]) != 0) {
//             perror("Failed to create thread");
//             exit(EXIT_FAILURE);
//         }
//     }

//     for (int i = 0; i < NUM_THREADS; i++) {
//         pthread_join(threads[i], NULL);
//     }

//     gettimeofday(&end, NULL);

//     double time_taken =
//         (end.tv_sec  - start.tv_sec) +
//         (end.tv_usec - start.tv_usec) / 1e6;

//     printf("\n========== Benchmark Results ==========\n");
//     printf("Threads:    %d\n",   NUM_THREADS);
//     printf("Operations: %d (insert + update + delete per thread)\n", NUM_THREADS * 3);
//     printf("Time taken: %.6f seconds\n", time_taken);
//     printf("Throughput: %.2f ops/sec\n", (NUM_THREADS * 3) / time_taken);
//     printf("=======================================\n");

//     db_client_destroy(&client);
//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "DB_Service/DB_Client/DB_Client.h"
#include "DB_Service/structs.h"


struct db_client DbClient;


int main(){


    db_client_init(&DbClient);  // initialize the client and connect to DB service



    struct orders order1 = {
    .orderID        = 100,
    
    };

    struct orders order_out;

    db_client_read_order(&DbClient, 3, &order_out); // read order with key=1 into order_out

    printf("Found order: ID=%d, Name=%s, Phone=%s\n", order_out.orderID, order_out.name, order_out.phone);




    db_client_destroy(&DbClient); // cleanup and disconnect


}