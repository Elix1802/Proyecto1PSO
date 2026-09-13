#include "Node.h"
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

    destroyHashTableFreq(hashTableFreq);
    return 0;
}