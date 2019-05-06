/* Project #3: Cache or Not to Cache by Kongmanee, Jaturong */
#include <csim.h>
#include <stdio.h>

/* #define is used to define CONSTANT */
#define SIM_TIME 50000.0
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
    int ir_status;
};
struct ir_msg *ir;
struct ir_msg *data_item_msg;
long ir_temp[100];
long l_bcast[100];


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
void receive_request();
long ir_counter;
long query_counter;
long ir_msg_size;
long bcast_list_size;

/* client side and its function*/
void client();
void generate_query();
void receive_ir();

/* utility function */
int is_cached();
int is_duplicated();
int get_list_size();
void clear_list();
int is_cache_full();
int get_oldest_invalid();
int get_oldest_valid();

/* measurement variable */
long cache_hit;
long cache_miss;
long T_update;
long T_query;
long query_count;
long num_query_per_interval;
TIME q_gen;
TIME q_delay;

void sim() {

    printf("Enter Mean update arrival time (T_update) in seconds:\n");
    scanf("%ld", &T_update);
    printf("You've entered T_update: %ld\n", T_update);

    printf("Enter Mean query generate time (T_query) in seconds:\n");
    scanf("%ld", &T_query);
    printf("You've entered T_query: %ld\n", T_query);

    create("sim");
    init();
    hold(SIM_TIME);

    printf("Enter Mean update arrival time (T_update) in seconds: %ld\n", T_update);
    printf("Enter Mean query generate time (T_query) in seconds: %ld\n", T_query);
    printf("#Cache hit: %ld ------ #Cache miss: %ld\n", cache_hit, cache_miss);
    printf("#Cache hit ratio %.2f\n", cache_hit/(float)(cache_hit + cache_miss));
    printf("Query delay (seconds): %12.3f\n", (q_delay/(float)NUM_CLIENTS)/(SIM_TIME/BROADCAST_INTERVAL));
    printf("#Queries served per interval: raw %ld, avg: %3.5f\n", num_query_per_interval, num_query_per_interval/((SIM_TIME)/BROADCAST_INTERVAL));

}

void init() {
    long i;
    char str[24];

    max_events(NUM_CLIENTS * NUM_CLIENTS * 100 + NUM_CLIENTS);
    max_mailboxes(NUM_CLIENTS * NUM_CLIENTS * 100 + NUM_CLIENTS);                     
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
    receive_request();
    invalidation_report();

    while(clock < SIM_TIME){
        hold(exponential(1));  
    }   
}

void invalidation_report() {
    create("ir");
    
    ir = NULL;
    data_item_msg = NULL;
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
        ir_msg_size = get_list_size(ir_temp, 0);
        if (ir_msg_size > 0) {
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
                    ir[i].ir_status = 1;
                }           
            }

            for (i = 0; i < ir_msg_size; i++) {
                printf("Testing IR ... data item id %ld, updated time %6.3f, IR size %ld\n", ir[i].id, ir[i].last_updated_time, ir[i].ir_size);
            }

            for (i = 1; i <= NUM_CLIENTS; i++) {   
                send(node[i].mbox, (long)ir);
                printf("Broadcasting IR to node %ld \n", i);
            }

            /* clear ir_temp list */
            clear_list(ir_temp);
            ir_counter = 0;
            printf("IR counter is set to 0\n");
        }

        /* broadcast L_bcast */
        /*if (data_item_msg != NULL) {
            free(data_item_msg);
            data_item_msg = NULL;
            printf("Deallocating DATA ITEM's memory ... \n");
        }*/
   
        bcast_list_size = get_list_size(l_bcast, 1);
        num_query_per_interval += bcast_list_size;
        if (bcast_list_size > 0) {
            data_item_msg = (struct ir_msg *)malloc(sizeof(struct ir_msg) * bcast_list_size);
            if (data_item_msg == NULL) {
                printf("Memory allocation error\n");
            }

            printf("Creating an L_bcast ... with size %ld at address %ld\n", bcast_list_size, &data_item_msg);
            long i;
            for (i = 0; i < bcast_list_size; i++) {
                if (l_bcast[i] < 1000) {
                    data_item_msg[i].id = database[l_bcast[i]].id;          
                    data_item_msg[i].last_updated_time = database[l_bcast[i]].last_updated_time;
                    data_item_msg[i].ir_size = bcast_list_size;
                    data_item_msg[i].ir_status = 0;
                }           
            }

            for (i = 1; i <= NUM_CLIENTS; i++) {   
                send(node[i].mbox, (long)(data_item_msg));
                printf("Broadcasting DATA ITEM to node %ld \n", i);
            }

            for (i = 0; i < bcast_list_size; i++) {
                printf("Testing DATA ITEM ... id %ld, updated time %6.3f, IR size %ld\n", data_item_msg[i].id, data_item_msg[i].last_updated_time, data_item_msg[i].ir_size);
            }

            clear_list(l_bcast);
            query_counter = 0;
            printf("QUERY counter is set to 0\n");      
        }
    }  
}

