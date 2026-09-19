/*
 * metadata.h - Metadatos de archivos comprimidos (firma MD5 + tamaños)
 *
 * Este módulo conecta md5.c/md5.h con el formato del archivo comprimido:
 * antes de correr Huffman sobre un archivo, se calcula su firma MD5
 * (firmar_archivo_original). Al descomprimir, se recalcula la firma sobre
 * el archivo reconstruido y se compara contra la guardada
 * (verificar_archivo_descomprimido) para confirmar que la descompresión
 * fue exitosa.
 *
 * Todo el cálculo de MD5 se hace con md5_file(), que lee el archivo
 * directamente de disco por bloques, sin necesidad de cargarlo completo
 * en memoria.
 */

#ifndef METADATA_H
#define METADATA_H

#include <stdint.h>
#include <stdio.h>
#include "md5.h"

#pragma pack(push, 1) /* sin padding: el layout debe ser exacto al
                          escribirse/leerse tal cual del archivo .cmb */
typedef struct {
    char     filename[256];             /* nombre/ruta relativa original */
    uint64_t original_size;             /* tamaño en bytes antes de comprimir */
    uint64_t compressed_size;           /* tamaño del bloque Huffman (lo llena tu compresor) */
    unsigned char md5[MD5_DIGEST_SIZE]; /* firma MD5 del archivo original */
} FileMetadata;
#pragma pack(pop)

/* --- Lado del COMPRESOR ---
 *
 * Llamar ANTES de correr Huffman sobre el archivo. Lee 'filepath' del
 * disco, calcula su MD5 y llena 'meta' (incluyendo original_size).
 * 'filename' es el nombre que se guardará en los metadatos (por ejemplo,
 * la ruta relativa dentro del directorio que el usuario eligió comprimir).
 *
 * Retorna 0 en éxito, -1 si no se pudo leer el archivo.
 */
int firmar_archivo_original(const char *filepath, const char *filename,
                             FileMetadata *meta);

/* Escribe el header de metadatos en el archivo comprimido (.cmb), en la
 * posición actual de 'out'. Debe escribirse justo antes de los bytes
 * comprimidos de Huffman correspondientes a ese archivo.
 * Retorna 0 en éxito, -1 si falló la escritura. */
int escribir_metadata(FILE *out, const FileMetadata *meta);

/* Lee un header de metadatos desde la posición actual de 'in' (usado por
 * el descompresor para saber qué archivo viene y cuál es su firma).
 * Retorna 0 en éxito, -1 si falló la lectura. */
int leer_metadata(FILE *in, FileMetadata *meta);

/* --- Lado del DESCOMPRESOR ---
 *
 * Llamar DESPUÉS de expandir el archivo comprimido a 'filepath' en disco.
 * Recalcula el MD5 del archivo ya descomprimido y lo compara contra el
 * que venía guardado en 'meta'.
 *
 * Retorna 1 si la firma coincide (descompresión exitosa),
 * 0 si NO coincide o si el archivo no se pudo leer.
 */
int verificar_archivo_descomprimido(const char *filepath, const FileMetadata *meta);

#endif /* METADATA_H */
