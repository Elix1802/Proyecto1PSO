#include "HashTableFreq.h"
#include "Node.h"
#include <stdio.h>
#include <stdlib.h>
#define ARRAY_SIZE 256

struct HashTableFreq {
    Node ** nodes;

    int totalChar;
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
    hashTableFreq->totalChar = 0;

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


void addHashTableFreqNode(HashTableFreq * self, Node * node){
    if(self != NULL) {
        int index = hashFunction(getCharacter(node));
        if(self->nodes[index] == NULL) {
            self->nodes[index] = node;
            addRep(node);
            self->total++;
        } else {
            addRep(self->nodes[index]);
            destroyNode(node);
        };
        
    }
}

void updateHashTableFreqNode(HashTableFreq * self, int total) {
    self->totalChar = total;
    if(self != NULL) {
        for(int i = 0; i < ARRAY_SIZE; i++) {
            if(self->nodes[i] != NULL) {
                finalFreq(self->nodes[i], total);
            }
        }
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

//Función que devuelve la frecuencia de un nodo segun su id en el arreglo
//Si la posición está vacía devuelve -1;

int getFrequencyById(HashTableFreq * self, int index){
    if(self != NULL) {
        if(self->nodes[index] != NULL) return getFreq(self->nodes[index]);
        else return -1;

    }
    return -1;
}

//Devuelve la frecuencia de un nodo.
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
                printf("%c : %.10f\n", getCharacter(self->nodes[i]), getFreq(self->nodes[i]));
            }
        }
        printf("\n");
    }
}

int totalNodesHashTable(HashTableFreq * self) {
    if(self!=NULL) {
        return self->total;
    }

    return -1;
}

int getTotalCharsCounted(HashTableFreq * self) {
    if(self!=NULL) {
        return self->totalChar;
    }

    return -1;
}
