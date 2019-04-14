/* Project #3: Cache or Not to Cache by Kongmanee, Jaturong */
#include <csim.h>
#include <stdio.h>

/*
1. tydedef
2. struct
3. sprintf
*/

/* #define is used to define CONSTANT */
#define SIM_TIME 5.0
#define NUM_CLIENTS 2l
#define NUM_SERVER 1l
#define NUM_BOXES 3

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


struct message {
    int id;
    /* represented by the number of update */
    int last_updated_time;

};

/* struct message data_list[100];*/

long msg;

MBOX mbox_arr[NUM_BOXES];


TABLE resp_tm;


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
    /*server();
    client();*/
    hold(SIM_TIME);
    /*while(clock < SIM_TIME){
        server();
        client();
    }*/
    
}

void init() {
    long i;
    char str[24];
    
    /*for (i = 0; i < 100; i++) {
         data_list[i].id = i;
         data_list[i].last_updated_time = 0;
    }*/

    long all_node = NUM_CLIENTS + NUM_CLIENTS;
    max_mailboxes(all_node);
    max_servers(all_node);
    max_events(all_node);

    for (i = 0; i < all_node; i++) {
        sprintf(str, "input.%d", i);
        node[i].mbox = mailbox(str);
    }

    /*create_mailbox_set("mbox set", mbox_arr, NUM_BOXES);*/

    resp_tm = table("msg rsp tm");

    server(0);
    for (i = 0; i < NUM_CLIENTS; i++) {
        client(i);
        printf("test %ld\n", i);
    }

    
}

void server(long n) {
    create("server");
    printf("Server is generated");
    while(clock < SIM_TIME){
        invalidation_report();
        /*update_data_items();
        receive_message();*/
    }   
}

void invalidation_report() {
    create("ir");
    printf("IR is generated");
    while(clock < SIM_TIME){
        hold(5);
        /* Broadcast: put the message into all clients' mailboxes */
        long i;
        for(i = 0; i < NUM_CLIENTS; i++) {
            /* Transmission delay */
            /* hold(2); */
            long msg1;
            msg1 = 5;
            send(node[i].mbox, msg1);
            printf("sent message to node %ld\n", i);
        }
    }
    
}

void update_data_items() {
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
    printf("Client %ld is generatedddd", n);
    while(clock < SIM_TIME){
        /*generate_query();*/
        receive_ir();
    }
    
}

void generate_query() {
    create("query");
    while(clock < SIM_TIME){
        /* code */
    }
    
}

void receive_ir() {
    create("receive_ir");
    long i;
    while(clock < SIM_TIME){
        /*receive(node, &msg);*/
        /*printf("message received %ld", node[0].mbox);*/
    }
    
}