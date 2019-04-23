/* Project #3: Cache or Not to Cache by Kongmanee, Jaturong */
#include <csim.h>
#include <stdio.h>


/* #define is used to define CONSTANT */
#define SIM_TIME 5000.0
#define NUM_CLIENTS 5l
#define NUM_SERVER 1l

#define DB_SIZE 1000
#define HOT_DATA_ITEM_LIMIT 49
#define COLD_DATA_ITEM_START 50

#define CACHE_SIZE 100

#define BROADCAST_INTERVAL 20
#define T_UPDATE 10
#define T_QUERY 100
#define HOT_DATA_UPDATE_PROB 0.33
#define HOT_DATA_ACCESS_PROB 0.8


struct nde {
    MBOX mbox;
};
struct nde node[NUM_CLIENTS + NUM_SERVER];


/* structure for server side */
struct data_items {
    long id;
    TIME last_updated_time;
};
struct data_items database[DB_SIZE];

struct ir_msg {
    long id;
    TIME last_updated_time;
    long ir_size;
};
struct ir_msg *ir;
long l_bcast[100];


/* structure for client side */
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
long counter;

/* client side and its function*/
void client();
void generate_query();
void receive_ir();

/* utility function */
int is_cached();
long is_duplicated();
long get_ir_size();
void clear_ir_list();


/* measurement variable */
long cache_hit;
long cache_miss;
long T_update;

void sim() {

    printf("Enter Mean update arrival time (T_update) in seconds:\n");
    scanf("%lf", &T_update);
    printf("You've entered T_update: %lf\n", T_update);

    create("sim");
    init();
    hold(SIM_TIME);

    printf("#Cache hit: %ld #Cache miss: %ld\n", cache_hit, cache_miss);
    printf("#Cache hit ratio %.2f", cache_hit/(float)(cache_hit + cache_miss));

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

    /* allocate memory for query request */
    q = (struct request *)malloc(sizeof(struct request) * 5);
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
    receive_message();
    update_data_items();
    while(clock < SIM_TIME){
        hold(exponential(1));  
    }   
}

void invalidation_report() {
    create("ir");
    long ir_msg_size;
    ir = NULL;
    while(clock < SIM_TIME){
        hold(exponential(BROADCAST_INTERVAL));    
        /* Broadcast: put the message into all clients' mailboxes */
        /* Create a new IR */
        if (ir != NULL) {
            free(ir);
            ir = NULL;
            printf("Deallocating memory\n");
        }
        
        /* check size of l_bcast */
        /*ir_msg_size = random(1L, 5l);*/
        ir_msg_size = get_ir_size(l_bcast);
        ir = (struct ir_msg *)malloc(sizeof(struct ir_msg) * ir_msg_size);
        if (ir == NULL) {
            printf("Memory allocation error\n");
        }
        printf("Creating an IR ... with size %ld at address %ld\n", ir_msg_size, &ir);
        long i;
        for (i = 0; i < ir_msg_size; i++) {
            ir[i].id = database[l_bcast[i]].id;          
            ir[i].last_updated_time = database[l_bcast[i]].last_updated_time;
            ir[i].ir_size = ir_msg_size;
        }

        for (i = 1; i <= NUM_CLIENTS; i++) {   
            send(node[i].mbox, (long)ir);
            printf("Broadcasting to node %ld \n", i);
        }

        for (i = 0; i < ir_msg_size; i++) {
            printf("Test IR id %ld and updated time %6.3f with size %ld\n", ir[i].id, ir[i].last_updated_time, ir[i].ir_size);
        }

        /* clear l_bcast list */
        clear_ir_list(l_bcast);
        counter = 0;
    }
    
}

long get_ir_size(long l_bcast[]) {
    long i;
    long counter = 0;
    for (i = 0; i < 100; i++) {
        if (l_bcast[i] != 0) {
            counter++;
        }
    }
    return counter;
}

void clear_ir_list(long l_bcast[]) {
    long i;
    for (i = 0; i < 100; i++) {
        l_bcast[i] = 0;
    }      
}

void update_data_items() {
    create("update");
    printf("Updating data items\n");
    while(clock < SIM_TIME){
        hold(exponential(T_update));
        if (uniform(0.0, 1.0) <= 0.33) {
            /* rand() % (max_number + 1 - minimum_number) + minimum_number */
            /* rand() % (65 + 1 - 0) + 0 */
            long rand_hot_item = rand() % (HOT_DATA_ITEM_LIMIT + 1 - 0) + 0;
            database[rand_hot_item].last_updated_time = clock;
            printf("Updating HOT DATA ITEM at index %ld with time %6.3f\n", rand_hot_item, database[rand_hot_item].last_updated_time);
        } else {
            long rand_cold_item = rand() % ((DB_SIZE - 1) + 1 - COLD_DATA_ITEM_START) + COLD_DATA_ITEM_START;
            database[rand_cold_item].last_updated_time = clock;
            printf("Updating COLD DATA ITEM at index %ld with time %6.3f\n", rand_cold_item, database[rand_cold_item].last_updated_time);
        }
    }   
}