int get_list_size(long list[], long bit) {
    int i;
    int counter = 0;
    if (bit == 0) {
        printf("Getting IR LIST size ... \n");
    } else {
        printf("Getting L_bcast LIST size ... \n");
    }
    
    for (i = 0; i < 100; i++) {
        if (list[i] != -1 && list[i] != 0) {
            /*printf("IR id %ld\n", ir_temp[i]);*/
            counter++;
        }
    }
    return counter;
}

void clear_list(long list[]) {
    long i;
    for (i = 0; i < 100; i++) {
        list[i] = 0;
    }      
}

void update_data_items() {
    create("update");
    printf("Updating data items ...\n");
    ir_counter = 0;
    while(clock < SIM_TIME){
        hold(exponential(T_update));
        if (uniform(0.0, 1.0) <= 0.33) {
            long rand_hot_item = random(0, HOT_DATA_ITEM_LIMIT);
            database[rand_hot_item].last_updated_time = clock;
            printf("Updating HOT DATA ITEM at index %ld, updated time %6.3f\n", rand_hot_item, database[rand_hot_item].last_updated_time);
            ir_temp[ir_counter] = database[rand_hot_item].id;
            ir_counter++;
            /*printf("Adding updated hot data item id %ld to IR %ld, counter %ld\n", ir_temp[ir_counter-1], database[rand_hot_item].id, ir_counter-1);*/
        } else {
            long rand_cold_item = random(COLD_DATA_ITEM_START, DB_SIZE);
            database[rand_cold_item].last_updated_time = clock;
            printf("Updating COLD DATA ITEM at index %ld, updated time %6.3f\n", rand_cold_item, database[rand_cold_item].last_updated_time);
            ir_temp[ir_counter] = database[rand_cold_item].id;
            ir_counter++;
            /*printf("Adding updated cold data item id %ld to IR %ld, counter %ld\n", ir_temp[ir_counter-1], database[rand_cold_item].id, ir_counter-1);*/
        }
    }   
}

void receive_request() {
    create("receive_request");
    query_counter = 0;
    status_mailboxes();
    while(clock < SIM_TIME){
        hold(exponential(1));
        receive(node[0].mbox, (long*)&q);
        /*printf("Server is receiving query %ld\n", q->item_id);*/
        if (!is_duplicated(l_bcast, q->item_id) && query_counter < 100) {
            l_bcast[query_counter] = q->item_id;
            printf("Server RECEIVES QUERY request id %ld\n", l_bcast[query_counter]);  
            query_counter++;                    
        }
    }
}

