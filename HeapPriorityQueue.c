#include <stdio.h>
#include <stdlib.h>
#include "Node.h"

#include "HeapPriorityQueue.h"

#define MAX 256


int size = 0;

Node** createHeapPriorityQueue() {
    Node** heap = malloc(MAX * sizeof(Node*));

    return heap;
}


void swap(Node **a, Node **b)
{
    Node *temp = *a;
    *a = *b;
    *b = temp;
}

Node* pop(Node** heap) {
    if (size == 0) return NULL;
    
    Node* root = heap[0];
    heap[0] = heap[size - 1];
    size--;
    
    int index = 0; 
    while (1) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int smallest = index;

        if (left < size && getFreq(heap[left]) < getFreq(heap[smallest])) {
            smallest = left;
        }
        if (right < size && getFreq(heap[right]) < getFreq(heap[smallest])) {
            smallest = right;
        }
        
        
        if (smallest == index) break;

        
        swap(&heap[index], &heap[smallest]);
        index = smallest;
    }
    return root;
}

void insert(Node** heap, Node** value)
{
    heap[size] = *value;
    int index = size;
    size++;

    while (index > 0 && getFreq(heap[(index - 1) / 2]) > getFreq(heap[index]))
    {
        swap(&heap[index], &heap[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
    return;
}

void addNodes(Node** heap){
    if (size <= 1 ){
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

void convertHuffman(Node** heap) {
    while (size > 1) {
        addNodes(heap);
    }
    return;
}

void display(Node** heap)
{
    for (int i = 0; i < size; i++)
    printf("%c : %f \n", getCharacter(heap[i]), getFreq(heap[i]));
    printf("\n");
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