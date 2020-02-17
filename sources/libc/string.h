
#ifndef STRING_H
#define STRING_H


extern char * empty_str;

char str_equal(char *a, char *b);
unsigned int str_length(char *a);
char *str_copy(char *a); // mallocs new string, use free

#endif