/*
* Author: Ignazio Leonardo Calogero Sperandeo.
 * Data: 13/06/2024
 * Repo: https://github.com/jim-bug/Multiple-Chat
 * Project Name: Multiple-Chat
 * by jim_bug :)
*/


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
#include "chat.h"

WINDOW* input_window;
WINDOW* output_window;
WINDOW* write_window;
FILE* log_file;
int start_x;
int start_y;
pthread_t receive_thread;
pthread_t write_thread;
struct Node* head_connected_client = NULL;


int main(int argc, char* argv[]) {
    char message_connection_log[MAX_LENGTH_MSG];
    log_file = fopen("log.txt", "w");

    if (argc < 2) {
        help();
    }
    else if(strcmp(argv[1], "-s") != 0 && strcmp(argv[1], "-c") != 0){     // The only valid options are: -s -c
        help();
    }
    else if (strcmp(argv[1], "-s") == 0) {                                 // server's side
        int server_sock;
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len = sizeof(client_addr);

        server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock < 0) {
            write_log("Creating server's side socket -> ERROR", 1);
        }
        write_log("Creating server's side socket -> ERROR", 0);

        // Initializing the server struct
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(SERVER_PORT);

        if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            write_log("Binding of server's side socket -> ERROR", 1);
        }
        write_log("Binding of server's side socket -> OK", 0);

        if (listen(server_sock, 1) < 0) {
            write_log("Listen of server's side socket -> ERROR", 1);
        }
        write_log("Listen of server's side socket -> OK", 0);
        while (1){
            ConnectedClientInfo* temp_client = (ConnectedClientInfo*)malloc(sizeof(ConnectedClientInfo));
            temp_client->sockfd = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);

            if (temp_client->sockfd < 0) {
                snprintf(message_connection_log, MAX_LENGTH_MSG, "The connection doesn't happen from: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                write_log(message_connection_log, 1);
            }
            snprintf(message_connection_log, MAX_LENGTH_MSG, "The connection happens from: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            write_log(message_connection_log, 0);

            temp_client->address = client_addr;
            pthread_create(&temp_client->fd_sender, NULL, client_thread, (void*)temp_client);

            pthread_mutex_lock(&mutex);
                insert(&head_connected_client, temp_client);
            pthread_mutex_unlock(&mutex);
        }
        close(server_sock);
    }

    else if (strcmp(argv[1], "-c") == 0 && atoi(argv[3]) >= 1024 && atoi(argv[3]) <= 49151) {         // client's side
		initscr();                                                         // creating the main ncurses window
        getmaxyx(stdscr, start_y, start_x);
        create_window(&input_window, start_y-4, start_x/2, 0, 0);           // The input window(51 x 101) start to row: 0 and column: 0
        create_window(&output_window, start_y-4, start_x/2, 0, start_x/2);  // The output window(51 x 101) start to row: 0 and column: 101
        create_window(&write_window, 4, start_x, start_y-4, 0);             // The write window(4 x 101) start to row: 51 and column: 0


        int client_sock;
        unsigned short port = (unsigned short) atoi(argv[3]);
        char name_client[MAX_NAME_LENGTH];
        struct sockaddr_in server_addr;
        struct hostent* hp;
        ConnectedClientInfo* client = (ConnectedClientInfo*)malloc(sizeof(ConnectedClientInfo));


        if(strlen(argv[4]) > 100){
            write_log("Choosen name -> ERROR, check the size(0 < size < 100)", 1);
        }
        write_log("Choosen name -> OK", 0);
        strncpy(client->name, argv[4], sizeof(client->name));


        client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock < 0) {
            write_log("Creating client socket -> ERROR", 1);
        }
        write_log("Creating client socket -> OK", 0);

        client->sockfd = client_sock;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;

        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, argv[2], &server_addr.sin_addr);


        if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            snprintf(message_connection_log, MAX_LENGTH_MSG, "The connection doesn't happen to: %s:%s", argv[2], argv[3]);
            write_log(message_connection_log, 1);
        }
        snprintf(message_connection_log, MAX_LENGTH_MSG, "The connection happens to: %s:%s", argv[2], argv[3]);
        write_log(message_connection_log, 0);


        pthread_create(&receive_thread, NULL, get_message_from_host, (void*)client);
        pthread_create(&write_thread, NULL, send_message_to_host, (void*)client);
        pthread_join(receive_thread, NULL);
		pthread_join(write_thread, NULL);
        close(client_sock);
    }
    else{
        help();
    }
    
    return 0;
}

// :)