
#include "buffer.h"

#include <stdint.h>

/*
Structure des buffers: 
il contiennent des sequences n, c_1, ..., c_n, n avec:
 - n le nombre de caractères sur la lignes
 - c_1, ..., c_n les caractère, à compléter par des espaces

il se lisent entre end et start (à l'envers)
Ainsi le buffer suivant
 n, a_n, ..., a_1, n, m, b_m, ... m ... p, z_p, ..., z_1, p  
 ^            ^                                           ^
 end                                                      start

represente:
z_1 ... z_p   
...
b_1 ... b_m
a_1 ... a_n
*/

buffer_t buffer_new(uint16_t *table, uint16_t taille) {
    buffer_t n;
    n.table = table;
    n.size = taille;
    n.start = 0;
    n.end = 0;
    return n;
}

static inline uint16_t mod(uint16_t a, uint16_t b) {
    return ((a % b) + b) % b; // reste euclidien
}

uint16_t buffer_free_space(buffer_t buf) {
    if (buf.end <= buf.start)
        return buf.end + buf.size - buf.start - 1;
    return buf.end - buf.start - 1;
}

void buffer_free(buffer_t buf) { // frees last block
    uint16_t p = buf.table[buf.end];
    buf.end = mod(buf.end + p + 2, buf.size);
}

// adds n first values of source to buffet
void buffer_add(buffer_t buf, const uint16_t *source, const uint16_t n) {
    while (buffer_free_space(buf) < n && !buffer_isempty(buf))
        buffer_free(buf);
    buf.table[mod(++buf.start, buf.size)] = n;
    for (uint8_t x = 0; x != n; ++x)
        buf.table[mod(++buf.start, buf.size)] = source[x];
    buf.table[mod(++buf.start, buf.size)] = n;
    buf.start = mod(++buf.start, buf.size);
}
// copies n top elements of buffer to dest
uint16_t buffer_pull(buffer_t buf, uint16_t *dest) {
    uint16_t n = buf.table[buf.start];
    for (uint16_t x = 0; x != n; ++x)
        dest[x] = buf.table[mod(buf.start - n - 1 + x, buf.size)];
    buf.start = mod(buf.start - n - 2, buf.size);
    return n;
}

char buffer_isempty(const buffer_t buf) {
    return buf.start == buf.end;
}