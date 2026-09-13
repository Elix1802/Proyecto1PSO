#ifndef NODE_H
#define NODE_H

typedef struct Node Node;

Node* createNode(char name);
void destroyNode(Node* self);

// Setters
void addLeftNode(Node* self, Node* node);
void addRightNode(Node* self, Node* node);
void addRep(Node* self);
void sumFreq(Node* self, float freqOne, float freqTwo);
void finalFreq(Node * self, int total);

// Getters
Node* getLeftNode(Node* self);
char getCharacter(Node* self);
Node* getRightNode(Node* self);
int isLeaf(Node * self);

float getFreq(Node* self);

#endif