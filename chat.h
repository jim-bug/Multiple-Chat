/*
* Author: Ignazio Leonardo Calogero Sperandeo.
 * Data: 13/06/2024
 * Repo: https://github.com/jim-bug/Multiple-Chat
 * Project Name: Multiple-Chat
 * by jim_bug :)
*/

#ifndef CHAT_H
#define CHAT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <math.h>

#define MAX_LENGTH_MSG 1024
#define MAX_NAME_LENGTH 100
#define SERVER_PORT 4870
#define MAX_HOST 10



// client's side
typedef struct ClientInfo{
    int sockfd;
    char name[MAX_NAME_LENGTH];
    char message[MAX_LENGTH_MSG];
} ClientInfo;


// server's side
typedef struct ConnectedClientInfo{
    int sockfd;
    struct sockaddr_in address;
    char message[MAX_LENGTH_MSG];
    char name[MAX_NAME_LENGTH];
    char closing_message[MAX_LENGTH_MSG];
    pthread_t fd_sender;
} ConnectedClientInfo;


typedef struct Node {
    ConnectedClientInfo* client;
    struct Node* next;
} Node;


// I've choosen extern variables because I need to use them in two different codes.
extern WINDOW* input_window;
extern WINDOW* output_window;
extern WINDOW* write_window;
extern FILE* log_file;
extern int start_y;
extern int start_x;
extern pthread_t receive_thread;
extern pthread_t write_thread;
// extern ConnectedClientInfo* client_connected[MAX_HOST];
extern struct Node* head_connected_client;
extern pthread_mutex_t mutex_connected_client;
extern pthread_mutex_t mutex;
extern int connected_client;

void closing_sequence();                            // it closes ncurses window, file.
void help();                                        // it prints the man of Multiple-Chat.
void write_log(char[], int);                        // it prints logs in a log file
void create_window(WINDOW**, int, int, int, int);   // it creates a ncurses window
void* get_message_from_host(void*);                 // client's side function, it gets messages from his fd.
void* send_message_to_host(void*);                  // client's side function, it sends messages from his fd.
void* listen_threads(void*);                        // client's side function, it waits a closing signal from client messages
void check_special_key(void*);
void* client_thread(void*);                         // server's side function, it receives messages from the client and sends them to other connected clients, similar to the flooding protocol.
void* send_to_all(void*);                           // server's side function


// List function
void insert(struct Node**, ConnectedClientInfo*);
void delete_node(struct Node**, int);
void print_list(struct Node*);
void len(struct Node*);


#endif