int is_duplicated(long list[], long id) {
    int i;
    for (i = 0; i < 100; i++) {
        if (id == list[i]) {
            printf("Id %ld is duplicated\n", id);
            return 1;
        } else {
            printf("Id %ld is NOT duplicated\n", id);
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
    generate_query(n);
    while(clock < SIM_TIME){
        hold(exponential(1));
    }   
}

void generate_query(long n) {
    create("query");
    while(clock < SIM_TIME) {
        hold(exponential(T_query));
        if (uniform(0.0, 1.0) <= 0.8) {
            long rand_access_hot_item_id = random(0, HOT_DATA_ITEM_LIMIT);
            /* check cache */
            if (!is_cached(n, rand_access_hot_item_id)) {
                /* generate query request */
                q->item_id = rand_access_hot_item_id;
                send(node[0].mbox, (long)q);
                printf("Client %ld is GENERATING QUERY request at HOT data... with id %ld at %6.3f\n", n, rand_access_hot_item_id, clock);
                q_gen = clock;
            }
        } else {
            long rand_access_cold_item_id = random(COLD_DATA_ITEM_START, DB_SIZE);
            if (!is_cached(n, rand_access_cold_item_id)) {
                q->item_id = rand_access_cold_item_id;
                send(node[0].mbox, (long)q);
                printf("Client %ld is GENERATING QUERY request at COLD data... with id %ld at %6.3f\n", n, rand_access_cold_item_id, clock);
                q_gen = clock;
            }
        }
    }  
}

int is_cached(long n, long item_id) {
    int cached = 0;
    long i;
    for (i = 0; i < CACHE_SIZE; i++) {
        if (cache_size[n][i].id == item_id) {
            cache_size[n][i].last_accessed_time = clock;
            if (is_cache_full(n)) {
                cache_hit++;
            }
            printf("node %ld CACHE HIT id %ld ---- generated query id %ld\n", n, cache_size[n][i].id, item_id);
            cached = 1;
        }
    }
    if (is_cache_full(n)) {
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
        if (ir[0].ir_status == 1) {
            printf("Node %ld address %ld, receives IR size %ld\n", n, &cache_size[n], ir[0].ir_size);
        } else {
            printf("Node %ld address %ld, receives DATA ITEM size %ld\n", n, &cache_size[n], ir[0].ir_size);
        }

        if (ir[0].ir_status == 1) {
            int i, j;
            for (i = 0; i < ir[0].ir_size; i++) {
                printf("IR id %ld\n", ir[i].id);
                for (j = 0; j < CACHE_SIZE; j++) {
                    if (cache_size[n][j].id == ir[i].id) {
                        cache_size[n][j].valid = 0;
                        printf("Invalidating cache id %ld, valid %ld, updated time %6.3f ... at node %ld\n", cache_size[n][j].id, cache_size[n][j].valid, cache_size[n][j].last_updated_time, n);
                    }           
                }
            }
        } else {
            int i, j;
            for (i = 0; i < ir[0].ir_size; i++) {
                printf("DATA ITEM id %ld\n", ir[i].id);
                if (!is_cache_full(n)) {
                    for (j = 0; j < CACHE_SIZE; j++) {
                        if (cache_size[n][j].id == -1) { 
                            cache_size[n][j].id = ir[i].id;
                            cache_size[n][j].valid = 1;
                            cache_size[n][j].last_updated_time = ir[i].last_updated_time;
                            cache_size[n][j].last_accessed_time = 0;
                            printf("NEW DATA ITEM is cached: id %ld, valid %ld, last updated time %6.3f, last accessed time %6.3f\n", cache_size[n][j].id, cache_size[n][j].valid, cache_size[n][j].last_updated_time, cache_size[n][j].last_accessed_time);
                            break;
                        }
                    }
                } else {
                    int updated = 0;
                    for (j = 0; j < CACHE_SIZE; j++) {
                        if (ir[i].id == cache_size[n][j].id && cache_size[n][j].valid == 0) {
                            cache_size[n][j].valid = 1;
                            cache_size[n][j].last_updated_time = ir[i].last_updated_time;
                            printf("CACHED DATA ITEM is updated: id %ld, valid %ld, last updated time %6.3f, last accessed time %6.3f\n", cache_size[n][j].id, cache_size[n][j].valid, cache_size[n][j].last_updated_time, cache_size[n][j].last_accessed_time);   
                            updated = 1;
                            q_delay += clock - q_gen;
                            break;
                        }
                    }
                    if (!updated) {
                        int inv_idx = get_oldest_invalid(n);
                        int val_idx = get_oldest_valid(n);
                        if (inv_idx != 0) {
                            printf("OLDEST INVALID data item id %ld idx %ld --- ", cache_size[n][inv_idx].id, inv_idx);
                            cache_size[n][inv_idx].id = ir[i].id;
                            cache_size[n][inv_idx].valid = 1;
                            cache_size[n][inv_idx].last_updated_time = ir[i].last_updated_time;
                            cache_size[n][inv_idx].last_accessed_time = 0;
                            printf("is replaced by new data item %ld\n", cache_size[n][inv_idx].id);
                            q_delay += clock - q_gen;
                        } 
                        else if (val_idx != 0) {
                            printf("OLDEST VVVVALID data item id %ld idx %ld --- ", cache_size[n][val_idx].id, val_idx);
                            cache_size[n][val_idx].id = ir[i].id;
                            cache_size[n][val_idx].valid = 1;
                            cache_size[n][val_idx].last_updated_time = ir[i].last_updated_time;
                            cache_size[n][val_idx].last_accessed_time = 0;
                            printf("is replaced by new data item %ld\n", cache_size[n][val_idx].id);
                            q_delay += clock - q_gen;
                        } 
                    }
                }
            }
        }
    }
}

int get_oldest_invalid(long n) {
    int k;
    int lru_idx = 0;
    long lru_time = cache_size[n][0].last_updated_time;
    for (k = 1; k < CACHE_SIZE; k++) {
        if (cache_size[n][k].valid == 0 && cache_size[n][k].last_updated_time <= lru_time) {
            lru_time = cache_size[n][k].last_updated_time;
            lru_idx = k;
        }
    }
    return lru_idx;
}

int get_oldest_valid(long n) {
    int k;
    int lru_idx = 0;
    long lru_time = cache_size[n][0].last_accessed_time;
    for (k = 1; k < CACHE_SIZE; k++) {
        if (cache_size[n][k].valid == 1 && cache_size[n][k].last_accessed_time <= lru_time) {
            lru_time = cache_size[n][k].last_accessed_time;
            lru_idx = k;
        }
    }
    return lru_idx;
}


int is_cache_full(long n) {
    int full = 1;
    long i;
    for (i = 0; i < CACHE_SIZE; i++) {
        if (cache_size[n][i].id == -1) {
            full = 0;
            return full;
        }
    }
    if (full) {
        printf("node %ld cache is full\n", n);
    }
    return full;
}