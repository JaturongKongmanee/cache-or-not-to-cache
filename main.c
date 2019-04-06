/* Project #3: Cache or Not to Cache by Kongmanee, Jaturong */

#include <csim.h>
#include <stdio.h>

/* #define is used to define CONSTANT */
#define SIM_TIME 1000.0
#define NUM_NODES 2l

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

/* server side and its function */
void server();
void invalidation_report();
void update_data_items();

/* client side and its function*/
void client();


void sim() {
    create("sim");
    while(clock < SIM_TIME){
        server();
        client();
    }
    
}

void server() {
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
