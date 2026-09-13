#include "Node.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    Node * node = createNode('a');
    Node * leftNode = createNode('b');
    Node * rightNode = createNode('c');

    addRep(node, 12);
    addLeftNode(node, leftNode);
    addRightNode(node, rightNode);
    float freq = getFreq(node);
    Node * left = getLeftNode(node);
    printf("Caracter: %c\n", getCharacter(left));

    destroyNode(node);
    return 0;
}