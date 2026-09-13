#ifndef HEAP_H
#define HEAP_H

typedef struct HeapPriorityQueue HeapPriorityQueue;

void swap(Node **a, Node **b);
void addNodes(HeapPriorityQueue* heap);
Node* pop(HeapPriorityQueue* heap);
void display(HeapPriorityQueue* heap);
void insert(HeapPriorityQueue* heap, Node** value);
void convertHuffman();
HeapPriorityQueue* createHeapPriorityQueue();
Node** getNodes(HeapPriorityQueue* heap);

#endif