void receive_message() {
    create("receive_message");
    counter = 0;
    while(clock < SIM_TIME){
        hold(exponential(1));
        receive(node[0].mbox, (long*)&q);
        if (!is_duplicated(l_bcast, q->item_id) && counter < 100) {
            l_bcast[counter] = q->item_id;
            printf("Server receives query request id %ld\n", l_bcast[counter]);
            counter += 1;           
        }
    }
}

long is_duplicated(long l_bcast[], long id) {
    long i;
    for (i = 0; i < 100; i++) {
        if (id == l_bcast[i]) {
            return 1;
        } else {
            return 0;
        }
    }
}

void client(long n) {
    create("client");
    printf("Client %ld is generated\n", n);

    /* Set up cache size */
    long i;
    for (i = 0; i < CACHE_SIZE; i++) {
        cache_size[n][i].id = -1;
        cache_size[n][i].last_updated_time = 0;
        cache_size[n][i].last_accessed_time = 0;
    }

    printf("Node %ld, cache address is %ld\n", n, &cache_size[n]);

    receive_ir(n);
    generate_query(n);
    while(clock < SIM_TIME){
        hold(exponential(1));  
    }
    
}

void generate_query(long n) {
    create("query");
    while(clock < SIM_TIME) {
        hold(exponential(T_QUERY));
        if (uniform(0.0, 1.0) <= 0.8) {
            /* rand() % (max_number + 1 - minimum_number) + minimum_number */
            /* rand() % (65 + 1 - 0) + 0 */
            long rand_access_hot_item_id = rand() % (HOT_DATA_ITEM_LIMIT + 1 - 0) + 0;
            /* check cache */
            if (is_cached(n, rand_access_hot_item_id) != 1) {
                /* generate query request */
                q->item_id = rand_access_hot_item_id;
                send(node[0].mbox, (long)q);
                printf("Client %ld is generating query request at HOT data... with id %ld\n", n, rand_access_hot_item_id);
            }
        } else {
            long rand_access_cold_item_id = rand() % ((DB_SIZE - 1) + 1 - COLD_DATA_ITEM_START) + COLD_DATA_ITEM_START;
            /* check cache */
            if (is_cached(n, rand_access_cold_item_id) != 1) {
                q->item_id = rand_access_cold_item_id;
                send(node[0].mbox, (long)q);
                printf("Client %ld is generating query request at COLD data... with id %ld\n", n, rand_access_cold_item_id);
            }
        }
    }  
}

void receive_ir(long n) {
    create("receive_ir");
    printf("receiving IR...\n");
    while(clock < SIM_TIME){
        hold(exponential(1));
        receive(node[n].mbox, (long*)&ir);
        printf("Test ..................... receive IR function %ld\n", (long*)&ir);
        printf("Node %ld address %ld, receives IR size %ld\n", n, &cache_size[n], ir[0].ir_size);
        long i, j;
        for (i = 0; i < ir[0].ir_size; i++) {
            printf("IR info: --> id %ld and updated time %6.3f at node %ld\n", ir[i].id, ir[i].last_updated_time, n);
            for (j = 0; j < CACHE_SIZE; j++) {
                if (cache_size[n][j].id != -1) {
                    if (ir[i].id == cache_size[n][j].id && ir[i].last_updated_time >= cache_size[n][j].last_updated_time) {
                        cache_size[n][j].last_updated_time = ir[i].last_updated_time;
                        printf("UPDATED DATA at cache index %ld, updated time %6.3f\n", j, cache_size[n][j].last_updated_time);
                        break;
                    } 
                } else {
                    cache_size[n][j].id = ir[i].id;
                    cache_size[n][j].last_updated_time = ir[i].last_updated_time;
                    printf("NEW DATA at cache index %ld, updated time %6.3f\n", j, cache_size[n][j].last_updated_time);
                    break;
                }              
            }
        }
        printf("--------Cache details of Node %ld (first five cache items)--------\n", n);
        for (i = 0; i < 5; i ++) {
            printf("Node %ld, id %ld, updated_time %6.3f, access_time %6.3f\n", n, cache_size[n][i].id, cache_size[n][i].last_updated_time, cache_size[n][i].last_accessed_time);
        }
    } 
}

int is_cached(long n, long item_id) {
    long i;
    for (i = 0; i < CACHE_SIZE; i++) {
        if (cache_size[n][i].id == item_id) {
            cache_size[n][i].last_accessed_time = clock;
            if (clock > 1000) {
                cache_hit++;
            }
            return 1;
        } else {
            if (clock > 1000) {
                cache_miss++;
            }
            return 0;
        }
    }
}