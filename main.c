#include "Node.h"
#include "HeapPriorityQueue.h"
#include "HashTableFreq.h"
#include "Dictionary.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "MD5/md5.h"
#include "MD5/metadata.h"


#include <sys/stat.h>
#include <sys/types.h>

struct BinaryHeader {
    char md5[33];
    int originalSize;
    int frecuencias[256];
};

typedef struct {
    char     hex[33]; 
    
} HashResultado;

typedef struct BinaryHeader binaryHeader;


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
    header->originalSize = getTotalCharsCounted(hashTableFreq); //total de caracteres
    for(int i = 0; i<256;i++) {
        header->frecuencias[i] = getFrequencyById(hashTableFreq, i);
    }

}

void addHeader(binaryHeader * header, FILE * archivo) {
    fwrite(&header, sizeof(header), 1, archivo);
}

void writeBytes(char *codigo, FILE *archivo, int *conteoFinal)
{
    if (codigo == NULL)
        return;

    unsigned char buffer_bits = 0;
    int conteoBits = 0;
    for (int i = 0; codigo[i] != '\0'; i++)
    {
        buffer_bits <<= 1;

        if (codigo[i] == '1'){
            buffer_bits |= 1;
        }

        conteoBits++;

        if (conteoBits == 8){
            fputc(buffer_bits, archivo);
            buffer_bits = 0;
            conteoBits = 0;
        }
    }

    *conteoFinal = conteoBits;
}

void writeFileEncrypted(char *route, HashTableFreq *hashTableFreq, Dictionary *dictionary)
{
    //Sección de creación de rutas
    FILE *archivoRead = fopen(route, "r");

    char *folderName = "compressed";
    mkdir(folderName, 0777);

    char *delimitadorCarpeta = strchr(route, '/');
    char *fileName = strdup(delimitadorCarpeta ? delimitadorCarpeta + 1 : route);

    char *delimitadorFormato = strrchr(fileName, '.');
    if (delimitadorFormato) {
        *delimitadorFormato = '\0';
    }
    char routeFile[1024];
    //printf("%s", fileName);

    snprintf(routeFile, sizeof(routeFile), "%s/%s", folderName, fileName);

    FILE *archivo = fopen(routeFile, "wb");
    binaryHeader * header = malloc(sizeof(binaryHeader));

    createHeader(hashTableFreq, header, routeFile);
    addHeader(header, archivo);


    if (archivo == NULL)
    {
        printf("Error al abrir el archivo.\n");
        return;
    }

    int c;
    int conteoFinal = 0;
    while ((c = fgetc(archivoRead)) != EOF)
    {
        char *value = getDictionaryValue(dictionary, c);
        writeBytes(value, archivo, &conteoFinal);
    }

    unsigned char buffer_bits = 0;

    //Exceso
    if (conteoFinal > 0){

        buffer_bits<<=(8-conteoFinal);
        fputc(buffer_bits, archivo);
        buffer_bits = 0;
        conteoFinal = 0;
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

void huffmanToText(char* route, HeapPriorityQueue* heap){
    FILE *archivoHuffmanBinario = fopen(route, "rb");
    FILE *archivoCambiado = fopen("books/Descomprimido.txt", "w");
    Node* root = getRoot(heap);
    
    if (archivoHuffmanBinario == NULL) {
        printf("Error al abrir el archivo.\n");
        return;
    }

    int c;
    while ((c = fgetc(archivoHuffmanBinario)) != EOF) {        
        if (c == '0') {
            root = getLeftNode(root);
        } else if (c == '1') {
            root = getRightNode(root);
        }
        if (isLeaf(root)) {
            fputc(getCharacter(root), archivoCambiado);
            root = getRoot(heap);
        }
    }
    fclose(archivoHuffmanBinario);
    fclose(archivoCambiado);
}

int main()
{
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

    // addRep(node, 12);
    // addLeftNode(node, leftNode);
    // addRightNode(node, rightNode);
    // float freq = getFreq(node);
    // Node * left = getLeftNode(node);
    // printf("Caracter: %c\n", getCharacter(left));

    // destroyHashTableFreq(hashTableFreq);

    HashTableFreq *hashTableFreq = createHashTableFreq();
    HeapPriorityQueue *heap = createHeapPriorityQueue();
    Dictionary *dictionary = createDictionary();
    readFile("books/002_Pride and Prejudice by Jane Austen (186807).txt", hashTableFreq);
    HashTableToHeap(hashTableFreq, heap);
    display(heap);
    convertHuffman(heap);
    display(heap);
    generateCodes(getNodes(heap)[0], dictionary, (char *)malloc(256), 0);
    printDictionaryValues(dictionary);
    writeFileEncrypted("books/002_Pride and Prejudice by Jane Austen (186807).txt", hashTableFreq, dictionary);

    //huffmanToText("books/Corán.txt", heap);

    return 0;
}