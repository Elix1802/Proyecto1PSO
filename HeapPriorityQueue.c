#include <stdio.h>
#include "Node.h"
#include "Node.c"

#define MAX 256

Node* heap[MAX];
int size = 0;


void swap(Node **a, Node **b)
{
    Node *temp = *a;
    *a = *b;
    *b = temp;
}

void insert(Node** value)
{
    heap[size] = *value;
    int index = size;
    size++;

    while (index > 0 && heap[(index - 1) / 2]->freq > heap[index]->freq)
    {
        swap(&heap[index], &heap[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
}


void display()
{
    for (int i = 0; i < size; i++)
        printf("%f ", heap[i]->freq );
    printf("\n");
}

int main()
{
    Node * node = createNode('a');
    Node * node2 = createNode('b');
    addRep(node, 100);
    addRep(node2, 200);
    

    insert(&node2);
    insert(&node);
    
    

    display();
    return 0;
    
}