/* Project #3: Cache or Not to Cache by Kongmanee, Jaturong */
#include <csim.h>
#include <stdio.h>


/* #define is used to define CONSTANT */
#define SIM_TIME 1000.0
#define NUM_CLIENTS 10l
#define NUM_SERVER 1l

#define DB_SIZE 10000
#define CACHE_SIZE 100

#define BROADCAST_INTERVAL 20
#define MEAN_UPDATE 20
#define MEAN_QUERY 10


struct nde {
    MBOX mbox;
};
struct nde node[NUM_CLIENTS + NUM_SERVER];


struct data_items {
    long id;
    TIME last_updated_time;

};
struct data_items database[DB_SIZE];
struct data_items *ir;

struct cache_items {
    long id;
    TIME last_updated_time;
    TIME last_accessed_time;
};
struct cache_items cache_size[NUM_CLIENTS][CACHE_SIZE];

struct request {
    long item_id;
};
struct request *q;


void init();
/* server side and its function */
void server();
void invalidation_report();
void update_data_items();
void receive_message();

/* client side and its function*/
void client();
void generate_query();
void receive_ir();


void sim() {
    create("sim");
    init();
    hold(SIM_TIME);
}

void init() {
    long i;
    char str[24];
    
    sprintf(str, "input.%d", 0);
    node[0].mbox = mailbox(str);

    for (i = 1; i <= NUM_CLIENTS; i++) {
        sprintf(str, "input.%d", i);
        node[i].mbox = mailbox(str);
    }

    server(0);
    printf("Calling server\n");

    for (i = 1; i <= NUM_CLIENTS; i++) {
        client(i);
        printf("Calling client%ld\n", i);
    }

    q = (struct request *)malloc(sizeof(struct request) * NUM_CLIENTS);
    for (i = 1; i <= NUM_CLIENTS; i++) {
        (q+i)->item_id = 2222;
        printf("Allocating request for Node %ld at address %ld with value %ld\n", i, (q+i), *(q+i));
    }
    printf("Q address %ld", &q);
    
}

void server(long n) {
    create("server");
    printf("Server is generated\n");

    /* Set up database */
    long i;
    for (i = 0; i < DB_SIZE; i++) {
        database[i].id = i;
        database[i].last_updated_time = clock;
    }
    printf("Calling IR\n");
    invalidation_report();
    /*update_data_items();*/
    receive_message();
    while(clock < SIM_TIME){
        hold(1);  
    }   
}

void invalidation_report() {
    create("ir"); 
    while(clock < SIM_TIME){
        hold(BROADCAST_INTERVAL);    
        /* Broadcast: put the message into all clients' mailboxes */
        /* Create a new IR */
        long i;
        /*if (ir != NULL) {
            printf("Deallocating memory previously allocated by an IR");
            for (i = 0; i < DB_SIZE; i++) {
                printf("---------------------------");
                free(ir[i].id);
                free(ir[i].last_updated_time);
            }
            free(ir);
        }*/
        
        ir = (struct data_item *)malloc(sizeof(struct data_items) * DB_SIZE);
        printf("Creating an IR...\n");
        for (i = 0; i < DB_SIZE; i++) {
            ir[i].id = i;
            ir[i].last_updated_time = clock;
        }

        for (i = 1; i <= NUM_CLIENTS; i++) {   
            send(node[i].mbox, (long)ir);
            printf("Broadcasting to node %ld \n", i);
        }
    }
    
}

void update_data_items() {
    create("update");
    printf("Updating data items\n");
    while(clock < SIM_TIME){
        hold(10);
        long i;
        for (i = 0; i < DB_SIZE; i++) {
            database[i].last_updated_time = clock;
            printf("Updating database at index %ld with time %6.3f\n", i, database[i].last_updated_time);
        }  
    }   
}

void receive_message() {
    create("receive_message");
    while(clock < SIM_TIME){
        hold(BROADCAST_INTERVAL);
        long request_item_id[NUM_CLIENTS];
        long i;
        for (i = 1; i <= NUM_CLIENTS; i++){
            receive(node[0].mbox, (q+i));
            printf("Receiving request from %ld\n", (q+i));
            request_item_id[i] = (q+i)->item_id;
            printf("Adding %ld to item_is_list from node address %ld\n", request_item_id[i], (q+i));
        }
        
    }
}


void client(long n) {
    create("client");
    printf("Client %ld is generated\n", n);

    /* Set up cache size */
    long i;
    for (i = 0; i < CACHE_SIZE; i++) {
        cache_size[n][i].id = 0;
        cache_size[n][i].last_updated_time = 0;
        cache_size[n][i].last_accessed_time = 0;
    }

    printf("Node %ld cache size address is %ld\n", n, &cache_size[n]);

    receive_ir(n);
    generate_query(n);
    while(clock < SIM_TIME){
        hold(1);  
    }
    
}

void generate_query(long i) {
    create("query");
    while(clock < SIM_TIME) {
        hold(MEAN_QUERY);       
        (q+i)->item_id = 1000000000000+i;
        send(node[0].mbox, (q+i)->item_id);
        printf("Requesting data_item ID from node %ld at address %ld with value %ld\n", i, (q+i), (q+i)->item_id);
    }  
}


void receive_ir(long n) {
    create("receive_ir");
    printf("receiving IR...\n");
    while(clock < SIM_TIME){
        hold(1);
        receive(node[n].mbox, (long *)&ir);
        int m;
        /*m = sizeof(ir);*/
        /*long i, j;
        for(i = 0; i < n; i++) {
            for (j = 0; j < CACHE_SIZE; j++) {
                if (ir[i].last_updated_time > cache_size[i].last_updated_time) {
                    cache_size[i].id = ir[i].id;
                    cache_size[i].last_updated_time = ir[i].last_updated_time;
                    cache_size[i].last_accessed_time = clock;
                } else {
                }
            }
        }*/
        printf("Node %ld receives IR %ld\n", n, &ir);
        
        /*for (i = 0; i < 5; i++) {
            printf("cache of node %ld at index %ld\n", n, cache_size[i].id);
        }*/ 
    }
    
}