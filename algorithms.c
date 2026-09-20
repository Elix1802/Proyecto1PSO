#include "dataStructs/Node.h"
#include "dataStructs/HeapPriorityQueue.h"
#include "dataStructs/HashTableFreq.h"
#include "dataStructs/Dictionary.h"
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#include <string.h>
#include "MD5/md5.h"
#include "MD5/metadata.h"
#include "algorithms.h"


#include <sys/stat.h>
#include <sys/types.h>

static char *selected_directoryA = NULL;
static char *selected_directoryAD = NULL;


HashResultado obtenerHashArchivo(const char *archivo) {
    HashResultado resultado;
    unsigned char md5[MD5_DIGEST_SIZE];
    if (md5_file(archivo, md5) != 0) {
        fprintf(stderr, "Error: no se pudo calcular el hash MD5 del archivo %s\n", archivo);
        resultado.hex[0] = '\0';
        return resultado;
    }
    md5_to_hex(md5, resultado.hex);
    return resultado;
}


//Sección de archivos

void readFile(char *route, HashTableFreq *hashTableFreq)
{
    FILE *archivo = fopen(route, "r");

    if (archivo == NULL)
    {
        printf("Error al abrir el archivo.\n");
        return;
    }

    int cant = 0;
    int c;
    while ((c = fgetc(archivo)) != EOF)
    {
        // putchar(c);
        addHashTableFreqNode(hashTableFreq, createNode(c));
        cant++;
    }

    fclose(archivo);
    updateHashTableFreqNode(hashTableFreq, cant);
    // printHashTableFreq(hashTableFreq);
    printf("\nCantidad de caracteres: %d\n", cant);
    return;
}

void createHeader (HashTableFreq *hashTableFreq, binaryHeader * header, char * route) {
    HashResultado resultado = obtenerHashArchivo(route);
    strcpy(header->md5, resultado.hex);
    header->originalSize = getTotalCharsCounted(hashTableFreq);
    for(int i = 0; i < 256; i++) {
        header->caracteres[i] = getCharById(hashTableFreq, i);
        header->frecuencias[i] = getFrequencyById(hashTableFreq, i);
    }
}

void addHeader(binaryHeader * header, FILE * archivo) {
    if (header != NULL && archivo != NULL) {
        fwrite(header, sizeof(binaryHeader), 1, archivo);
    }
}

void writeBits(char *codigo, FILE *archivo, unsigned char *buffer_bits, int *conteoBits)
{
    if (codigo == NULL)
        return;

    for (int i = 0; codigo[i] != '\0'; i++)
    {
        *buffer_bits <<= 1;

        if (codigo[i] == '1')
            *buffer_bits |= 1;

        (*conteoBits)++;

        if (*conteoBits == 8)
        {
            fputc(*buffer_bits, archivo);
            *buffer_bits = 0;
            *conteoBits = 0;
        }
    }
}

void writeFileEncrypted(char *route, HashTableFreq *hashTableFreq, Dictionary *dictionary)
{
    //Sección de creación de rutas
    FILE *archivoRead = fopen(route, "r");

    if (archivoRead == NULL)
    {
        printf("Error al abrir el archivo de lectura.\n");
        return;
    }

    char folderName[1024];
    char *folderNamePtr = "compressed";
    snprintf(folderName, sizeof(folderName), "%s/%s", selected_directoryA, folderNamePtr);
    mkdir(folderName, 0777);


    // Cambiamos strchr por strrchr para encontrar el último slash '/'
    char *delimitadorCarpeta = strrchr(route, '/');
    char *fileName = strdup(delimitadorCarpeta ? delimitadorCarpeta + 1 : route);

    char *delimitadorFormato = strrchr(fileName, '.');
    if (delimitadorFormato) {
        *delimitadorFormato = '\0';
    }
    char routeFile[1024];

    snprintf(routeFile, sizeof(routeFile), "%s/%s.bin", folderName, fileName);

    //FIn de creación de rutas

    FILE *archivo = fopen(routeFile, "wb");

    if (archivo == NULL)
    {
        printf("Error al abrir el archivo.\n");
        free(fileName);
        fclose(archivoRead);
        return;
    }

    binaryHeader * header = malloc(sizeof(binaryHeader));

    createHeader(hashTableFreq, header, route);
    addHeader(header, archivo);

    int c;
    unsigned char buffer_bits = 0;
    int conteoBits = 0;
    while ((c = fgetc(archivoRead)) != EOF)
    {
        char *value = getDictionaryValue(dictionary, c);
        writeBits(value, archivo, &buffer_bits, &conteoBits);
    }

    //Exceso
    if (conteoBits > 0)
    {
        buffer_bits <<= (8 - conteoBits);
        fputc(buffer_bits, archivo);
    }

    free(fileName);
    free(header);
    fclose(archivoRead);
    fclose(archivo);
    return;
}


//Sección de algoritmos
void HashTableToHeap(HashTableFreq *hashTableFreq, HeapPriorityQueue *heap)
{
    Node **nodes = getHashTableFreq(hashTableFreq);
    for (int i = 0; i < 256; i++)
    {
        if (nodes[i] != NULL)
        {
            insert(heap, &nodes[i]);
        }
    }
}

