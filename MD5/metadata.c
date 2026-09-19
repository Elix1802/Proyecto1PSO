/*
 * metadata.c - Implementación de firmar_archivo_original,
 * escribir_metadata, leer_metadata y verificar_archivo_descomprimido.
 */

#include "metadata.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int firmar_archivo_original(const char *filepath, const char *filename,
                             FileMetadata *meta) {
    memset(meta, 0, sizeof(*meta));

    /* Obtener el tamaño del archivo original */
    struct stat st;
    if (stat(filepath, &st) != 0) {
        return -1;
    }
    meta->original_size = (uint64_t)st.st_size;

    /* Calcular la firma MD5 leyendo el archivo directo de disco */
    if (md5_file(filepath, meta->md5) != 0) {
        return -1;
    }

    strncpy(meta->filename, filename, sizeof(meta->filename) - 1);
    /* compressed_size lo llena tu compresor después de correr Huffman */

    return 0;
}

int escribir_metadata(FILE *out, const FileMetadata *meta) {
    return fwrite(meta, sizeof(*meta), 1, out) == 1 ? 0 : -1;
}

int leer_metadata(FILE *in, FileMetadata *meta) {
    return fread(meta, sizeof(*meta), 1, in) == 1 ? 0 : -1;
}

int verificar_archivo_descomprimido(const char *filepath, const FileMetadata *meta) {
    unsigned char digest_actual[MD5_DIGEST_SIZE];

    if (md5_file(filepath, digest_actual) != 0) {
        return 0; /* no se pudo ni leer el archivo descomprimido */
    }

    return memcmp(digest_actual, meta->md5, MD5_DIGEST_SIZE) == 0;
}
