/*
 * ejemplo_flujo_completo.c
 *
 * Simula el flujo completo que usarás en tu compresor real:
 *   1. Firmar un archivo original (antes de Huffman)
 *   2. Escribir sus metadatos a un archivo .cmb
 *   3. (Aquí en tu proyecto real: comprimir con Huffman y escribir
 *      los bytes comprimidos justo después de los metadatos)
 *   4. Leer los metadatos de vuelta desde el .cmb
 *   5. (Aquí en tu proyecto real: descomprimir Huffman a un archivo)
 *   6. Verificar el archivo descomprimido contra la firma guardada
 *
 * Compilar:
 *   gcc -Wall -Wextra -std=c11 md5.c metadata.c ejemplo_flujo_completo.c -o ejemplo_flujo
 */

#include <stdio.h>
#include "metadata.h"

int main(void) {
   //Nombre del archivo, debe estar e la misma carpeta
    const char *archivo_original = "original.txt";
    const char *metadatos_cmb    = "salida.cmb";
    const char *archivo_salida   = "reconstruido.txt"; /* simula la descompresión */

    
    //FILE *f = fopen(archivo_original, "wb");
    //fputs("Hola mundo desde el compresor Huffman", f);
    //fclose(f);

    //Al comprimir

    //Firmar el archivo
    FileMetadata meta;
    if (firmar_archivo_original(archivo_original, "original.txt", &meta) != 0) {
        fprintf(stderr, "Error: no se pudo firmar el archivo\n");
        return 1;
    }

    //Cadena para guaradar el hash, el ultimo es el salto d elinea
    char hex[33];
    md5_to_hex(meta.md5, hex);
    printf("[Compresor] Archivo: %s\n", meta.filename);
    printf("[Compresor] Tamano original: %llu bytes\n",
           (unsigned long long)meta.original_size);
    printf("[Compresor] MD5: %s\n", hex);

    //Realizar la compresion y guardar el tamano
    meta.compressed_size = 30; /* valor de ejemplo */

    FILE *out = fopen(metadatos_cmb, "wb");
    escribir_metadata(out, &meta);
    /* Aquí escribirías los bytes comprimidos de Huffman justo después */
    fclose(out);
    //printf("[Compresor] Metadatos escritos en %s\n\n", metadatos_cmb);




    // Al descomprimir

    FileMetadata meta_leida;
    FILE *in = fopen(metadatos_cmb, "rb");
    leer_metadata(in, &meta_leida);
    /* Aquí leerías y descomprimirías los bytes de Huffman que siguen */
    fclose(in);

    /* Simulamos la reconstrucción del archivo original (en tu proyecto
     * real esto sale de tu descompresor Huffman, no de una copia) */
    //FILE *fo = fopen(archivo_salida, "wb");
    //fputs("Hola mundo desde el compresor Huffman MKJXCHJKSXVCJSKXCNKSB BSNDCJ", fo);
    //fclose(fo);

    int ok = verificar_archivo_descomprimido(archivo_salida, &meta_leida);
    //printf("[Descompresor] Verificando %s contra la firma guardada...\n",
      //     archivo_salida);
    //printf("[Descompresor] Resultado: %s\n",
         //  ok ? "OK (firma coincide, compresion exitosa)"
           //   : "FALLO (firma NO coincide)");

    return ok ? 0 : 1;
}
