#include "Node.h"
#include "HeapPriorityQueue.h"
#include "HashTableFreq.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    Node * node = createNode('z');
    Node * leftNode = createNode('a');
    Node * rightNode = createNode('g');
    HashTableFreq * hashTableFreq = createHashTableFreq();
    addHashTableFreqNode(hashTableFreq, node);
    addHashTableFreqNode(hashTableFreq, leftNode);
    addHashTableFreqNode(hashTableFreq, rightNode);
    printHashTableFreq(hashTableFreq);   



    //addRep(node, 12);
    //addLeftNode(node, leftNode);
    //addRightNode(node, rightNode);
    //float freq = getFreq(node);
    //Node * left = getLeftNode(node);
    //printf("Caracter: %c\n", getCharacter(left));

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


    destroyHashTableFreq(hashTableFreq);
    return 0;
}