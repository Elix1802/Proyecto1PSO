#ifndef HEAP_H
#define HEAP_H

typedef struct Node Node;

void swap(Node **a, Node **b);
void addNodes();
Node* pop(Node** heap);
void display(Node** heap);
void insert(Node** heap, Node** value);
void convertHuffman();
Node** createHeapPriorityQueue();

#endif