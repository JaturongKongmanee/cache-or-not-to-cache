/* Project #3: Cache or Not to Cache by Kongmanee, Jaturong */
#include <csim.h>
#include <stdio.h>

/*
1. tydedef
2. struct
3. sprintf
*/

/* #define is used to define CONSTANT */
#define SIM_TIME 1000.0
#define NUM_CLIENTS 2l
#define NUM_SERVER 1l

/* typedef is used to give a name to a type (it can be user-defined type)*/
typedef struct msg *msg_t;
struct msg {
    long type;
    long from;
    long to;
    TIME start_time;
    msg_t link;
};

msg_t msg_queue;

struct nde {
    FACILITY cpu;
    MBOX mbox;
};

struct nde node[NUM_NODES];
FACILITY network[NUM_NODES][NUM_NODES];


void init();
/* server side and its function */
void server();
void invalidation_report();
void update_data_items();
void receive_message();

/* client side and its function*/
void client();
void query();
void receive_ir();


void sim() {
    create("sim");
    init();
    hold(SIM_TIME);
    /*while(clock < SIM_TIME){
        server();
        client();
    }*/
    
}

void init() {
    long i, j;
    char str[24];

    max_facilities(NUM_CLIENTS * NUM_CLIENTS + NUM_CLIENTS);
    max_servers(NUM_CLIENTS * NUM_CLIENTS + NUM_CLIENTS);
    max_mailboxes(NUM_CLIENTS + NUM_SERVER);
    max_events(2 * NUM_CLIENTS * NUM_CLIENTS);

    long all_node = NUM_CLIENTS + NUM_CLIENTS;
    for(i = 0; i < all_node; i++){
        sprintf(str, "cpu.%d", i);
        node[i].cpu = facility(str);
        sprintf(str, "input.%d", i);
        node[i].mbox = mailbox(str);
    }

    for(i = 0; i < NUM_SERVER; i++){
        for(j = 0; j < NUM_CLIENTS; j++){
            sprintf(str, "nt%d.%d", i, j);
            network[i][j] = facility(str);
        }
    }

    for(i = 0; i < NUM_CLIENTS; i++){
        client(i);
    }

    server(0);
}

void server(long n) {
    create("server");
    while(clock < SIM_TIME){
        invalidation_report();
        update_data_items();
    }   
}

void invalidation_report() {
    create("ir");
    while(clock < SIM_TIME){
        /* code */
    }
    
}

void invalidation_report() {
    create("update");
    while(clock < SIM_TIME){
        /* code */
    }
    
}

void receive_message() {
    create("receive_message");
    while(clock < SIM_TIME){
        /**/
    }
}


void client(long n) {
    create("client");
    while(clock < SIM_TIME){
        query();
        receive_ir();
    }
    
}

void query() {
    create("query");
    while(clock < SIM_TIME){
        /* code */
    }
    
}

void receive_ir() {
    create("receive_ir");
    while(clock < SIM_TIME){
        /* code */
    }
    
}