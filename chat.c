/*
* Author: Ignazio Leonardo Calogero Sperandeo.
 * Data: 13/06/2024
 * Repo: https://github.com/jim-bug/Multiple-Chat
 * Project Name: Multiple-Chat
 * by jim_bug :)
*/
#include "chat.h"



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_rows = PTHREAD_MUTEX_INITIALIZER;
int window_rows_sharing = 1;


void closing_sequence(){
		delwin(input_window);
		delwin(output_window);
		delwin(write_window);
        endwin();
        fclose(log_file);
}


void help(){
        printf("\tHelp good-chat!\nUsage: -s | -c ip port\n\t1) -s: Opzione server\n\t2) -c: Opzione client, speficare anche l'ip e il numero di porta(compreso tra 1024 e 49515) del server\n");
        exit(EXIT_FAILURE);
}


// CLIENT'S SIDE
void write_log(char log_message[], int state_error){
        if(state_error == 1){       // caso di errore.
            fprintf(log_file, "%s\n", log_message);
            closing_sequence();
            exit(EXIT_FAILURE);
        }
        fprintf(log_file, "%s\n", log_message);
}


void create_window(WINDOW** new_win, int row, int col, int begin_y, int begin_x){
	    *new_win = newwin(row, col, begin_y, begin_x);
        refresh();
        // box(*new_win, '|', '|');     // small debug for windows :)
        wrefresh(*new_win);
}


// CLIENT'S SIDE
void* get_message_from_host(void* arg) {
    ConnectedClientInfo* client = (ConnectedClientInfo*)arg;
    char buf[MAX_LENGTH_MSG]; 
    ssize_t bytes_read = 1;
    do {
        bytes_read = recv(client->sockfd, buf, sizeof(buf), 0);
        buf[bytes_read] = '\0';
    	mvwprintw(output_window, window_rows_sharing, 1, "%s", buf);

		if(window_rows_sharing >= (start_y-4)){
        	wclear(output_window);
			wclear(input_window);
			wrefresh(output_window);
			wrefresh(input_window);
			pthread_mutex_lock(&mutex_rows);
				window_rows_sharing = 1;
			pthread_mutex_unlock(&mutex_rows);
		}
		else if(strlen(buf) > (start_x/2)){			// see TOO_SIZE_PROBLEM.txt to review the problem and the solution.
			pthread_mutex_lock(&mutex_rows);
				window_rows_sharing += floor((double)strlen(buf)/(start_x/2))+1;
			pthread_mutex_unlock(&mutex_rows);
		}
		else{
			pthread_mutex_lock(&mutex_rows);
				window_rows_sharing ++;
		    pthread_mutex_unlock(&mutex_rows);
		}

        wrefresh(output_window);
    } while(bytes_read > 0);

    return NULL;
}


// CLIENT'S SIDE
void* send_message_to_host(void* arg) {
    ConnectedClientInfo* client = (ConnectedClientInfo*)arg;
    ssize_t bytes_written;
    write(client->sockfd, client->name, strlen(client->name)+1);
    do {
        mvwprintw(write_window, 1, 1, "%s> ", client->name);
        mvwgetstr(write_window, 1, strlen(client->name)+2, client->message);
    	bytes_written = write(client->sockfd, client->message, strlen(client->message)+1);
    	mvwprintw(input_window, window_rows_sharing, 1, "%s> %s", client->name, client->message);

        if(strcmp(client->message, "/cls") == 0){		// clear command
                wclear(output_window);
				wclear(input_window);
                wrefresh(output_window);
				wrefresh(input_window);
        }
		else if(window_rows_sharing >= (start_y-4)){
        	wclear(output_window);
			wclear(input_window);
			wrefresh(output_window);
			wrefresh(input_window);

			pthread_mutex_lock(&mutex_rows);
				window_rows_sharing = 1;
			pthread_mutex_unlock(&mutex_rows);
		}
		else if(strlen(client->message) > (start_x/2)){            // see TOO_SIZE_PROBLEM.txt to review the problem and the solution.
			pthread_mutex_lock(&mutex_rows);
				window_rows_sharing += floor((double)strlen(client->message)/(start_x/2))+1;
			pthread_mutex_unlock(&mutex_rows);
		}
		else{
			pthread_mutex_lock(&mutex_rows);
				window_rows_sharing ++;
			pthread_mutex_unlock(&mutex_rows);
		}

        wrefresh(input_window);
        wrefresh(write_window);
        wclear(write_window);

    } while(strcmp(client->message, "/exit") != 0);
	closing_sequence();
    exit(EXIT_SUCCESS);
    return NULL;
}


// SERVER'S SIDE
void* client_thread(void* arg){
    ConnectedClientInfo* client = (ConnectedClientInfo*)arg;
    pthread_t sender_thread;
    size_t bytes_written;
	bytes_written = recv(client->sockfd, client->name, sizeof(client->name), 0);		// I get the client name
	client->name[bytes_written] = '\0';
    printf("The connection happens from: %s:%d -- %s connected with server\n", inet_ntoa(client->address.sin_addr), ntohs(client->address.sin_port), client->name);
    // printf("Assegned Name: %s\n", client->name); // debug :)

    do{
        bytes_written = recv(client->sockfd, client->message, sizeof(client->message), 0);
        client->message[bytes_written] = '\0';
        if(bytes_written > 0){
            char temp_message[MAX_LENGTH_MSG*2];
            snprintf(temp_message, sizeof(temp_message), "%s> %s", client->name, client->message);
            strncpy(client->message, temp_message, sizeof(client->message));
            pthread_create(&sender_thread, NULL, send_to_all, arg);
        }
    } while(strcmp(client->message, "/exit") != 0);


    pthread_join(sender_thread, NULL);
    pthread_mutex_lock(&mutex);
        delete_node(&head_connected_client, client->sockfd);
        close(client->sockfd);
    pthread_mutex_unlock(&mutex);
    printf("The Client closed connection from: %s:%d -- %s came out of the chat\n", inet_ntoa(client->address.sin_addr), ntohs(client->address.sin_port), client->name);
    return NULL;
}


// SERVER'S SIDE
void* send_to_all(void* arg) {
    ConnectedClientInfo* client = (ConnectedClientInfo*)arg;
    struct Node* current = head_connected_client;
    while(current != NULL) {
        pthread_mutex_lock(&mutex);
        if(current->client->sockfd != 0 && current->client->sockfd != client->sockfd) {
            ssize_t result = write(current->client->sockfd, client->message, strlen(client->message) + 1);
        }
        current = current->next;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}



void insert(struct Node** head, ConnectedClientInfo* value) {
    struct Node* new_node = (struct Node*)malloc(sizeof(Node));
    new_node->client = value;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        return;
    }

    struct Node* temp_node = *head;
    while (temp_node->next != NULL) {
        temp_node = temp_node->next;
    }
    temp_node->next = new_node;
}


void delete_node(struct Node** head, int sockfd_key) {
    struct Node *tmp = *head;
    struct Node *prev = NULL;

    if (tmp != NULL && tmp->client->sockfd == sockfd_key) {
        *head = tmp->next;
        free(tmp);
        return;
    }

    while (tmp != NULL && tmp->client->sockfd != sockfd_key) {
        prev = tmp;
        tmp = tmp->next;
    }

    if (tmp == NULL)
        return;

    prev->next = tmp->next;
    free(tmp);
}


void print_list(struct Node* head){
    while(head != NULL){
        printf("Nodo -> FD: %d\n", head->client->sockfd);
        head = head->next;
    }
}

// :)