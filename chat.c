/*
 * Autori: Ignazio Leonardo Calogero Sperandeo.
 * Data: 13/06/2024
 * Consegna: Realizzare una chat in C che presenti una CLI. La chat deve permettere il dialogo tra due terminali nella stessa LAN e in LAN diverse.
 * Link al repo: https://github.com/jim-bug/Multiple-Chat
 * Riferimenti alla parte dell'ingegnieria del software: 
 * Nome progetto: Multiple-Chat
*/

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "chat.h"



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_rows = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_ncurses = PTHREAD_MUTEX_INITIALIZER;
int window_rows_sharing = 1;
char flag_state_close = 'n';
char flag_start_get_message = 'n';


void closing_sequence(){
        delwin(input_window);
        delwin(output_window);
        delwin(write_window);
        endwin();
        fclose(log_file);
}

void print_stack_trace(){
        closing_sequence();
        printf("\tHelp good-chat!\nUsage: -s | -c ip port\n\t1) -s: Opzione server\n\t2) -c: Opzione client, speficare anche l'ip e il numero di porta(compreso tra 1024 e 49515) del server\n");
        exit(EXIT_FAILURE);
}

// FUNZIONE LATO CLIENT
void write_log(char log_message[], int state_error){
        if(state_error == 1){       // caso di errore.
            fprintf(log_file, "%s\n", log_message);
            closing_sequence();
            exit(EXIT_FAILURE);
        }
        fprintf(log_file, "%s\n", log_message);
}

// Funzione che crea una finestra con una box, FUNZIONE LATO CLIENT
void create_window(WINDOW** new_win, int row, int col, int begin_y, int begin_x){
	    *new_win = newwin(row, col, begin_y, begin_x);
        refresh();
        // box(*new_win, '|', '|');     // piccolo debug per le finestre
        wrefresh(*new_win);
}

// FUNZIONE LATO CLIENT
void* get_message_from_host(void* arg) {        // funzione che riceve qualcosa dal server/client
    int sockfd = *((int*)arg);
    char buf[MAX_LENGTH_MSG]; // Buffer per i dati
    ssize_t bytes_read = 1;
    do {
        if(flag_start_get_message != 'n'){
            bytes_read = recv(sockfd, buf, sizeof(buf), 0);
            buf[bytes_read] = '\0';
            
            pthread_mutex_lock(&mutex_rows);
                window_rows_sharing++;
            pthread_mutex_unlock(&mutex_rows);
            

            mvwprintw(output_window, window_rows_sharing, 1, "%s", buf);
            wrefresh(output_window);
        }

    } while(bytes_read > 0);

    return NULL;
}

// FUNZIONE LATO CLIENT
void* send_message_to_host(void* arg) {     // funzione che invia qualcosa al server/client
    int sockfd = *((int*)arg);
    char buf[MAX_LENGTH_MSG];
    char name[MAX_LENGTH_MSG];
    ssize_t bytes_written;
    ssize_t bytes_read = recv(sockfd, name, sizeof(name), 0);
    name[bytes_read] = '\0';
    if(bytes_read > 0){
        strcat(name, "> ");
        flag_start_get_message = 'y';
        // mvwprintw(write_window, 1, 1, "NOME ASSEGNATO MA FA BENE");
    }
    else{
        // mvwprintw(write_window, 5, 1, "NOME NON ASSEGNATO SCHIFO");
        flag_state_close = 'y';
        // closing_sequence();
        // exit(EXIT_FAILURE);
    }
    do {
        mvwprintw(write_window, 1, 1, name);      // chiedo all'utente cosa vuole mandare all'altro host su una terza finestra.
        mvwgetstr(write_window, 1, 4, buf);
        mvwprintw(input_window, window_rows_sharing, 1, "%s %s", name, buf);       // mando a video sulla finestra di input ciò che ho inviato 
        pthread_mutex_lock(&mutex_rows);
            window_rows_sharing++;
        pthread_mutex_unlock(&mutex_rows);

        bytes_written = write(sockfd, buf, strlen(buf)+1);
        wrefresh(input_window);
        wrefresh(write_window);
        wclear(write_window);

    } while(strcmp(buf, "/exit") != 0 || bytes_written <= 0);
    flag_state_close = 'y';
    return NULL;
}

// FUNZIONE LATO SERVER
void* client_thread(void* arg){
    Client_info* client = (Client_info*)arg;
    char str[2] = {client->name_client, '\0'};
    write(client->sockfd, str, strlen(str)+1);
    // printf("%c è entrato nel suo thread\n", client->name_client); debug :)
    do{
        size_t bytes_written = recv(client->sockfd, client->message, sizeof(client->message), 0);
        client->message[bytes_written] = '\0';
        if(bytes_written > 0){
            char temp_message[2000];
            snprintf(temp_message, sizeof(temp_message), "%c> %s", client->name_client, client->message);
            strncpy(client->message, temp_message, sizeof(client->message));

            // printf("%c sta provando a mandare un messaggio\n", client->name_client); debug :)
            // printf("%c prova a mandare: %s\n", client->name_client, client->message); debug :)
            pthread_create(&client->fd_sender, NULL, send_to_all, (void*)arg);
            pthread_detach(client->fd_sender);
        }
    } while(strcmp(client->message, "/exit") != 0);


    // da confermare, in fase di sviluppo la parte dedicaata alla chiusura da parte di un client.
    /*
    pthread_mutex_lock(&mutex);
        connected_host --;
        close(client->sockfd);
        remove_fd(client->sockfd);
    pthread_mutex_unlock(&mutex);
    */
    return NULL;

}

// FUNZIONE LATO SERVER
void* send_to_all(void* arg){
    Client_info* client = (Client_info*)arg;
    for(int i = 0; i < MAX_HOST; i++){
        pthread_mutex_lock(&mutex);
        if(client_connected[i] == NULL){
            pthread_mutex_unlock(&mutex);
            continue;
        }
        if(client_connected[i]->sockfd != 0 && client_connected[i]->sockfd != client->sockfd){
            write(client_connected[i]->sockfd, client->message, strlen(client->message)+1);
            // printf("%c sta inviando..... %s fd: %d vs clientsock: %d\n", client->name_client, client->message, fd_array[i], client->sockfd); debug :)
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// FUNZIONE LATO SERVER
void remove_fd(int fd){
    for(int i = 0; i < MAX_HOST; i++){
        if(client_connected[i]->sockfd == fd){
            client_connected[i] = NULL;
        }
    }
}


/*
FUNZIONE LATO CLIENT
Funzione che eseguirà un terzo thread, starà sempre in attesa fin quando il valore della variabile flag_state_close non sara pari a y. In quel caso
significa che uno dei due host vuole chiudere la connessione e perciò cancello i thread di ricezione e scrittura e chiamo la sequenza di chiusura.
*/
void* listen_threads(void* arg){
    while(flag_state_close != 'y');
    pthread_cancel(receive_thread);
    pthread_cancel(write_thread);
    closing_sequence();
    exit(EXIT_SUCCESS);
}
