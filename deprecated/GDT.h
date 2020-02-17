/* Module qui sert à créer, remplir et indiquer au CPU ou
se trouve la GDT (utilisation de la mémoire) */

#ifndef GDT_H
#define GDT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint64_t * get_GDT(void);
void set_GDT(uint64_t *gdt, size_t gdt_size);
void init_gdt(void);

#endif