/* Project #3: Cache or Not to Cache by Kongmanee, Jaturong */
#include <csim.h>
#include <stdio.h>

/*
1. tydedef
2. struct
3. sprintf
*/

/* #define is used to define CONSTANT */
#define SIM_TIME 50.0
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

struct message data_list[10];
struct message *ptr[10];


/*struct message *msg;
struct message *m;*/


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
    
    for (i = 0; i < 100; i++) {
        data_list[i].id = i;
        data_list[i].last_updated_time = 0;
        ptr[i] = &data_list[i];
        /*printf("Test pointer %ld\n", ptr[i]->id);*/
    }

    long all_node = NUM_CLIENTS + NUM_CLIENTS;

    for (i = 0; i < all_node; i++) {
        sprintf(str, "input.%d", i);
        node[i].mbox = mailbox(str);
    }

    printf("modify array\n");
    data_list[0].id = 555;

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
        hold(5);    
        /* Broadcast: put the message into all clients' mailboxes */
        /*struct message *msg;*/
        long i;
        /*for (i = 0; i < 100; i++) {
            data_list[i].last_updated_time = 88888;
        }*/

        /*printf("pppp\n");
        printf(ptr[0]->id);
        printf("uuuu\n");*/
        

        
            
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
        for (i = 0; i < NUM_CLIENTS; i++) {
            printf("sending %ld \n", data_list[i].id);
            send(node[i].mbox, data_list);
            printf("sent message to node %ld with %ld\n", i, data_list[i].id);
        }
    }
    
}

void update_data_items() {
    create("update");
    printf("Updating data items\n");
    while(clock < SIM_TIME){
        /* code */
        hold(10);
        long i;
        for (i = 0; i < 100; i++) {
            data_list[i].last_updated_time = 88888;
        }
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
    printf("Client %ld is generated\n", n);
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
    printf("receiving messages\n");
    while(clock < SIM_TIME){
        hold(5);
        receive(node[n].mbox, ptr);
        long i;
        /*for(i = 0; i < 100; i++)
        {
            printf("Node %ld received message\n", ptr);
        }*/
        
        
    }
    
}