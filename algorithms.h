#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdio.h>
#include <pthread.h>

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


typedef struct {
    int inicio;
    int final; 
    struct dirent **nameList;
    FILE * huffmanFile;
    pthread_mutex_t *mutexFile;
} entradaHilo;

typedef struct {
    int inicio;
    int final; 
    struct dirent **nameList;
    char huffFileNameRoute[1024];
} entradaProceso;


typedef struct {
    const char *method;
    const char *healthPercentage;
    double compress_time_s;
    double decompress_time_s;
    double compAcceleration;
    double decompAcceleration;
    double filesSize;
    double compressedSize;
    double radius;
} StatRecord;

typedef struct BinaryHeader binaryHeader;

typedef struct {
    char hex[33]; 
} HashResultado;

// -------------------------------------------------------------------
// Prototipos de Funciones
// -------------------------------------------------------------------

double elapsedTime(struct timespec start, struct timespec end);

// Utilidades de Hash y Metadatos
HashResultado obtenerHashArchivo(const char *archivo);
void createHeader(HashTableFreq *hashTableFreq, binaryHeader *header, char *route);
void addHeader(binaryHeader *header, FILE *archivo);

// Manejo de Lectura/Escritura de Archivos y Bits
void readFile(char *route, HashTableFreq *hashTableFreq);
void writeBits(char *codigo, FILE *archivo, unsigned char *buffer_bits, int *conteoBits);
void writeFileEncrypted(char *route, HashTableFreq *hashTableFreq, Dictionary *dictionary, FILE * huffmanFile);

// Algoritmos de Conversión y Árbol de Huffman
void HashTableToHeap(HashTableFreq *hashTableFreq, HeapPriorityQueue *heap);
void generateCodes(Node *node, Dictionary *dictionary, char *code, int depth);
float huffmanToText(char *route, FILE *archivoHuffmanBinario);
void encryptFiles(char *route, FILE * huffmanFile);
void encryptFilesThread(char * route, FILE * huffmanFile, entradaHilo * entrada);

// Funciones de Procesamiento por Lote (Directorio)
StatRecord* compressAllFiles(char *selected_directory);
StatRecord* decompressAllFiles(char *selected_directory);

StatRecord* compressAllFilesThreads(char *selected_directory);
StatRecord* compressAllFilesFork(char * selected_directory);


#endif // ALGORITHMS_H
