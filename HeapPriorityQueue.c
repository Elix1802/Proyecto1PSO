#include <stdio.h>
#include "Node.h"

#include "HeapPriorityQueue.h"

#define MAX 256

Node* heap[MAX];
int size = 0;


void swap(Node **a, Node **b)
{
    Node *temp = *a;
    *a = *b;
    *b = temp;
}

Node* pop() {
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

void insert(Node** value)
{
    heap[size] = *value;
    int index = size;
    size++;

    while (index > 0 && getFreq(heap[(index - 1) / 2]) > getFreq(heap[index]))
    {
        swap(&heap[index], &heap[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
}

void addNodes(){
    if (size <= 1 ){
        printf("Tamaño del heap insuficiente\n");
        return;
    }
    Node* left = pop();
    Node* right = pop();
    Node* newNode = createNode('$');
    sumFreq(newNode, getFreq(left), getFreq(right));
    addLeftNode(newNode, left);
    addRightNode(newNode, right);
    insert(&newNode);
    return;
}

void convertHuffman(){
    while (size > 1) {
        addNodes();
    }
    return;
}

void display()
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