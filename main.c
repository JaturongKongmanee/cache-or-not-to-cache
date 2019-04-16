/* Project #3: Cache or Not to Cache by Kongmanee, Jaturong */
#include <csim.h>
#include <stdio.h>


/* #define is used to define CONSTANT */
#define SIM_TIME 100.0
#define NUM_CLIENTS 2l
#define NUM_SERVER 1l
#define NUM_BOXES 3

#define DB_SIZE 3
#define CACHE_SIZE 2

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
struct cache_items cache_size[CACHE_SIZE];

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

        for (i = 0; i < NUM_CLIENTS; i++) {   
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
        receive(node[0].mbox, (long *)&q);
        printf("Receiving request from %ld\n", &q);
    }
}


void client(long n) {
    create("client");
    printf("Client %ld is generated\n", n);

    /* Set up cache size */
    long i;
    for (i = 0; i < CACHE_SIZE; i++) {
        cache_size[i].id = 0;
        cache_size[i].last_updated_time = 0;
        cache_size[i].last_accessed_time = 0;
    }

    printf("Node %ld cache size address is %ld\n", n, &cache_size);

    receive_ir(n);
    generate_query(n);
    while(clock < SIM_TIME){
        hold(1);  
    }
    
}

void generate_query(long n) {
    create("query");
    while(clock < SIM_TIME) {
        hold(MEAN_QUERY);       
        q = (struct request *)malloc(sizeof(*q));
        q->item_id = 777;
        send(node[0].mbox, q);
        printf("Requesting data_item ID from node %ld at address %ld...\n", n, &q);
    }  
}


void receive_ir(long n) {
    create("receive_ir");
    printf("receiving IR...\n");
    while(clock < SIM_TIME){
        hold(1);
        receive(node[n].mbox, (long *)&ir);
        int n;
        n = sizeof(ir);
        long i, j;
        for(i = 0; i < n; i++) {
            for (j = 0; j < CACHE_SIZE; j++) {
                /* not cached */
                if (ir[i].last_updated_time > cache_size[i].last_updated_time) {
                    cache_size[i].id = ir[i].id;
                    cache_size[i].last_updated_time = ir[i].last_updated_time;
                    cache_size[i].last_accessed_time = clock;
                } else {
                    /* cached */
                }
            }
        }
        printf("Node %ld receives IR %ld\n", n, &ir);
        
        /*for (i = 0; i < 5; i++) {
            printf("cache of node %ld at index %ld\n", n, cache_size[i].id);
        }*/ 
    }
    
}