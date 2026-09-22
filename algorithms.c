#include "dataStructs/Node.h"
#include "dataStructs/HeapPriorityQueue.h"
#include "dataStructs/HashTableFreq.h"
#include "dataStructs/Dictionary.h"
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>
#include "MD5/md5.h"
#include "MD5/metadata.h"
#include "algorithms.h"


#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/wait.h>

#include <time.h>


static char *selected_directoryA = NULL;
static char *selected_directoryAD = NULL;


/**
 * Function which shows how many miliseconds and nanoseconds
 * have passed between the execution of the algorithm
 */
double elapsedTime(struct timespec start, struct timespec end)
{
    long seconds = end.tv_sec - start.tv_sec;
    long nanoseconds = end.tv_nsec - start.tv_nsec;

    if (nanoseconds < 0)
    {
        seconds--;
        nanoseconds += 1000000000L;
    }

    double elapsed_ns = (seconds * 1e9) + nanoseconds;
    double elapsed_ms = elapsed_ns / 1e6;

    return elapsed_ms;
}

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

    //filenameSection
    char *delimitadorCarpeta = strrchr(route, '/');
    const char *fileNameCleaned = delimitadorCarpeta ? delimitadorCarpeta + 1 : route;

    strncpy(header->fileName, fileNameCleaned, sizeof(header->fileName) - 1);
    header->fileName[sizeof(header->fileName) - 1] = '\0';


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

