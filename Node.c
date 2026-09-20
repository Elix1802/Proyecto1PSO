#include "Node.h"
#include <stdio.h>
#include <stdlib.h>

struct Node {
    char character;
    double freq;
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

struct Node * createNodeFreq(char name, double freq) {
    Node * node = (Node*) malloc(sizeof(Node));
    if(node == NULL) return NULL;
    node->character = name;
    node->rep = 0;
    node->freq = freq;
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

void destroyTree(Node * self) {
    if(self != NULL) {
        destroyTree(self->leftNode);
        destroyTree(self->rightNode);
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

void addRep(Node * self) {
    if(self != NULL) {
        self->rep++;
    }
}

void finalFreq(Node * self, int total) {
    if(self != NULL) {
        self->freq = (double)self->rep / total;
    }
}



void sumFreq(Node * self,  double freqOne,  double freqTwo) {
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

double getFreq(Node * self) {
    if(self != NULL) {
        return self->freq;
    }
    return 0.0;
}