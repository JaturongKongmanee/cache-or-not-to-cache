/* Project #3: Cache or Not to Cache by Kongmanee, Jaturong */
#include <csim.h>
#include <stdio.h>

/*
TO-DO list
1.) query delay
2.) number query served per IR
3.) send data items in the IR list one by one
*/



/* #define is used to define CONSTANT */
#define SIM_TIME 1000.0
#define NUM_CLIENTS 3l
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
long ir_temp[100];


/* structure for client side */
struct cache_items {
    int valid;
    long id;
    TIME last_updated_time;
    TIME last_accessed_time;
};

/* Since we know exactly the size of cache,
and we randomly read/access data items so many time.
It's good to use array since its reading time complexity is O(1)*/
struct cache_items cache_size[NUM_CLIENTS + 1][CACHE_SIZE];

struct request {
    long item_id;
};
struct request *q;


void init();
/* server side and its function */
void server();
void update_data_items();
void invalidation_report();
void receive_message();
long counter;

/* client side and its function*/
void client();
void generate_query();
void receive_ir();

/* utility function */
int is_cached();
int is_duplicated();
int get_ir_size();
void clear_ir_list();

/* measurement variable */
long cache_hit;
long cache_miss;
long T_update;
long T_query;
long query_count;


void sim() {

    printf("Enter Mean update arrival time (T_update) in seconds:\n");
    scanf("%ld", &T_update);
    printf("You've entered T_update: %ld\n", T_update);

    printf("Enter Mean query generate time (T_update) in seconds:\n");
    scanf("%ld", &T_query);
    printf("You've entered T_query: %ld\n", T_query);

    create("sim");
    init();
    hold(SIM_TIME);

    printf("#Cache hit: %ld #Cache miss: %ld\n", cache_hit, cache_miss);
    printf("#Cache hit ratio %.2f\n", cache_hit/(float)(cache_hit + cache_miss));

}

