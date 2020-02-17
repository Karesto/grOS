/* Contient un objet de type liste chainée de str */

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct linked_list
{
	char *hd;
	struct linked_list* tl;
} linked_list;

void ll_free(struct linked_list *l);
// free memory
void ll_print(struct linked_list *l, char sep); 
// prints the list separated by sep

unsigned int ll_length(struct linked_list *l); // number of elements in list
unsigned int ll_length_concat(struct linked_list *l); 
// concatenated length of all strings in ll, includes trailing 0

struct linked_list * ll_split(char string[], char sep);
// splits string into list according to sep
char *ll_stitch(struct linked_list *l, char sep);
// operation inverse

struct linked_list *ll_purge(struct linked_list *);
//removes leading empty elems in list

#endif