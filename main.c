/* Project #3: Cache or Not to Cache by Kongmanee, Jaturong */
#include <csim.h>
#include <stdio.h>

/*
1. tydedef
2. struct
3. sprintf
*/

/* #define is used to define CONSTANT */
#define SIM_TIME 20.0
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
    long id;
    /* represented by the number of update */
    long last_updated_time;

};

struct message data_list[100];


struct message *msg;
struct message *m;


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
    
    for (i = 0; i < 100; i++) {
         data_list[i].id = i;
         data_list[i].last_updated_time = 0;
    }

    long all_node = NUM_CLIENTS + NUM_CLIENTS;
    /*max_mailboxes(all_node);*/
    /*max_servers(all_node);
    max_events(all_node);*/

    for (i = 0; i < all_node; i++) {
        sprintf(str, "input.%d", i);
        node[i].mbox = mailbox(str);
    }

    /*create_mailbox_set("mbox set", mbox_arr, NUM_BOXES);*/

    server(0);
    printf("Calling server\n");

    for (i = 0; i < NUM_CLIENTS; i++) {
        client(i);
        printf("Calling client %ld\n", i);
    }
    
}

void server(long n) {
    create("server");
    printf("Server is generated\n");
    invalidation_report();
    while(clock < SIM_TIME){
        hold(5);
        printf("Calling IR\n");
        
        
        /*update_data_items();
        receive_message();*/
    }   
}

void invalidation_report() {
    create("ir");
    printf("IR is generated\n");
    while(clock < SIM_TIME){
        hold(5);
        /* Broadcast: put the message into all clients' mailboxes */
        /*struct message *msg;*/
        msg = (struct message*)malloc(sizeof(*msg));

        long j;
        for (j = 0; j < 100; j++) {
            data_list[j].last_updated_time = 1;
        }
            
        /*send(node[0].mbox, (long)msg);
        printf("sent message to node %ld\n", 0);
        send(node[1].mbox, (long)msg);
        printf("sent message to node %ld\n", 1);*/
        /*struct message *m;
        m = &msg;*/
        /*receive(node[0].mbox, (long*)&m);
        printf("message received %ld \n", node[0].mbox);
        receive(node[1].mbox, (long*)&m);
        printf("message received %ld \n", node[1].mbox);*/
        long i;
        for (i = 0; i < NUM_CLIENTS; i++) {
            /* Transmission delay */
            /* hold(2); */
            /*msg->id = i+1000;*/
            send(node[i].mbox, (long)data_list);
            printf("sent message to node %ld with %ld\n", i, data_list);
            /*receive(node[i].mbox, (long*)&m);
            printf("message received %ld \n", m->id);*/
        }
    }
    
}

void update_data_items() {
    create("update");
    while(clock < SIM_TIME){
        /* code */
        hold(1);
    }
    
}

void receive_message() {
    create("receive_message");
    while(clock < SIM_TIME){
        /**/
        hold(1);
    }
}








void client(long n) {
    create("client");
    printf("Client %ld is generatedddd\n", n);
    receive_ir(n);
    while(clock < SIM_TIME){
        hold(1);
        /*generate_query();*/
        
    }
    
}

void generate_query() {
    create("query");
    while(clock < SIM_TIME){
        /* code */
        hold(1);
    }
    
}

void receive_ir(long n) {
    create("receive_ir");
    struct message *m;
    m = &msg;
    while(clock < SIM_TIME){
        hold(1);
        receive(node[n].mbox, (long*)&m);
        printf("Node %ld received %ld message\n", n, m->id);
    }
    
}