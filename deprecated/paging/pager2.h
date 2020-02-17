#if ndef PAGING_H
#define PAGING_H

#include <stdint.h>

typedef struct page
{
	bool present; //vrai ssi la page est présente en mémoire
	bool rw; //faux ssi lecture seule
	bool user; //vrai ssi accessible par l'utilisateur
	bool accessed; //vrai ssi la page a été accédée depuis le dernier refresh
	bool unused[7]; //bits inutilisés et réservés
	uint32_t frame : 20; //Addresse de la frame, (décalée vers la droite de 12 bits)
} page_t;

typedef struct page_table
{
	page_t pages[1024];
} page_table_t;

typedef struct page_directory
{
	page_table_t *tables[1024];
	uint32_t tables_phys[1024];
	uint32_t p_addr;
} page_directory_t;

void init_paging();

void switch_page_directory(page_directory_t *new_dir);

page_t *get_page(uint32_t addr, int make, page_directory_t *dir);

void page_fault();