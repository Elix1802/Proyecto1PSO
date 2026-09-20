#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdio.h>
#include "dataStructs/Node.h"
#include "dataStructs/HeapPriorityQueue.h"
#include "dataStructs/HashTableFreq.h"
#include "dataStructs/Dictionary.h"

// -------------------------------------------------------------------
// Estructuras
// -------------------------------------------------------------------

struct BinaryHeader {
    char fileName[128];
    char md5[33];
    int originalSize;
    double frecuencias[256];
    char caracteres[256];
};

typedef struct BinaryHeader binaryHeader;

typedef struct {
    char hex[33]; 
} HashResultado;

// -------------------------------------------------------------------
// Prototipos de Funciones
// -------------------------------------------------------------------

// Utilidades de Hash y Metadatos
HashResultado obtenerHashArchivo(const char *archivo);
void createHeader(HashTableFreq *hashTableFreq, binaryHeader *header, char *route);
void addHeader(binaryHeader *header, FILE *archivo);

// Manejo de Lectura/Escritura de Archivos y Bits
void readFile(char *route, HashTableFreq *hashTableFreq);
void writeBits(char *codigo, FILE *archivo, unsigned char *buffer_bits, int *conteoBits);
void writeFileEncrypted(char *route, HashTableFreq *hashTableFreq, Dictionary *dictionary);

// Algoritmos de Conversión y Árbol de Huffman
void HashTableToHeap(HashTableFreq *hashTableFreq, HeapPriorityQueue *heap);
void generateCodes(Node *node, Dictionary *dictionary, char *code, int depth);
void huffmanToText(char *route);
void encryptFiles(char *route);

// Funciones de Procesamiento por Lote (Directorio)
void compressAllFiles(char *selected_directory);
void decompressAllFiles(char *selected_directory);

#endif // ALGORITHMS_H
