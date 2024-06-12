#ifndef LISTA_H
#define LISTA_H

#include "chat.h"


typedef struct Node {
    Client_info* client;
    struct Node* next;
} Node;


#endif