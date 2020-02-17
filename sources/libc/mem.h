#ifndef MEM_H
#define MEM_H

#include <stdint.h>

typedef struct 
{
	uint32_t size;
	uint8_t is_hole;
} header_t;

typedef struct
{
	header_t * header;
} footer_t;

char * memcpy(char *source, char *dest, int nbytes);
void set(uint8_t *dest, uint8_t val, int nbytes);
void * malloc(uint32_t sz);
void free(void* p);

uint32_t kmalloc_a(uint32_t sz);
uint32_t kmalloc_p(uint32_t sz);
uint32_t kmalloc_ap(uint32_t sz);
uint32_t kmalloc(uint32_t sz);

#endif