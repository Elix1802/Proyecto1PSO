#ifndef HASHTABLEFREQ_H
#define HASHTABLEFREQ_H
#include "Node.h"

typedef struct HashTableFreq HashTableFreq;

HashTableFreq * createHashTableFreq();
void destroyHashTableFreq(HashTableFreq * self);

//Setters
void addHashTableFreqNode(HashTableFreq * self, Node * node);
void updateHashTableFreqNode(HashTableFreq * self, int total);

//getters
Node ** getHashTableFreq(HashTableFreq * self);
Node * getHashTableFreqNode(HashTableFreq * self,  Node * node);
void printHashTableFreq(HashTableFreq * self);

int getTotalCharsCounted(HashTableFreq * self);
int getFrequencyById(HashTableFreq * self, int index);
char getCharById(HashTableFreq * self, int index);

#endif