/* Project #3: Cache or Not to Cache by Kongmanee, Jaturong */
#include <csim.h>
#include <stdio.h>


/* #define is used to define CONSTANT */
#define SIM_TIME 1000.0
#define NUM_CLIENTS 2l
#define NUM_SERVER 1l

#define DB_SIZE 100
#define CACHE_SIZE 10

#define BROADCAST_INTERVAL 20
#define T_UPDATE 20
#define T_QUERY 10
#define HOT_DATA_UPDATE_PROB 0.33
#define HOT_DATA_ACCESS_PROB 0.8


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

/* utility function */
int check_cache();


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

    q = (struct request *)malloc(sizeof(struct request) * 5);
    /*for (i = 1; i <= NUM_CLIENTS; i++) {
        (q+i)->item_id = 2222;
        printf("Allocating request for Node %ld at address %ld with value %ld\n", i, (q+i), *(q+i));
    }
    printf("Q address %ld", &q);*/
    
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
    /*receive_message();*/
    /*update_data_items();*/  
    while(clock < SIM_TIME){
        hold(1);  
    }   
}

void invalidation_report() {
    create("ir");
    long ir_size;
    while(clock < SIM_TIME){
        hold(BROADCAST_INTERVAL);    
        /* Broadcast: put the message into all clients' mailboxes */
        /* Create a new IR */
        long i;
        /*if (ir != NULL) {
            printf("Deallocating memory previously allocated by an IR");
            for (i = 0; i < ir_size; i++) {
                printf("---------------------------");
                /*free(ir[i].id);
                free(ir[i]);
            }
            /*free(ir);
        }*/
        
        ir_size = rand() % (49 + 1 - 0) + 0;
        ir = (struct data_item *)malloc(sizeof(struct data_items) * ir_size);
        printf("Creating an IR...with size %ld\n", ir_size);
        for (i = 0; i < ir_size; i++) {
            ir[i].id = i;
            ir[i].last_updated_time = clock;
        }
        for (i = 0; i < ir_size; i++) {
            printf("Test ir id %ld and time %6.3f\n", ir[i].id, ir[i].last_updated_time);
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
        hold(T_UPDATE);
        if (uniform(0.0, 1.0) <= 0.33) {
            /* rand() % (max_number + 1 - minimum_number) + minimum_number */
            /* rand() % (65 + 1 - 0) + 0 */
            long rand_hot_item = rand() % (49 + 1 - 0) + 0;
            database[rand_hot_item].last_updated_time = clock;
            printf("Updating HOT DATA ITEM at index %ld with time %6.3f\n", rand_hot_item, database[rand_hot_item].last_updated_time);
        } else {
            long rand_cold_item = rand() % ((DB_SIZE - 1) + 1 - 50) + 50;
            database[rand_cold_item].last_updated_time = clock;
            printf("Updating COLD DATA ITEM at index %ld with time %6.3f\n", rand_cold_item, database[rand_cold_item].last_updated_time);
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
    /*generate_query(n);*/
    while(clock < SIM_TIME){
        hold(1);  
    }
    
}

void generate_query(long n) {
    create("query");
    while(clock < SIM_TIME) {
        hold(T_QUERY);
        if (uniform(0.0, 1.0) <= 0.8) {
            long rand_access_hot_item = rand() % (49 + 1 - 0) + 0;
            /* check cache */
            if (is_cached(n, rand_access_hot_item) != 1) {
                /* generate query request */
                printf("generating query request at hot data... at %ld index %ld\n", n, rand_access_hot_item);
            }
        } else {
            long rand_access_cold_item = rand() % ((DB_SIZE - 1) + 1 - 50) + 50;
            /* check cache */
            if (is_cached(n, rand_access_cold_item) != 1) {
                /* generate query request */
                printf("generating query request at cold data... at %ld index %ld\n", n, rand_access_cold_item);
            }
        }
        /*(q+i)->item_id = 1000000000000+i;
        send(node[0].mbox, (q+i)->item_id);
        printf("Requesting data_item ID from node %ld at address %ld with value %ld\n", i, (q+i), (q+i)->item_id);*/
    }  
}

void receive_ir(long n) {
    create("receive_ir");
    printf("receiving IR...\n");
    while(clock < SIM_TIME){
        hold(1);
        receive(node[n].mbox, (long *)&ir);
        int m;
        m = sizeof(ir);
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
        printf("Node %ld receives IR size %ld %ld\n", n, m, &ir);
        long i;
        for (i = 0; i < m; i++) {
            printf("IR id %ld and time %6.3f at node %ld\n", ir[i].id, ir[i].last_updated_time, n);
        }
        
        /*for (i = 0; i < 5; i++) {
            printf("cache of node %ld at index %ld\n", n, cache_size[i].id);
        }*/ 
    } 
}

int is_cached(long cache_num, long item) {
    if (cache_size[cache_num][item].last_accessed_time != 0) {
        return 1;
    } else {
        return 0;
    }
}