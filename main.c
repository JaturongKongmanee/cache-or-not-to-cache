/* Project #3: Cache or Not to Cache by Kongmanee, Jaturong */
#include <csim.h>
#include <stdio.h>


/* #define is used to define CONSTANT */
#define SIM_TIME 100.0
#define NUM_CLIENTS 2l
#define NUM_SERVER 1l
#define NUM_BOXES 3

#define DB_SIZE 100
#define CACHE_SIZE 10

#define BROADCAST_INTERVAL 5
#define MEAN_UPDATE 20
#define MEAN_QUERY 25

/* typedef is used to give a name to a type (it can be user-defined type)*/
/*typedef struct msg *msg_t;
struct msg {
    long type;
    long from;
    long to;
    TIME start_time;
    msg_t link;
};*/

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
    long accessed_number;
};
struct cache_items cache_size[CACHE_SIZE];


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
    
    long all_node = NUM_CLIENTS + NUM_CLIENTS;

    for (i = 0; i < all_node; i++) {
        sprintf(str, "input.%d", i);
        node[i].mbox = mailbox(str);
    }

    server(0);
    printf("Calling server\n");

    for (i = 0; i < NUM_CLIENTS; i++) {
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
    update_data_items();
    /*receive_message();*/
    while(clock < SIM_TIME){
        hold(1);
        
    }   
}

void invalidation_report() {
    create("ir"); 
    printf("IR is generated\n");
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
            send(node[i].mbox, ir);
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

    }
}


void client(long n) {
    create("client");
    printf("Client %ld is generated\n", n);

    /* Set up cache size */
    long i;
    for (i = 0; i < CACHE_SIZE; i++) {
        cache_size[i].id = i;
        cache_size[i].last_updated_time = i;
        cache_size[i].accessed_number = i;
    }

    receive_ir(n);
    /*generate_query();*/
    while(clock < SIM_TIME){
        hold(1);  
    }
    
}

void generate_query() {
    create("query");
    while(clock < SIM_TIME) {
        hold(MEAN_QUERY);

    }  
}

void receive_ir(long n) {
    create("receive_ir");
    printf("receiving messages\n");
    while(clock < SIM_TIME){
        hold(BROADCAST_INTERVAL);
        receive(node[n].mbox, ir);
        long i;
        for(i = 0; i < 10; i++) {
            printf("Node %ld received message %6.3f\n", n, ir[i].last_updated_time);
        }
        
        for (i = 0; i < 5; i++) {
            printf("cache of node %ld at index %ld\n", n, cache_size[i].id);
        }   
    }
    
}