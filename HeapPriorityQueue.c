#include <stdio.h>
#include <stdlib.h>
#include "Node.h"

#include "HeapPriorityQueue.h"

#define MAX 256

struct HeapPriorityQueue {
    Node ** nodes;
    int size;
};

HeapPriorityQueue* createHeapPriorityQueue() {
    HeapPriorityQueue* heap = (HeapPriorityQueue*)malloc(sizeof(HeapPriorityQueue));
    heap->nodes = (Node**)malloc(MAX * sizeof(Node*));
    heap->size = 0;
    

    return heap;
}


void swap(Node **a, Node **b)
{
    Node *temp = *a;
    *a = *b;
    *b = temp;
}

Node* pop(HeapPriorityQueue* heap) {
    if (heap->size == 0) return NULL;
    
    Node* root = heap->nodes[0];
    heap->nodes[0] = heap->nodes[heap->size - 1];
    heap->size--;
    
    int index = 0; 
    while (1) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int smallest = index;

        if (left < heap->size && getFreq(heap->nodes[left]) < getFreq(heap->nodes[smallest])) {
            smallest = left;
        }
        if (right < heap->size && getFreq(heap->nodes[right]) < getFreq(heap->nodes[smallest])) {
            smallest = right;
        }
        
        
        if (smallest == index) break;

        
        swap(&heap->nodes[index], &heap->nodes[smallest]);
        index = smallest;
    }
    return root;
}

void insert(HeapPriorityQueue* heap, Node** value)
{
    heap->nodes[heap->size] = *value;
    int index = heap->size;
    heap->size++;

    while (index > 0 && getFreq(heap->nodes[(index - 1) / 2]) > getFreq(heap->nodes[index]))
    {
        swap(&heap->nodes[index], &heap->nodes[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
    return;
}

void addNodes(HeapPriorityQueue* heap){
    if (heap->size <= 1 ){
        printf("Tamaño del heap insuficiente\n");
        return;
    }
    Node* left = pop(heap);
    Node* right = pop(heap);
    Node* newNode = createNode('$');
    sumFreq(newNode, getFreq(left), getFreq(right));
    addLeftNode(newNode, left);
    addRightNode(newNode, right);
    insert(heap, &newNode);
    return;
}

void convertHuffman(HeapPriorityQueue* heap) {
    while (heap->size > 1) {
        addNodes(heap);
    }
    return;
}

void display(HeapPriorityQueue* heap)
{
    for (int i = 0; i < heap->size; i++)
    printf("%c : %.10f\n", getCharacter(heap->nodes[i]), getFreq(heap->nodes[i]));
    printf("\n");
}

Node** getNodes(HeapPriorityQueue* heap) {
    return heap->nodes;
}

Node* getRoot(HeapPriorityQueue* heap) {
    if (heap->size == 0) return NULL;
    return heap->nodes[0];
}

/*
int main()
{
    Node * node1 = createNode('a');
    Node * node2 = createNode('b');
    Node * node3 = createNode('c');
    addRep(node1, 100);
    addRep(node2, 150);
    addRep(node3, 200);

    insert(&node2);
    insert(&node1);
    insert(&node3);

    display();
    
    convertHuffman();

    display();

    
    addNodes();

    display();
    
    return 0;
    
}
*/