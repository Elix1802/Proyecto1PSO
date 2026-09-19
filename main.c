#include "Node.h"
#include "HeapPriorityQueue.h"
#include "HashTableFreq.h"
#include "Dictionary.h"
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
    //printHashTableFreq(hashTableFreq);
    printf("\nCantidad de caracteres: %d\n", cant);
    return;
}

void HashTableToHeap(HashTableFreq* hashTableFreq, HeapPriorityQueue* heap) {
    Node** nodes = getHashTableFreq(hashTableFreq);
    for (int i = 0; i < 256; i++) {
        if (nodes[i] != NULL) {
            insert(heap, &nodes[i]);
        }
    }
}

void generateCodes(Node* node, Dictionary* dictionary, char* code, int depth)
{
    if (node == NULL)
        return;

    
    if (isLeaf(node))
    {
        code[depth] = '\0';

        addDictionaryElement(
            dictionary,
            getCharacter(node),
            code
        );

        return;
    }

    
    code[depth] = '0';

    generateCodes(
        getLeftNode(node),
        dictionary,
        code,
        depth + 1
    );

    
    code[depth] = '1';

    generateCodes(
        getRightNode(node),
        dictionary,
        code,
        depth + 1
    );
}

void huffmanToText(char* route, HeapPriorityQueue* heap){
    FILE *archivo = fopen(route, "r");
    Node* root = getRoot(heap);
    
    if (archivo == NULL) {
        printf("Error al abrir el archivo.\n");
        return;
    }

    int c;
    while ((c = fgetc(archivo)) != EOF) {        
        if (c == '0') {
            root = getLeftNode(root);
        } else if (c == '1') {
            root = getRightNode(root);
        }
        if (isLeaf(root)) {
            printf("%c", getCharacter(root));
            root = getRoot(heap);
        }
    }
    fclose(archivo);
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
    HeapPriorityQueue* heap = createHeapPriorityQueue();
    Dictionary* dictionary = createDictionary();
    readFile("books/002_Pride and Prejudice by Jane Austen (186807).txt", hashTableFreq);
    HashTableToHeap(hashTableFreq, heap);
    display(heap);
    convertHuffman(heap);
    display(heap);
    generateCodes(getNodes(heap)[0], dictionary, (char*)malloc(256), 0);
    printDictionaryValues(dictionary);

    huffmanToText("books/Corán.txt", heap);

    return 0;
}