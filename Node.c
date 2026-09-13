#include "Node.h"
#include <stdio.h>
#include <stdlib.h>

struct Node {
    char character;
    float freq;
    int rep;
    int isFather;
    struct Node* leftNode;
    struct Node* rightNode;
};

struct Node * createNode(char name) {
    Node * node = (Node*) malloc(sizeof(Node));
    if(node == NULL) return NULL;
    node->character = name;
    node->rep = 0;
    node->freq = 0.0;
    node->isFather = 0;
    node->leftNode = NULL;
    node->rightNode = NULL;

    return node;
}

void destroyNode(Node * self) {
    if(self != NULL) {
        free(self);
    }
}

//Setters
void addLeftNode(Node * self, Node * leftNode){
    if(self != NULL) {
        self->isFather = 1;
        self->leftNode = leftNode;
    }

}

void addRightNode(Node * self, Node * rightNode) {
    if(self != NULL) {
        self->isFather = 1;
        self->rightNode = rightNode;
    }
}

void addRep(Node * self, int total) {
    if(self != NULL) {
        self->rep++;
        self->freq = (float)self->rep/total;
    }
}

void sumFreq(Node * self, float freqOne, float freqTwo) {
    self->isFather = 1;
    if(self != NULL) {
        self->freq = freqOne + freqTwo;
    }
}
//Getters
char getCharacter(Node* self) {
    if(self != NULL) {
        return self->character;
    }
    return ' ';
}
Node * getLeftNode(Node * self) {
    if(self != NULL) {
        return self->leftNode;
    }
    return NULL;
}
Node * getRightNode(Node * self) {
    if(self != NULL) {
        return self->rightNode;
    }
    return NULL;
}

int isLeaf(Node * self) {
    if(self!=NULL) {
        if(self->leftNode == NULL && self->rightNode == NULL) {
            return 1;
        }
        return 0;
    }
    return -1;
}

float getFreq(Node * self) {
    if(self != NULL) {
        return self->freq;
    }
    return 0.0;
}