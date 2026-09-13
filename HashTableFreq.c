#include "HashTableFreq.h"
#include "Node.h"
#include <stdio.h>
#include <stdlib.h>
#define ARRAY_SIZE 256

struct HashTableFreq {
    Node ** nodes;
    int total;
};

//Aux
int hashFunction(char c) {
    unsigned char caracter = (unsigned char) c;
    return caracter % ARRAY_SIZE;
}

HashTableFreq * createHashTableFreq() {
    HashTableFreq * hashTableFreq = (HashTableFreq*) malloc(sizeof(HashTableFreq));
    if(hashTableFreq == NULL) return NULL;
    hashTableFreq->nodes = (Node**) malloc(ARRAY_SIZE*sizeof(Node*));
    
    if (hashTableFreq->nodes  == NULL) {
        return NULL; 
    }

    for (int i = 0; i < ARRAY_SIZE; i++) {
        hashTableFreq->nodes[i] = NULL; 
    }

    hashTableFreq->total = 0;

    return hashTableFreq;
}

void destroyHashTableFreq(HashTableFreq * self) {
    if(self != NULL) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            if(self->nodes[i] != NULL) {
                destroyNode(self->nodes[i]);
            } 
        }
        free(self);
    }
    
}


void addHashTableFreqNode(HashTableFreq * self, Node * node, int total){
    if(self != NULL) {
        int index = hashFunction(getCharacter(node));
        if(self->nodes[index] == NULL) self->nodes[index] = node;
        else addRep(self->nodes[index], total);
        
    }
}


//getters
/**
 * This function return the whole array
 */
Node** getHashTableFreq(HashTableFreq * self) {
    if(self != NULL) {
        return self->nodes;
    }

    return NULL;
}

Node * getHashTableFreqNode(HashTableFreq * self,  Node * node){
    if(self != NULL) {
        int index = hashFunction(getCharacter(node));
        return self->nodes[index];
    }
    return NULL;
}

void printHashTableFreq(HashTableFreq * self) {
    if(self != NULL) {
        for(int i = 0; i < ARRAY_SIZE; i++) {
            if(self->nodes[i] != NULL) {
                printf("%c ", getCharacter(self->nodes[i]));
            }
        }
        printf("\n");
    }
}