void generateCodes(Node *node, Dictionary *dictionary, char *code, int depth)
{
    if (node == NULL)
        return;

    if (isLeaf(node))
    {
        code[depth] = '\0';

        addDictionaryElement(
            dictionary,
            getCharacter(node),
            code);

        return;
    }

    code[depth] = '0';

    generateCodes(
        getLeftNode(node),
        dictionary,
        code,
        depth + 1);

    code[depth] = '1';

    generateCodes(
        getRightNode(node),
        dictionary,
        code,
        depth + 1);
}

void huffmanToText(char* route)
{
    FILE *archivoHuffmanBinario = fopen(route, "rb");

    char folderName[1024];
    char *folderNamePtr = "decompressed";
    snprintf(folderName, sizeof(folderName), "%s/%s", selected_directoryAD, folderNamePtr);
    mkdir(folderName, 0777);

    char *delimitadorCarpeta = strrchr(route, '/');
    char *fileName = strdup(delimitadorCarpeta ? delimitadorCarpeta + 1 : route);

    char *delimitadorFormato = strrchr(fileName, '.');
    if (delimitadorFormato) {
        *delimitadorFormato = '\0';
    }
    char routeFile[1024];

    snprintf(routeFile, sizeof(routeFile), "%s/%s.txt", folderName, fileName);

    FILE *archivoCambiado = fopen(routeFile, "w");

    if (archivoHuffmanBinario == NULL || archivoCambiado == NULL)
    {
        printf("Error al abrir el archivo.\n");
        free(fileName);
        if (archivoHuffmanBinario) fclose(archivoHuffmanBinario);
        if (archivoCambiado) fclose(archivoCambiado);
        return;
    }

    binaryHeader header;
    fread(&header, sizeof(binaryHeader), 1, archivoHuffmanBinario);


    HeapPriorityQueue *heapDecodificacion = createHeapPriorityQueue();

    for (int i = 0; i < 256; i++)
    {
        if (header.frecuencias[i] > 0)
        {
            Node *node = createNodeFreq(header.caracteres[i], header.frecuencias[i]);
            insert(heapDecodificacion, &node);
        }
    }

    convertHuffman(heapDecodificacion);
    Node* root = getRoot(heapDecodificacion);

    int caracteresDecodificados = 0;
    int c;

    while (caracteresDecodificados < header.originalSize &&
           (c = fgetc(archivoHuffmanBinario)) != EOF)
    {
        for (int i = 7; i >= 0 && caracteresDecodificados < header.originalSize; i--)
        {
            int bit = (c >> i) & 1;

            root = bit == 0 ? getLeftNode(root) : getRightNode(root);

            if (isLeaf(root))
            {
                fputc(getCharacter(root), archivoCambiado);
                caracteresDecodificados++;
                root = getRoot(heapDecodificacion);
            }
        }
    }

    free(fileName);
    fclose(archivoHuffmanBinario);
    fclose(archivoCambiado);
}

void encryptFiles(char * route) {
    HashTableFreq *hashTableFreq = createHashTableFreq();
    HeapPriorityQueue *heap = createHeapPriorityQueue();
    Dictionary *dictionary = createDictionary();

    readFile(route, hashTableFreq);
    HashTableToHeap(hashTableFreq, heap);

    convertHuffman(heap);
    generateCodes(getNodes(heap)[0], dictionary, (char *)malloc(256), 0);
    writeFileEncrypted(route, hashTableFreq, dictionary);

    destroyDictionary(dictionary);
    destroyHashTableFreq(hashTableFreq);
    destroyHeapPriorityQueue(heap);
}


StatRecord* compressAllFiles(char *selected_directory) {
    StatRecord* record = NULL;
    record->decompAcceleration = 0.0;
    record->decompress_time_s = 0.0;
    record->method = "Basic";

    selected_directoryA = selected_directory;
    DIR * dir =  opendir(selected_directory);

    if (dir == NULL) {
        dir =  opendir("./books");
    }

    struct dirent *entrada;

    char folderFileName[512];

    int i = 0;

    while ((entrada = readdir(dir)) != NULL) {

        if(i == 5) break;

        if (!strcmp(entrada->d_name, ".") || !strcmp(entrada->d_name, "..")) {
            continue;
        }

        snprintf(folderFileName, sizeof(folderFileName), "%s/%s", selected_directoryA, entrada->d_name);
        printf("Archivo: %s\n", folderFileName);
        encryptFiles(folderFileName);
        i++;
    }

}

void decompressAllFiles(char *selected_directory) {
    selected_directoryAD = selected_directory;

    DIR * dir =  opendir(selected_directoryAD);

    if (dir == NULL) {
        dir =  opendir("./compressed");
    }

    struct dirent *entrada;


    char folderFileName[512];
    int i = 0;

    while ((entrada = readdir(dir)) != NULL) {

        if(i == 5) break;

        if (!strcmp(entrada->d_name, ".") || !strcmp(entrada->d_name, "..")) {
            continue;
        }

        snprintf(folderFileName, sizeof(folderFileName), "%s/%s", selected_directoryAD, entrada->d_name);
        printf("Archivo: %s\n", folderFileName);
        huffmanToText(folderFileName);
        i++;
    }

}
