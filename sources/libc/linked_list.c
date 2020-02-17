
#include "linked_list.h"

#include "../libc/string.h"
#include "../libc/mem.h"
#include "../libc/iostream.h"

void ll_free(struct linked_list *l) {
	if(l){
		ll_free(l->tl);
		free(l->hd);
        free(l);
    }
}

void ll_print(struct linked_list *l, char sep){
	if(l){
		kprintf("%s%c", *(l->hd), sep);
		ll_print(l->tl, sep);
	}
}

unsigned int ll_length(struct linked_list *l) {
	if (l)
		return 1 + ll_length(l->tl);
	return 0;
}

unsigned int ll_length_concat(struct linked_list *l) {
	if (l)
		return str_length(l->hd) + 1 + ll_length_concat(l->tl);
	return 0;
}

struct linked_list * ll_split(char string[], char sep){
	if (string[0] != 0){
		struct linked_list *res = malloc(sizeof(struct linked_list));
		if (string[0] == sep){
			res->hd = empty_str;
			res->tl = ll_split(&string[1], sep);
			return res;
		}
		int i = 1;
		while((string[i] != sep) && (string[i] != 0)){
			++i;
		}
		char *sub_string = malloc(i*sizeof(char));
		int j;
		for (j = 0; j < i; ++j) {
			sub_string[j] = string[j];
		}
		res->hd = sub_string;
		res->tl = ((string[i] == 0) ? 0 : ll_split(&string[i], sep));
		return res;
	}
	return 0;
}

char* ll_stitch(struct linked_list *l, char sep)
{
	if(l==0) return empty_str;
    unsigned int len = ll_length_concat(l);
    char* res = malloc(len*sizeof(char));
	int i = 0;
	while(l)
	{
		int j = str_length(l->hd);
        memcpy(l->hd, &res[i], j);
        i += j;
        res[i++] = sep;
		l = l->tl;
	}
	res[i-1] = 0; //last sep if 0
	return res;
}

struct linked_list* ll_purge(struct linked_list *l)
{
	if(l)
	{
		if(str_equal(l->hd, empty_str)) {
            //free(l->hd);
            free(l);
            return ll_purge(l->tl);
        }
	}
	return l;
}