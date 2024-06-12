/*
 * Autori: Ignazio Leonardo Calogero Sperandeo.
 * Data: 13/06/2024
 * Consegna: Realizzare una chat in C che presenti una CLI. La chat deve permettere il dialogo tra due terminali nella stessa LAN e in LAN diverse.
 * Link al repo: https://github.com/jim-bug/Multiple-Chat
 * Riferimenti alla parte dell'ingegnieria del software: 
 * Nome progetto: Multiple-Chat
*/


#include <netinet/in.h>
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
#include "chat.h"


WINDOW* input_window;
WINDOW* output_window;
WINDOW* write_window;
FILE* log_file;
int start_x;
int start_y;
pthread_t receive_thread;
pthread_t write_thread;
int fd_array[MAX_HOST];
int connected_host = 0;

int main(int argc, char* argv[]) {
    pthread_t listener;
    char message_connection_log[MAX_LENGTH_MSG];
    log_file = fopen("log.txt", "w");



    if (argc < 2) {
        // se non inserisco alcuna opzione.
        print_stack_trace();
    }
    else if(strcmp(argv[1], "-s") != 0 && strcmp(argv[1], "-c") != 0){
        // se non inserisco un opzione prevista.
        print_stack_trace();
    }
    else if (strcmp(argv[1], "-s") == 0) { // caso server
        int server_sock;
        pthread_t threads[MAX_HOST];
        int i = 0;
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len = sizeof(client_addr);
        

        // Creazione del socket del server
        server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock < 0) {
            write_log("Creazione mezzo socket del server -> ERRORE", 1);
        }
        write_log("Creazione mezzo socket del server -> OK", 0);


        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(SERVER_PORT);

        // Binding del mezzo socket del server all'indirizzo locale
        if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            write_log("Binding del mezzo socket del server -> ERRORE", 1);
        }
        write_log("Binding del mezzo socket del server -> OK", 0);
        
        // Metto il mezzo socket del server in ascolto di connessioni, con una coda massima di 1 persona.
        if (listen(server_sock, 1) < 0) {
            write_log("Listen del mezzo socket del server -> ERRORE", 1);
        }
        write_log("Listen del mezzo socket del server -> OK", 0);

        // Metto il mezzo socket del server in grado di accettare connessioni
        while (connected_host < MAX_HOST) {
            fd_array[connected_host] = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
            if (fd_array[connected_host] < 0) {
                snprintf(message_connection_log, MAX_LENGTH_MSG, "Connessione non avvenuta da parte della destinazione: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                write_log(message_connection_log, 1);
            }
            Client_info* client = (Client_info*)malloc(sizeof(Client_info));

            client->sockfd = fd_array[connected_host];
            client->name_client = 'A' + connected_host;
            snprintf(message_connection_log, MAX_LENGTH_MSG, "Connessione avvenuta da parte della destinazione: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));      // funzioni usate per mandare a video porta e ip del client.
            printf("%s\nClient %c connesso\nClient %c fd: %d\n", message_connection_log, client->name_client, client->sockfd);
            // write_log(message_connection_log, 0);
            pthread_create(&threads[i], NULL, client_thread, (void*)client);
            connected_host ++;
        }
        for(int i = 0; i < MAX_HOST; i++){
            pthread_join(threads[i], NULL);
        }
        close(server_sock);
    }

    else if (strcmp(argv[1], "-c") == 0 && atoi(argv[3]) >= 1024 && atoi(argv[3]) <= 49151) { // caso client, con controllo sul numero di porta scelto.
        initscr(); // Inizializza la finestra ncurses principale
        getmaxyx(stdscr, start_y, start_x); // Ottengo le dimensioni dello schermo
        create_window(&input_window, start_y-4, start_x, 0, 0);   // 51 x 202 parte da riga:0 e colonna: 0
        create_window(&write_window, 4, start_x, start_y-4, 0);     // 4 x 101 parte da riga: 51 e colonna: 0

        
        int client_sock;
        unsigned short port = (unsigned short) atoi(argv[3]);
        struct sockaddr_in server_addr;
        struct hostent* hp;
        

        // Creazione del socket del client
        client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock < 0) {
            write_log("Creazione del mezzo socket del client -> ERRORE", 1);
        }
        write_log("Creazione del mezzo socket del client -> OK", 0);
        
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;

        server_addr.sin_port = htons(port);        // converto in intero la stringa che indica il numero di porta
        inet_pton(AF_INET, argv[2], &server_addr.sin_addr);	// assegno l'ip del server alla quale il client si dovrà connettere.


        if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            snprintf(message_connection_log, MAX_LENGTH_MSG, "Connessione non avvenuta alla destinazione -> %s:%s", argv[2], argv[3]);
            write_log(message_connection_log, 1);
        }
        snprintf(message_connection_log, MAX_LENGTH_MSG, "Connessione avvenuta alla destinazione -> %s:%s", argv[2], argv[3]);
        write_log(message_connection_log, 0);

        // Creazione dei thread per la ricezione e l'invio dei messaggi
        pthread_create(&receive_thread, NULL, get_message_from_host, &client_sock);
        pthread_create(&write_thread, NULL, send_message_to_host, &client_sock);
        pthread_create(&listener, NULL, listen_threads, NULL);
        pthread_join(receive_thread, NULL);
        pthread_join(write_thread, NULL);
        pthread_join(listener, NULL);
        close(client_sock);
    }
    else{
        print_stack_trace();
    }
    
    return 0;
}
