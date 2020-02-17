
#include "string.h"

#include "../libc/mem.h"

char* empty_str = "";

char str_equal(char * a, char * b) {
    int i = -1;
    do {
        ++i;
        if (a[i] == 0)
            return (b[i] == 0);
        if (b[i] == 0)
            return 0;
    } while (a[i] == b[i]);
    return 0;
}

unsigned int str_length(char * a) {
    unsigned int i = 0;
    while (a[i])
        ++i;
    return i;
}

char * str_copy(char * a) {
    unsigned int i = str_length(a);
    char *rep = malloc((i + 1) * sizeof(char));
    memcpy(a, rep, i+1); // copies last 0
    return rep;
}