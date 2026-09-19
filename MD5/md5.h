/*
 * Basada en la descripción del algoritmo publicada en:
 * Rivest, R. (1992). The MD5 Message-Digest Algorithm (RFC 1321).
 * IETF. https://www.rfc-editor.org/info/rfc1321/
 *
 * Esta es una implementación propia escrita a partir de la especificación
 * del algoritmo (secciones 3.1 a 3.5 del RFC), NO una copia del código de
 * referencia del Apéndice A (RSAREF), para evitar los requisitos de
 * licencia/atribución de ese código de ejemplo.
 */

#ifndef MD5_H
#define MD5_H

#include <stddef.h>
#include <stdint.h>

#define MD5_DIGEST_SIZE 16   //128 bits
#define MD5_BLOCK_SIZE  64   // 512 bits

typedef struct {
    uint32_t state[4];        /* buffer A, B, C, D (Sección 3.3) */
    uint64_t bit_count;       /* longitud del mensaje en bits, para el padding */
    unsigned char buffer[MD5_BLOCK_SIZE]; /* bloque parcial acumulado */
    size_t buffer_len;        /* bytes válidos actualmente en buffer[] */
} MD5_CTX;

/* Inicializa el contexto (Sección 3.3: Initialize MD Buffer) */
void md5_init(MD5_CTX *ctx);

/* Alimenta 'len' bytes de datos al cálculo. Se puede llamar varias veces
 * seguidas para procesar un archivo en pedazos (streaming), sin necesidad
 * de tenerlo completo en memoria. */
void md5_update(MD5_CTX *ctx, const unsigned char *data, size_t len);

/* Cierra el cálculo: aplica el padding (Sección 3.1 y 3.2) y escribe
 * el digest final de 16 bytes en 'digest'. */
void md5_final(MD5_CTX *ctx, unsigned char digest[MD5_DIGEST_SIZE]);

/* Función de conveniencia: calcula el MD5 de un buffer completo en memoria. */
void md5_buffer(const unsigned char *data, size_t len,
                 unsigned char digest[MD5_DIGEST_SIZE]);

/* Función de conveniencia: calcula el MD5 de un archivo en disco, leyéndolo
 * por bloques (no carga el archivo completo en memoria).
 * Retorna 0 en éxito, -1 si no pudo abrir el archivo. */
int md5_file(const char *path, unsigned char digest[MD5_DIGEST_SIZE]);

/* Convierte un digest de 16 bytes a su representación hexadecimal de
 * 32 caracteres + '\0' (33 bytes en 'hex_out'). Útil para imprimir o
 * comparar firmas de forma legible. */
void md5_to_hex(const unsigned char digest[MD5_DIGEST_SIZE], char hex_out[33]);

#endif
