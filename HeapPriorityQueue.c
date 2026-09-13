#include <stdio.h>
#include "Node.h"
#include "Node.c"
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

Node* pop(){
    if (size == 0) return NULL;
    Node* root = heap[0];
    heap[0] = heap[size - 1];
    size--;
    while (1) {
        int left = 2 * 0 + 1;
        int right = 2 * 0 + 2;
        int smallest = 0;

        if (left < size && heap[left]->freq < heap[smallest]->freq) {
            smallest = left;
        }
        if (right < size && heap[right]->freq < heap[smallest]->freq) {
            smallest = right;
        }
        if (smallest == 0) break;

        swap(&heap[0], &heap[smallest]);
    }
    return root;
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

void addNodes(){
    if (size <= 1 ){
        printf("Tamaño del heap insuficiente\n");
        return;
    }
    Node* left = pop();
    Node* right = pop();
    Node* newNode = createNode('$');
    addLeftNode(newNode, left);
    addRightNode(newNode, right);
    insert(&newNode);
    return;
}



void display()
{
    for (int i = 0; i < size; i++)
    printf("%c : %f \n", heap[i]->character, heap[i]->freq);
    printf("\n");
}


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
    
    addNodes();

    display();

    addNodes();

    display();
    
    return 0;
    
}
