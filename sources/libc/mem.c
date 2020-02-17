#include "mem.h"

/*gestion de la mémoire hyper basique, on a seulement un malloc primitif
pour l'instant, une fonction de copie bourrin et pas de free.
C'est moche, mais on fera avec pour l'instant.*/

/* La fonction memcopy permet de copier n octets de src vers dest.
 * Les adresses sont lineaires.
 */
char * memcpy(char *dest, char *source, int nbytes)
{
	char * p = dest;
	while (nbytes--){
		*dest++ = *source++;
	}
	return p;
}

void set(uint8_t *dest, uint8_t val, int nbytes)
{
	int i;
	for(i=0; i < nbytes; i++){
		*(dest + i) = val;
	}
}

uint32_t fmem_addr = 0x10000;
uint32_t start_of_mem = 0x10000;
uint32_t end_of_mem = 0x10000;
/*point de départ arbitraire*/

footer_t *get_footer(header_t* h) {
	return h->size + sizeof(header_t);
}

header_t *next_header(header_t* h) {
	return h->size + sizeof(header_t) + sizeof(footer_t);
}

header_t *prev_header(header_t* h) {
	return (h>start_of_mem) ? ((footer_t *)(h-sizeof(footer_t)))->header : -1;
}

void free_header(header_t * h) {
	header_t *next_h = next_header(h);
	header_t *prev_h = prev_header(h);

	h->is_hole = 1;

	if (next_h < end_of_mem && next_h->is_hole)
	{
		h->size += sizeof(header_t) + sizeof(footer_t) + next_h->size;
		get_footer(h)->header = h;
	}
	else if (next_h == end_of_mem) {end_of_mem = h;}

	if(prev_h != -1 && prev_h->is_hole) {free_header(prev_h);}
}

uint32_t find_hole(uint32_t size) {
	header_t* i = start_of_mem;
	while(i < end_of_mem) {
		if (i->is_hole && i->size >= size) {
			i->is_hole = 0;

			if (i->size > size + sizeof(header_t) + sizeof(footer_t)) {
				uint32_t tmp = i->size;
				i->size = size;
				header_t * j = next_header(i);
				j->size = tmp - sizeof(header_t) - sizeof(footer_t);
				get_footer(j)->header = j;
				free_header(j);
			}
			return i;
		}
		i += sizeof(header_t) + i->size;
	}
	return -1;
}

void * malloc(uint32_t size) {
	uint32_t ho = find_hole(size);
	if (ho == -1) {
		header_t * h = end_of_mem;
		end_of_mem += sizeof(header_t) + sizeof(footer_t) + size;
		h->is_hole = 0;
		h->size = size;
		get_footer(h)->header = h;
		return h + sizeof(header_t);
	}
	return ho + sizeof(header_t);
}

void free(void* p) {
	free_header(p - sizeof(header_t));
}

uint32_t kmalloc_base(uint32_t size, char align, uint32_t *p_addr){
	/*pages alignées sur 4K, donc 0x1000*/
	if (align && (fmem_addr & 0xFFFFF000)){
		fmem_addr &= 0xFFFFF000;
		fmem_addr += 0x1000;
	}

	/*sauvegarde de l'adresse physique*/
	if (p_addr) *p_addr = fmem_addr;

	uint32_t res = fmem_addr;
	fmem_addr += size;
	return res;
}

uint32_t kmalloc_a(uint32_t size) {
	return kmalloc_base(size, 1, 0);
}

uint32_t kmalloc_p(uint32_t size) {
	uint32_t *res;
	kmalloc_base(size, 0, res);
	return *res;
}

uint32_t kmalloc_ap(uint32_t size) {
	uint32_t *res;
	kmalloc_base(size, 1, res);
	return *res;
}

uint32_t kmalloc(uint32_t size) {
	return kmalloc_base(size, 0, 0);
}