void writeFileEncrypted(char *route, HashTableFreq *hashTableFreq, Dictionary *dictionary, FILE * huffmanFile)
{
    //Sección de creación de rutas
    FILE *archivoRead = fopen(route, "rb");

    if (archivoRead == NULL)
    {
        printf("Error al abrir el archivo de lectura.\n");
        return;
    }


    binaryHeader * header = malloc(sizeof(binaryHeader));

    createHeader(hashTableFreq, header, route);
    addHeader(header, huffmanFile);

    int c;
    unsigned char buffer_bits = 0;
    int conteoBits = 0;
    while ((c = fgetc(archivoRead)) != EOF)
    {
        char *value = getDictionaryValue(dictionary, c);
        writeBits(value, huffmanFile, &buffer_bits, &conteoBits);
    }

    //Exceso
    if (conteoBits > 0)
    {
        buffer_bits <<= (8 - conteoBits);
        fputc(buffer_bits, huffmanFile);
    }

    free(header);
    fclose(archivoRead);
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

float huffmanToText(char* route,  FILE *archivoHuffmanBinario)
{
    binaryHeader header;   
    float health = 0.0;
    float files = 0.0;
    while (fread(&header, sizeof(binaryHeader), 1, archivoHuffmanBinario) == 1) {
            files ++;
            printf("Extrayendo: %s (MD5: %s)\n", header.fileName, header.md5);

            char routeFile[1024];
            snprintf(routeFile, sizeof(routeFile), "%s/%s", route, header.fileName);

            FILE *archivoSalida = fopen(routeFile, "w");
            if (archivoSalida == NULL) {
                printf("Error al crear el archivo extraído: %s\n", routeFile);
                break;
            }

            HeapPriorityQueue *heapDecodificacion = createHeapPriorityQueue();
            for (int i = 0; i < 256; i++) {
                if (header.frecuencias[i] > 0) {
                    Node *node = createNodeFreq(header.caracteres[i], header.frecuencias[i]);
                    insert(heapDecodificacion, &node);
                }
            }

            convertHuffman(heapDecodificacion);
            Node *root = getRoot(heapDecodificacion);

            int caracteresDecodificados = 0;
            int c;

            while (caracteresDecodificados < header.originalSize &&
                (c = fgetc(archivoHuffmanBinario)) != EOF) {

                for (int i = 7; i >= 0 && caracteresDecodificados < header.originalSize; i--) {
                    int bit = (c >> i) & 1;
                    root = (bit == 0) ? getLeftNode(root) : getRightNode(root);

                    if (isLeaf(root)) {
                        fputc(getCharacter(root), archivoSalida);
                        caracteresDecodificados++;
                        root = getRoot(heapDecodificacion);
                    }
                }
            }
            HashResultado result = obtenerHashArchivo(routeFile);
            if (strcmp(result.hex, header.md5)) {
                health += 1.0;
            }
            fclose(archivoSalida);
            destroyHeapPriorityQueue(heapDecodificacion);
        }

        fclose(archivoHuffmanBinario);
        printf("¡Proceso de descompresión finalizado exitosamente!\n");

    return health/files;
}

float huffmanToTextThread(char* route,  FILE *archivoHuffmanBinario){
    
    return 0.0;
}

void encryptFiles(char * route, FILE * huffmanFile) {
    HashTableFreq *hashTableFreq = createHashTableFreq();
    HeapPriorityQueue *heap = createHeapPriorityQueue();
    Dictionary *dictionary = createDictionary();

    readFile(route, hashTableFreq);
    HashTableToHeap(hashTableFreq, heap);

    convertHuffman(heap);
    generateCodes(getNodes(heap)[0], dictionary, (char *)malloc(256), 0);
    writeFileEncrypted(route, hashTableFreq, dictionary, huffmanFile);

    destroyDictionary(dictionary);
    destroyHashTableFreq(hashTableFreq);
    destroyHeapPriorityQueue(heap);
}


StatRecord* compressAllFiles(char *selected_directory) {
    StatRecord* record = (StatRecord*) malloc(sizeof(StatRecord));
    if (record == NULL) return NULL;
    
    record->decompAcceleration = 0.0;
    record->decompress_time_s = 0.0;
    record->compress_time_s = 0.0;
    record->method = "Basic";
    record->radius = 999.999;
    record->filesSize = 0.0;
    record->compressedSize = 0.0;
    record->healthPercentage = "0%";

    struct timespec start, end;
    
    selected_directoryA = selected_directory;
    DIR * dir = opendir(selected_directory);

    if (dir == NULL) {
        selected_directory = "./books";
        selected_directoryA = selected_directory;
        dir = opendir(selected_directory);
        if (dir == NULL) {
            free(record);
            return NULL;
        }
    }

    char folderName[1024];
    char * newFolder = "compressed";
    snprintf(folderName, sizeof(folderName), "%s/%s", selected_directoryA, newFolder); //Nueva carpeta en donde vivirá el .huff
    mkdir(folderName, 0777);

    char huffFileNameRoute[1024]; // Nombre del archivo huff en el directorio actual
    char *fileName = "books";
    snprintf(huffFileNameRoute, sizeof(huffFileNameRoute), "%s/%s.huff", folderName, fileName);

    FILE * huffmanFile = fopen(huffFileNameRoute, "wb");

    if (huffmanFile == NULL) {
        perror("Error al crear books.huff");
        closedir(dir);
        free(record);
        return NULL;
    }

    struct dirent *entrada;
    char folderFileName[512]; // Nombre del documento actual en su directorio
    int i = 0;

    while ((entrada = readdir(dir)) != NULL) {

        //if (i == 5) break;

        if (!strcmp(entrada->d_name, ".") || !strcmp(entrada->d_name, "..")) {
            continue;
        }

        // Ignorar subcarpetas
        if (entrada->d_type == DT_DIR) {
            continue;
        }   

        if (strstr(entrada->d_name, ".huff") != NULL) {
            continue;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);

        snprintf(folderFileName, sizeof(folderFileName), "%s/%s", selected_directoryA, entrada->d_name);
        printf("Archivo: %s\n", folderFileName);

        // Mide el tamaño del archivo original
        struct stat fileStat;
        if (stat(folderFileName, &fileStat) == 0) {
            double sizeKB = fileStat.st_size / 1000.0;
            record->filesSize += sizeKB;
        } else {
            perror("stat");
        }

        encryptFiles(folderFileName, huffmanFile);

        clock_gettime(CLOCK_MONOTONIC, &end);
        record->compress_time_s += elapsedTime(start, end);

        i++;
    }

    fclose(huffmanFile);
    closedir(dir);

    // Mide el tamaño total final del archivo .huff generado
    struct stat compressedStat;
    if (stat(huffFileNameRoute, &compressedStat) == 0) {
        record->compressedSize = compressedStat.st_size / 1000.0; 
    } else {
        perror("stat (archivo comprimido)");
    }

    return record;
}


void encryptFilesThread(char * route, FILE * huffmanFile, entradaHilo * entrada) {
    HashTableFreq *hashTableFreq = createHashTableFreq();
    HeapPriorityQueue *heap = createHeapPriorityQueue();
    Dictionary *dictionary = createDictionary();

    readFile(route, hashTableFreq);
    HashTableToHeap(hashTableFreq, heap);

    convertHuffman(heap);
    generateCodes(getNodes(heap)[0], dictionary, (char *)malloc(256), 0);

    pthread_mutex_lock(entrada->mutexFile);
    writeFileEncrypted(route, hashTableFreq, dictionary, huffmanFile);
    pthread_mutex_unlock(entrada->mutexFile);

    destroyDictionary(dictionary);
    destroyHashTableFreq(hashTableFreq);
    destroyHeapPriorityQueue(heap);
}



void *writtingThread(void*arg) {
    entradaHilo * entrada = (entradaHilo*) arg;
    for (int i = entrada->inicio; i< entrada->final; i++) {
        struct dirent * document = entrada->nameList[i];
        if (strcmp(document->d_name, ".") != 0 && strcmp(document->d_name, "..") != 0) { 
            char fullPath[1024];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", selected_directoryA, document->d_name);
            struct stat statbuf;
            if (stat(fullPath, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
                encryptFilesThread(fullPath, entrada->huffmanFile, entrada);
            }
        
        }

    }
    return NULL;
}
 
StatRecord* compressAllFilesThreads(char *selected_directory) {
    StatRecord* record = (StatRecord*) malloc(sizeof(StatRecord));
    if (record == NULL) return NULL;
    
    record->decompAcceleration = 0.0;
    record->decompress_time_s = 0.0;
    record->compress_time_s = 0.0;
    record->method = "Basic";
    record->radius = 999.999;
    record->filesSize = 0.0;
    record->compressedSize = 0.0;

    struct timespec start, end;
    int threadNumber = 8;
    int limitG1 = 12;
    int limitG2 = 13;

    double * resultChilds[threadNumber];
    pthread_t threads[threadNumber];
    entradaHilo entradas[threadNumber];
    pthread_mutex_t huffmanFileMutex = PTHREAD_MUTEX_INITIALIZER;


    selected_directoryA = selected_directory;
    DIR * dir = opendir(selected_directory);

    if (dir == NULL) {
        selected_directory = "./books";
        selected_directoryA = selected_directory;
        dir = opendir(selected_directory);
        if (dir == NULL) {
            free(record);
            return NULL;
        }
    }

    char folderName[1024];
    char * newFolder = "compressedThread";
    snprintf(folderName, sizeof(folderName), "%s/%s", selected_directoryA, newFolder); //Nueva carpeta en donde vivirá el .huff
    mkdir(folderName, 0777);

    char huffFileNameRoute[1024]; // Nombre del archivo huff en el directorio actual
    char *fileName = "books";
    snprintf(huffFileNameRoute, sizeof(huffFileNameRoute), "%s/%s.huff", folderName, fileName);

    FILE * huffmanFile = fopen(huffFileNameRoute, "wb");

    if (huffmanFile == NULL) {
        perror("Error al crear books.huff");
        closedir(dir);
        free(record);
        return NULL;
    }

    struct dirent **nameList;
    int n = scandir(selected_directoryA, &nameList, NULL, alphasort);
    if (n < 0) {
        perror("Error al leer el directorio");
        return NULL;
    }
    

    //Variables que represetan por cual sección del arreglo de archivos vamos
    int inicio = 0;
    int final = limitG1;

    char folderFileName[512]; // Nombre del documento actual en su directorio
    int i = 0;

    for (int i = 0; i<threadNumber; i++) {
        entradas[i].inicio = inicio;
        entradas[i].final = final;
        entradas[i].nameList = nameList;
        entradas[i].huffmanFile = huffmanFile;
        entradas[i].mutexFile = &huffmanFileMutex;
        if(i<4){

            pthread_create(&threads[i], NULL, writtingThread, (void *)&entradas[i]);
            if(i!=3) {
                inicio += limitG1;
                final += limitG1;
            } else {
                inicio += limitG1;
                final += limitG2;
            }

        }else {
            pthread_create(&threads[i], NULL, writtingThread, (void *)&entradas[i]);
                inicio += limitG2;
                final += limitG2;
        }
    }
    
    for(int i =0; i<threadNumber; i++) {
        pthread_join(threads[i], NULL);
    }

    fclose(huffmanFile);
    free(nameList);

    closedir(dir);

    return record;
}


//Fork functions

void encryptFilesFork(char * route, FILE * huffmanFile) {
    HashTableFreq *hashTableFreq = createHashTableFreq();
    HeapPriorityQueue *heap = createHeapPriorityQueue();
    Dictionary *dictionary = createDictionary();

    int fd = fileno(huffmanFile); //Necesario para realizar un lock del archivo al escribir

    readFile(route, hashTableFreq);
    HashTableToHeap(hashTableFreq, heap);

    convertHuffman(heap);
    generateCodes(getNodes(heap)[0], dictionary, (char *)malloc(256), 0);

    //En esta sección se bloquea el archivo huffman 
    flock(fd, LOCK_EX);
    fseek(huffmanFile, 0, SEEK_END);
    writeFileEncrypted(route, hashTableFreq, dictionary, huffmanFile);
    fflush(huffmanFile);
    flock(fd, LOCK_UN); 
    

    destroyDictionary(dictionary);
    destroyHashTableFreq(hashTableFreq);
    destroyHeapPriorityQueue(heap);
}

/**
 * Función writtingFork
 * Descripción: Función que se encarga de escribir en el archivo.huff los diferentes libros
 * recibe un struc entrada el cual posee los siguientes elementos
 * -    int inicio: Indice de inicio en la carpeta de los libros
 * -    int final; Indice del final de la carpeta de los libros
 * -    struct dirent **nameList;: Struct encargado de las posiciones de la lista
 * -    char huffFileNameRoute[1024]; Ruta del archivo huffman a crear
 */

void writtingFork(entradaProceso entrada) {
    FILE * huffmanFile = fopen(entrada.huffFileNameRoute, "ab"); //Vamos a escribir en el amiguín
    if (!huffmanFile) {
        perror("Error al abrir el archivo contenedor .huff");
        _exit(1);
    }

    for (int i = entrada.inicio; i< entrada.final; i++) {
        struct dirent * document = entrada.nameList[i];
        if (strcmp(document->d_name, ".") != 0 && strcmp(document->d_name, "..") != 0) { 
            char fullPath[1024];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", selected_directoryA, document->d_name);
            struct stat statbuf;
            if (stat(fullPath, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
                encryptFilesFork(fullPath, huffmanFile);
            }
        
        }

    }

    fclose(huffmanFile);
    _exit(0);
}
 

StatRecord* compressAllFilesFork(char * selected_directory){
    StatRecord* record = (StatRecord*) malloc(sizeof(StatRecord));
    if (record == NULL) return NULL;
    
    record->decompAcceleration = 0.0;
    record->decompress_time_s = 0.0;
    record->compress_time_s = 0.0;
    record->method = "Basic";
    record->radius = 999.999;
    record->filesSize = 0.0;
    record->compressedSize = 0.0;

    struct timespec start, end;
    int processNumber = 2;
    int inicio = 0;
    int final = 50;

    double * resultChilds[processNumber];
    entradaProceso entradas[processNumber];


    selected_directoryA = selected_directory;
    DIR * dir = opendir(selected_directory);

    if (dir == NULL) {
        selected_directory = "./books";
        selected_directoryA = selected_directory;
        dir = opendir(selected_directory);
        if (dir == NULL) {
            free(record);
            return NULL;
        }
    }

    char folderName[1024];
    char * newFolder = "compressedFork";
    snprintf(folderName, sizeof(folderName), "%s/%s", selected_directoryA, newFolder); //Nueva carpeta en donde vivirá el .huff
    mkdir(folderName, 0777);

    char huffFileNameRoute[1024]; // Nombre del archivo huff en el directorio actual
    char *fileName = "books";
    snprintf(huffFileNameRoute, sizeof(huffFileNameRoute), "%s/%s.huff", folderName, fileName);

    FILE * huffmanFile = fopen(huffFileNameRoute, "wb");

    if (huffmanFile == NULL) {
        perror("Error al crear books.huff");
        closedir(dir);
        free(record);
        return NULL;
    }
    fclose(huffmanFile);

    //Sección de nameList, aqui se conoce la cantidad de elementos en la capreta
    struct dirent **nameList;
    int n = scandir(selected_directoryA, &nameList, NULL, alphasort);
    if (n < 0) {
        perror("Error al leer el directorio");
        return NULL;
    }

    int totalArchivos = n; 
    int mitad = totalArchivos / 2;

    entradas[0].inicio = 0;
    entradas[0].final = mitad;
    entradas[0].nameList = nameList;
    strcpy(entradas[0].huffFileNameRoute, huffFileNameRoute);

    entradas[1].inicio = mitad;
    entradas[1].final = totalArchivos;
    entradas[1].nameList = nameList;
    strcpy(entradas[1].huffFileNameRoute, huffFileNameRoute);

    //En esta sección se crean dos procesos hijos para dejar al padre nada más trabajr en la interfaz
    pid_t pid1 = fork();
    printf("Inicia compresión por subproceso \n");
    if (pid1 < 0) {
        perror("Error al crear el proceso 1");
        closedir(dir);
        free(record);
        return NULL;
    }

    if(pid1 == 0) {
        writtingFork(entradas[0]);
    }

    pid_t pid2 = fork();

    if (pid2 < 0) {
        perror("Error al crear el proceso 2");
        closedir(dir);
        free(record);
        return NULL;
    }

    if(pid2 == 0) {
        writtingFork(entradas[1]);
    }

    //Se esperan a los dos procesos creados

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    closedir(dir);
    return record;
}

StatRecord* decompressAllFiles(char *selected_directory) {

    StatRecord* record = (StatRecord*) malloc(sizeof(StatRecord));
    if (record == NULL) return NULL;

    record->decompAcceleration = 0.0;
    record->decompress_time_s = 0.0;
    record->compress_time_s = 0.0;
    record->method = "Basic";
    record->radius = 999.999;
    record->filesSize = 0.0;
    record->compressedSize = 0.0;

    float health = 0.0;
    char healthP[10];

    struct timespec start, end;

    selected_directoryAD = selected_directory;

    DIR * dir = opendir(selected_directoryAD);

    if (dir == NULL) {
        selected_directory = "./compressed";
        selected_directoryAD = selected_directory;
        dir = opendir(selected_directoryAD);
        if (dir == NULL) {
            free(record);
            return NULL;
        }
    }

    char folderName[1024];
    char * newFolder = "decompressed";


    struct dirent *entrada;

    char folderFileName[512];
    int i = 0; //Contador de huff

    clock_gettime(CLOCK_MONOTONIC, &start);

    while ((entrada = readdir(dir)) != NULL) {

        if (!strcmp(entrada->d_name, ".") || !strcmp(entrada->d_name, "..")) {
            continue;
        }

        if (entrada->d_type == DT_DIR) {
            continue;
        }

        if (strstr(entrada->d_name, ".huff") != NULL) {
            snprintf(folderName, sizeof(folderName), "%s/%s", selected_directoryAD, newFolder);
            mkdir(folderName, 0777);
            snprintf(folderFileName, sizeof(folderFileName), "%s/%s", selected_directoryAD, entrada->d_name);

        
            struct stat huffStat;
            if (stat(folderFileName, &huffStat) == 0) {
                record->compressedSize += huffStat.st_size / 1000.0;
            } else {
                perror("stat (.huff)");
            }

            FILE *archivoHuffmanBinario = fopen(folderFileName, "rb");
            if (archivoHuffmanBinario == NULL) {
                printf("Error al abrir el archivo contenedor: %s\n", folderFileName);
                closedir(dir);
                free(record);
                return NULL;
            }

            printf("Archivo: %s\n", folderFileName);
            health = huffmanToText(folderName, archivoHuffmanBinario);
            
            snprintf(healthP, sizeof(healthP), "%.2f", health * 100);
            
            i++;
        }
    }

    closedir(dir);

    clock_gettime(CLOCK_MONOTONIC, &end);
    record->decompress_time_s = elapsedTime(start, end);
    record->healthPercentage = healthP;


    if (i!=0) {
    DIR *outDir = opendir(folderName);
    if (outDir != NULL) {
        struct dirent *outEntry;
        struct stat outStat;
        char outPath[1024];

        while ((outEntry = readdir(outDir)) != NULL) {
            if (!strcmp(outEntry->d_name, ".") || !strcmp(outEntry->d_name, "..")) continue;
            if (outEntry->d_type == DT_DIR) continue;

            snprintf(outPath, sizeof(outPath), "%s/%s", folderName, outEntry->d_name);
            if (stat(outPath, &outStat) == 0) {
                record->filesSize += outStat.st_size / 1000.0;
            }
        }
        closedir(outDir);
    }

    return record; 
    }

    else return NULL;

}

StatRecord* decompressAllFilesThread(char *selected_directory) {
    StatRecord* record = (StatRecord*) malloc(sizeof(StatRecord));
    if (record == NULL) return NULL;

    record->decompAcceleration = 0.0;
    record->decompress_time_s = 0.0;
    record->compress_time_s = 0.0;
    record->method = "Basic";
    record->radius = 999.999;
    record->filesSize = 0.0;
    record->compressedSize = 0.0;

    float health = 0.0;
    char healthP[10];

    struct timespec start, end;

    selected_directoryAD = selected_directory;

    DIR * dir = opendir(selected_directoryAD);

    if (dir == NULL) {
        selected_directory = "./compressed";
        selected_directoryAD = selected_directory;
        dir = opendir(selected_directoryAD);
        if (dir == NULL) {
            free(record);
            return NULL;
        }
    }

    char folderName[1024];
    char * newFolder = "decompressedThread";

    struct dirent *entrada;

    char folderFileName[512];
    int i = 0; //Contador de huff

    clock_gettime(CLOCK_MONOTONIC, &start);

    while ((entrada = readdir(dir)) != NULL) {

        if (!strcmp(entrada->d_name, ".") || !strcmp(entrada->d_name, "..")) {
            continue;
        }

        if (entrada->d_type == DT_DIR) {
            continue;
        }

        if (strstr(entrada->d_name, ".huff") != NULL) {
            snprintf(folderName, sizeof(folderName), "%s/%s", selected_directoryAD, newFolder);
            mkdir(folderName, 0777);
            snprintf(folderFileName, sizeof(folderFileName), "%s/%s", selected_directoryAD, entrada->d_name);

        
            struct stat huffStat;
            if (stat(folderFileName, &huffStat) == 0) {
                record->compressedSize += huffStat.st_size / 1000.0;
            } else {
                perror("stat (.huff)");
            }

            FILE *archivoHuffmanBinario = fopen(folderFileName, "rb");
            if (archivoHuffmanBinario == NULL) {
                printf("Error al abrir el archivo contenedor: %s\n", folderFileName);
                closedir(dir);
                free(record);
                return NULL;
            }

            health = huffmanToText(folderName, archivoHuffmanBinario);
            
            snprintf(healthP, sizeof(healthP), "%.2f", health * 100);
            
            i++;
        }
    }

    closedir(dir);

    
}
