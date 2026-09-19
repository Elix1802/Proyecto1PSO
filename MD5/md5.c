/*
 * md5.c - Implementación del algoritmo MD5
 *
 * Escrita a partir de la especificación algorítmica descrita en:
 * Rivest, R. (1992). The MD5 Message-Digest Algorithm (RFC 1321).
 * IETF. https://www.rfc-editor.org/info/rfc1321/
 *
 * Cubre los 5 pasos definidos en el RFC:
 *   1. Append Padding Bits   (Sección 3.1)
 *   2. Append Length         (Sección 3.2)
 *   3. Initialize MD Buffer  (Sección 3.3)
 *   4. Process Message in 16-Word Blocks (Sección 3.4)
 *   5. Output                (Sección 3.5)
 */

#include "md5.h"
#include <stdio.h>
#include <string.h>

/* --- Funciones auxiliares F, G, H, I (Sección 3.4) --- */
static uint32_t F(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~x & z); }
static uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return (x & z) | (y & ~z); }
static uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }
static uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return y ^ (x | ~z); }

static uint32_t rotate_left(uint32_t x, int s) {
    return (x << s) | (x >> (32 - s));
}

/* Tabla T[1..64], construida a partir de la función seno como indica el
 * RFC ("integer part of 4294967296 times abs(sin(i))"). Se deja precalculada
 * para no depender de <math.h> en tiempo de ejecución. */
