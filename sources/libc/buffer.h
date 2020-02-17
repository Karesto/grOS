/*
Defini un objet buffer (tableau circulaire pour stocker des uint16_t)
*/

#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

typedef struct {
    uint16_t start;
    uint16_t end;
    uint16_t *table; // doit déjà etre alloué
    uint16_t size;
} buffer_t;

buffer_t buffer_new(uint16_t *table, uint16_t taille);

// adds n first values of source to buffet
void buffer_add(buffer_t, const uint16_t *source, const uint16_t n);
// copies n top elements of buffer to dest
uint16_t buffer_pull(buffer_t, uint16_t *dest);

char buffer_isempty(const buffer_t);

#endif
