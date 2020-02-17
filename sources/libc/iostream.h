/*
Module printer, gère l'impression sur le terminal.

Caractères reconnus par kprintf
Entier:
  %d -> entier decimal
  %o -> entier octal
  %x -> entier hexadecimal
  %b -> entier binaire
  Ils peuvent être prefixé par 
   - %ud pour unsigned
   - %ld pour long (64 bits)
   - un nombre pour le padding:
      - printf(%5d, 1)  imprime '    1'
      - printf(%05d, 1) imprime '00001' (observer le zero précédant le nombre)
  Ces valeurs sont combinables, mais le nombre doit être en première positions:
   - kprintf("%05ulx", ...) imprime en entier non-signé 64-bits 
                           sur au moins 5 caractères en hexadécimal
Caractères:                           
  %s -> chaine de caracteres (char *)
  %c -> caractere
  Ils peuvent également être précédés d'un nombre, indiquant le nombre 
  de répétitions de la chaine ou du caractère:
   - printf("%12c", ' ') imprime 12 espaces
Autres:
  %% -> symbole %



Caractères traités spécialement par le terminal
\n \r -> retour a la ligne
\t -> saut au prochain multiple de 4


*/

#ifndef IOSTREAM_H
#define IOSTREAM_H


#include <stdint.h>

void kprintf(const char *, ...);

void key_pressed(uint32_t keycode); // appele lors d'une interruption clavier

char *readline(); 
// pause l'execution en attendant que l'utilisateur appuie sur entree
// renvoie tous les caractères tapés entre l'appel et l'appui sur entrée
// lis une ligne (ignore les caractere precedent, max 800 caracteres)
// renvoie le buffer, sera ecrasé au prochain appel
#endif