static const uint32_t T[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

/* Cantidad de bits que rota cada operación por ronda (fijas por el RFC) */
static const int SHIFT[64] = {
    7,12,17,22, 7,12,17,22, 7,12,17,22, 7,12,17,22,
    5, 9,14,20, 5, 9,14,20, 5, 9,14,20, 5, 9,14,20,
    4,11,16,23, 4,11,16,23, 4,11,16,23, 4,11,16,23,
    6,10,15,21, 6,10,15,21, 6,10,15,21, 6,10,15,21
};

/* Orden en que se consume cada palabra del bloque X[k] en cada una de
 * las 64 operaciones (Sección 3.4, listas [ABCD k s i] de cada ronda) */
static const int WORD_ORDER[64] = {
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,       /* Ronda 1 */
     1, 6,11, 0, 5,10,15, 4, 9,14, 3, 8,13, 2, 7,12,       /* Ronda 2 */
     5, 8,11,14, 1, 4, 7,10,13, 0, 3, 6, 9,12,15, 2,       /* Ronda 3 */
     0, 7,14, 5,12, 3,10, 1, 8,15, 6,13, 4,11, 2, 9        /* Ronda 4 */
};

/* Procesa un único bloque de 64 bytes, actualizando ctx->state.
 * Corresponde al cuerpo del "for i = 0 to N/16-1" del RFC. */
static void md5_process_block(MD5_CTX *ctx, const unsigned char block[MD5_BLOCK_SIZE]) {
    uint32_t X[16];
    /* Decode: 4 bytes little-endian -> 1 palabra de 32 bits (Sección 2) */
    for (int j = 0; j < 16; j++) {
        X[j] = (uint32_t)block[j * 4]
             | ((uint32_t)block[j * 4 + 1] << 8)
             | ((uint32_t)block[j * 4 + 2] << 16)
             | ((uint32_t)block[j * 4 + 3] << 24);
    }

    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];

    for (int i = 0; i < 64; i++) {
        uint32_t f;
        if (i < 16)      f = F(b, c, d);
        else if (i < 32) f = G(b, c, d);
        else if (i < 48) f = H(b, c, d);
        else             f = I(b, c, d);

        uint32_t tmp = d;
        d = c;
        c = b;
        uint32_t sum = a + f + X[WORD_ORDER[i]] + T[i];
        b = b + rotate_left(sum, SHIFT[i]);
        a = tmp;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
}

void md5_init(MD5_CTX *ctx) {
    /* Constantes de inicialización (Sección 3.3) */
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
    ctx->bit_count = 0;
    ctx->buffer_len = 0;
}

void md5_update(MD5_CTX *ctx, const unsigned char *data, size_t len) {
    ctx->bit_count += (uint64_t)len * 8;

    size_t offset = 0;
    /* Si hay datos pendientes en el buffer, completarlo primero */
    if (ctx->buffer_len > 0) {
        size_t need = MD5_BLOCK_SIZE - ctx->buffer_len;
        size_t take = (len < need) ? len : need;
        memcpy(ctx->buffer + ctx->buffer_len, data, take);
        ctx->buffer_len += take;
        offset += take;

        if (ctx->buffer_len == MD5_BLOCK_SIZE) {
            md5_process_block(ctx, ctx->buffer);
            ctx->buffer_len = 0;
        }
    }

    /* Procesar bloques completos directamente desde 'data' */
    while (offset + MD5_BLOCK_SIZE <= len) {
        md5_process_block(ctx, data + offset);
        offset += MD5_BLOCK_SIZE;
    }

    /* Guardar el resto en el buffer para la próxima llamada */
    size_t remaining = len - offset;
    if (remaining > 0) {
        memcpy(ctx->buffer, data + offset, remaining);
        ctx->buffer_len = remaining;
    }
}

void md5_final(MD5_CTX *ctx, unsigned char digest[MD5_DIGEST_SIZE]) {
    uint64_t bit_count_le = ctx->bit_count; /* ya la guardamos en bits */

    /* Paso 1: agregar un bit '1' (byte 0x80) y ceros hasta llegar a
     * 56 bytes dentro del bloque de 64 (Sección 3.1) */
    unsigned char pad_start = 0x80;
    md5_update(ctx, &pad_start, 1);

    unsigned char zero = 0x00;
    while (ctx->buffer_len != 56) {
        md5_update(ctx, &zero, 1);
    }

    /* Paso 2: agregar la longitud original en bits, como 8 bytes
     * little-endian (Sección 3.2). Se escribe directo al buffer para
     * no volver a sumar al contador de bits. */
    unsigned char len_bytes[8];
    for (int i = 0; i < 8; i++) {
        len_bytes[i] = (unsigned char)((bit_count_le >> (8 * i)) & 0xff);
    }
    memcpy(ctx->buffer + 56, len_bytes, 8);
    ctx->buffer_len = 64;
    md5_process_block(ctx, ctx->buffer);
    ctx->buffer_len = 0;

    /* Paso 5: Output. Cada palabra de estado se vuelca en little-endian */
    for (int i = 0; i < 4; i++) {
        digest[i * 4]     = (unsigned char)(ctx->state[i] & 0xff);
        digest[i * 4 + 1] = (unsigned char)((ctx->state[i] >> 8) & 0xff);
        digest[i * 4 + 2] = (unsigned char)((ctx->state[i] >> 16) & 0xff);
        digest[i * 4 + 3] = (unsigned char)((ctx->state[i] >> 24) & 0xff);
    }
}

void md5_buffer(const unsigned char *data, size_t len,
                 unsigned char digest[MD5_DIGEST_SIZE]) {
    MD5_CTX ctx;
    md5_init(&ctx);
    md5_update(&ctx, data, len);
    md5_final(&ctx, digest);
}

int md5_file(const char *path, unsigned char digest[MD5_DIGEST_SIZE]) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    MD5_CTX ctx;
    md5_init(&ctx);

    unsigned char chunk[8192];
    size_t n;
    while ((n = fread(chunk, 1, sizeof(chunk), f)) > 0) {
        md5_update(&ctx, chunk, n);
    }

    fclose(f);
    md5_final(&ctx, digest);
    return 0;
}

void md5_to_hex(const unsigned char digest[MD5_DIGEST_SIZE], char hex_out[33]) {
    static const char hex_chars[] = "0123456789abcdef";
    for (int i = 0; i < MD5_DIGEST_SIZE; i++) {
        hex_out[i * 2]     = hex_chars[(digest[i] >> 4) & 0x0f];
        hex_out[i * 2 + 1] = hex_chars[digest[i] & 0x0f];
    }
    hex_out[32] = '\0';
}
