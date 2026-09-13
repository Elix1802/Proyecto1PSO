#include "Node.h"
#include "HeapPriorityQueue.h"
#include "HashTableFreq.h"
#include <stdlib.h>
#include <stdio.h>

void readFile(char* route, HashTableFreq* hashTableFreq) {
    FILE *archivo = fopen(route, "r");

    if (archivo == NULL) {
        printf("Error al abrir el archivo.\n");
        return;
    }

    int cant = 0;
    int c;
    while ((c = fgetc(archivo)) != EOF) {
        putchar(c);
        addHashTableFreqNode(hashTableFreq, createNode(c));
        cant++;
    }


    fclose(archivo);
    updateHashTableFreqNode(hashTableFreq, cant);
    printf("\nCantidad de caracteres: %d\n", cant);
    return;
}

void HashTableToHeap(HashTableFreq* hashTableFreq, Node** heap) {
    Node** nodes = getHashTableFreq(hashTableFreq);
    for (int i = 0; i < 256; i++) {
        if (nodes[i] != NULL) {
            insert(heap, &nodes[i]);
        }
    }
}

int main() {
    /*
    Node * node = createNode('z');
    Node * leftNode = createNode('a');
    Node * rightNode = createNode('g');
    HashTableFreq * hashTableFreq = createHashTableFreq();
    addHashTableFreqNode(hashTableFreq, node);
    addHashTableFreqNode(hashTableFreq, leftNode);
    addHashTableFreqNode(hashTableFreq, rightNode);
    printHashTableFreq(hashTableFreq);   
*/


    //addRep(node, 12);
    //addLeftNode(node, leftNode);
    //addRightNode(node, rightNode);
    //float freq = getFreq(node);
    //Node * left = getLeftNode(node);
    //printf("Caracter: %c\n", getCharacter(left));


    //destroyHashTableFreq(hashTableFreq);

    HashTableFreq * hashTableFreq = createHashTableFreq();
    Node** heap = createHeapPriorityQueue();
    readFile("books/002_Pride and Prejudice by Jane Austen (186807).txt", hashTableFreq);
    HashTableToHeap(hashTableFreq, heap);
    display(heap);
    convertHuffman(heap);
    display(heap);
    return 0;
}