void init() {
    long i;
    char str[24];

    max_events(NUM_CLIENTS * NUM_CLIENTS + NUM_CLIENTS);
    max_mailboxes(NUM_CLIENTS * NUM_CLIENTS + NUM_CLIENTS);                     
    max_messages(NUM_CLIENTS * NUM_CLIENTS * 100 + NUM_CLIENTS);

    sprintf(str, "server_mailbox.%d", 0);
    node[0].mbox = mailbox(str);

    for (i = 1; i <= NUM_CLIENTS; i++) {
        sprintf(str, "client_mailbox.%d", i);
        node[i].mbox = mailbox(str);
    }

    server(0);
    printf("Calling server\n");

    for (i = 1; i <= NUM_CLIENTS; i++) {
        client(i);
        printf("Calling client %ld\n", i);
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
    printf("Setting up a database ...\n");
    
    update_data_items(); 
    /*receive_message();*/
    invalidation_report();

    while(clock < SIM_TIME){
        hold(exponential(1));  
    }   
}

void invalidation_report() {
    create("ir");
    long ir_msg_size;
    ir = NULL;
    printf("Creating IR ...\n");
    while(clock < SIM_TIME){
        hold(BROADCAST_INTERVAL);    
        /* Broadcast: send the IR into all clients' mailboxes */
        /* Create a new IR */
        if (ir != NULL) {
            free(ir);
            ir = NULL;
            printf("Deallocating IR's memory ... \n");
        }
        
        /* check size of ir_temp */
        /*ir_msg_size = random(1L, 5l);*/
        ir_msg_size = get_ir_size(ir_temp);
        ir = (struct ir_msg *)malloc(sizeof(struct ir_msg) * ir_msg_size);
        if (ir == NULL) {
            printf("Memory allocation error\n");
        }
        printf("Creating an IR ... with size %ld at address %ld\n", ir_msg_size, &ir);
        long i;
        for (i = 0; i < ir_msg_size; i++) {
            /* Segmentation fault: ir_temp[i] = 4640444542306902472 for example */
            if (ir_temp[i] < 1000) {
                ir[i].id = database[ir_temp[i]].id;          
                ir[i].last_updated_time = database[ir_temp[i]].last_updated_time;
                ir[i].ir_size = ir_msg_size;
            }           
        }

        for (i = 1; i <= NUM_CLIENTS; i++) {   
            send(node[i].mbox, (long)ir);
            printf("Broadcasting to node %ld \n", i);
        }

        for (i = 0; i < ir_msg_size; i++) {
            printf("Testing IR ... data item id %ld, updated time %6.3f, IR size %ld\n", ir[i].id, ir[i].last_updated_time, ir[i].ir_size);
        }

        /* clear ir_temp list */
        clear_ir_list(ir_temp);
        counter = 0;
        printf("Counter is set to 0\n");

        /* broadcast L_bcast */
    }
    
}

int get_ir_size(long ir_temp[]) {
    int i;
    int counter = 0;
    printf("Getting IR size\n");
    for (i = 0; i < 100; i++) {
        if (ir_temp[i] != -1 && ir_temp[i] != 0) {
            printf("IR id %ld\n", ir_temp[i]);
            counter++;
        }
    }
    return counter;
}

void clear_ir_list(long ir_temp[]) {
    long i;
    for (i = 0; i < 100; i++) {
        ir_temp[i] = 0;
    }      
}

void update_data_items() {
    create("update");
    printf("Updating data items ...\n");
    while(clock < SIM_TIME){
        hold(exponential(T_update));
        if (uniform(0.0, 1.0) <= 0.33) {
            long rand_hot_item = random(0, HOT_DATA_ITEM_LIMIT);
            database[rand_hot_item].last_updated_time = clock;
            printf("Updating HOT DATA ITEM at index %ld, updated time %6.3f\n", rand_hot_item, database[rand_hot_item].last_updated_time);
            ir_temp[counter] = database[rand_hot_item].id;
            counter++;
            printf("Adding updated hot data item id %ld to IR %ld, counter %ld\n", ir_temp[counter-1], database[rand_hot_item].id, counter-1);
        } else {
            long rand_cold_item =random(COLD_DATA_ITEM_START, DB_SIZE);
            database[rand_cold_item].last_updated_time = clock;
            printf("Updating COLD DATA ITEM at index %ld, updated time %6.3f\n", rand_cold_item, database[rand_cold_item].last_updated_time);
            ir_temp[counter] = database[rand_cold_item].id;
            counter++;
            printf("Adding updated cold data item id %ld to IR %ld, counter %ld\n", ir_temp[counter-1], database[rand_cold_item].id, counter-1);
        }
    }   
}

void receive_message() {
    create("receive_message");
    counter = 0;
    status_mailboxes();
    while(clock < SIM_TIME){
        hold(exponential(1));
        receive(node[0].mbox, (long*)&q);
        /*printf("Server is receiving query %ld\n", q->item_id);*/
        if (!is_duplicated(ir_temp, q->item_id) && counter < 100) {
            ir_temp[counter] = q->item_id;
            printf("Server receives query request id %ld and counter = %ld\n", ir_temp[counter], counter);  
            counter++;                    
        }
    }
}

int is_duplicated(long ir_temp[], long id) {
    int i;
    for (i = 0; i < 100; i++) {
        if (id == ir_temp[i]) {
            printf("Id %ld is duplicated\n", id);
            return 1;
        } else {
            printf("Id %ld is not duplicated\n", id);
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
        cache_size[n][i].valid = 0;
        cache_size[n][i].id = -1;
        cache_size[n][i].last_updated_time = 0;
        cache_size[n][i].last_accessed_time = 0;
    }

    printf("Node %ld, cache address is %ld\n", n, &cache_size[n]);

    receive_ir(n);
    /*generate_query(n);*/
    while(clock < SIM_TIME){
        hold(exponential(1));
    }   
}

void generate_query(long n) {
    create("query");
    while(clock < SIM_TIME) {
        hold(exponential(T_query));
        if (uniform(0.0, 1.0) <= 0.8) {
            /* rand() % (max_number + 1 - minimum_number) + minimum_number */
            /* rand() % (65 + 1 - 0) + 0 */
            long rand_access_hot_item_id = rand() % (HOT_DATA_ITEM_LIMIT + 1 - 0) + 0;
            /* check cache */
            if (!is_cached(n, rand_access_hot_item_id)) {
                /* generate query request */
                q->item_id = rand_access_hot_item_id;
                send(node[0].mbox, (long)q);
                printf("Client %ld is generating query request at HOT data... with id %ld %6.3f\n", n, rand_access_hot_item_id, clock);
            }
        } else {
            long rand_access_cold_item_id = rand() % ((DB_SIZE - 1) + 1 - COLD_DATA_ITEM_START) + COLD_DATA_ITEM_START;
            /* check cache */
            if (!is_cached(n, rand_access_cold_item_id)) {
                q->item_id = rand_access_cold_item_id;
                send(node[0].mbox, (long)q);
                printf("Client %ld is generating query request at COLD data... with id %ld %6.3f\n", n, rand_access_cold_item_id, clock);
            }
        }
    }  
}

int is_cached(long n, long item_id) {
    long cached = 0;
    long i;
    for (i = 0; i < CACHE_SIZE; i++) {
        if (cache_size[n][i].id == item_id) {
            cache_size[n][i].last_accessed_time = clock;
            if (clock > 500) {
                cache_hit++;
            }
            printf("node %ld CACHE HIT id %ld ---- generated query id %ld\n", n, cache_size[n][i].id, item_id);
            cached = 1;
        }
    }
    if (clock > 500) {
        cache_miss++;
    }

    if (cached != 1) {
        printf("node %ld CACHE MISS id %ld\n", n, item_id);
    }

    return cached;
}

void receive_ir(long n) {
    create("receive_ir");
    printf("receiving IR...\n");
    while(clock < SIM_TIME){
        hold(exponential(1));
        receive(node[n].mbox, (long*)&ir);
        printf("Test ..................... receive IR function %ld\n", (long*)&ir);
        printf("Node %ld address %ld, receives IR size %ld\n", n, &cache_size[n], ir[0].ir_size);
        int i, j;
        for (i = 0; i < ir[0].ir_size; i++) {
            for (j = 0; j < CACHE_SIZE; j++) {
                if (cache_size[n][j].id == ir[i].id) {
                    cache_size[n][j].valid = 0;
                    printf("Invalidating cache id %ld, valid %ld, updated time %6.3f ... at node %ld\n", cache_size[n][j].id, cache_size[n][j].valid, cache_size[n][j].last_updated_time, n);
                }           
            }
        }


        printf("--------Cache details of Node %ld (first five cache items)--------\n", n);
        for (i = 0; i < 5; i ++) {
            printf("Node %ld, valid %ld, id %ld, updated_time %6.3f, access_time %6.3f\n", n, cache_size[n][i].valid, cache_size[n][i].id, cache_size[n][i].last_updated_time, cache_size[n][i].last_accessed_time);
        }

        /* LRU section */
        /*printf("-------------------------LRU performing at node %ld-------------------------\n", n);
        int lru_idx;
        long lru_time;
        for (i = 0; i < ir[0].ir_size; i++) {
            if (ir[i].id != -1) { 
                lru_idx = 0;
                lru_time = cache_size[n][0].last_accessed_time;
                for (j = 1; j < CACHE_SIZE; j++) {
                    if (cache_size[n][j].last_accessed_time < lru_time) {
                        lru_time = cache_size[n][j].last_accessed_time;
                        lru_idx = j;
                    }
                }
                printf("LRU idx %ld\n", lru_idx);
                cache_size[n][lru_idx].id = ir[i].id;
                cache_size[n][lru_idx].last_updated_time = ir[i].last_updated_time;
                cache_size[n][lru_idx].last_accessed_time = clock;
            }
        }*/   
    } 
}