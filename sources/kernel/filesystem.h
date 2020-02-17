/*
Systeme de fichier de GrOS, traduit les fichiers en blocs chainés
structure du systeme de fichiers:
                   utilisateur 
                      |
                   chemin (char *), contenu (char *)
libc/fileio           |
                   addresse disques (uint16_t), contenu (char *)
kernel/filesystem     |
                   blocs formatés (1 par secteur, 512 char/bloc), doublement chainés
drivers/disk          |
                   disque dur

Les fichiers sont encodés en blocs (secteurs de 258 uint16_t)
Structure d'un bloc:
Les blocs sont addressés sur 16 bits (65 536 blocs max = 33 Mo)

8 premiers bits d'un bloc: infos bloc
 - bit 0 : 0 libre, 1 occupé
 - bit 1 : 0 dernier bloc, 1 il existe un bloc suivant
 - bit 2 : 0 premier bloc, 1 il existe un bloc precedent
 - bit 3-7: reservés

16 prochain bits: 
 - si occupé, bloc suivant si existant, sinon nombre de char du bloc utilisés (0 - 506)
 - si libre, bloc libre suivant (0 indique que tous les blocs suivant sont libres)

16 prochain bits:
 - si occupé, bloc précedent si existant, parent sinon
 - si libre, bloc libre précédent

Suivi du contenu...

*/

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>

uint16_t free_block(); // returns a new free bloc id

/*
ecrit le fichier contents[0, size[ sur le disque
renvoie secteur debut (16 bits hauts) et fin (16 bas)
*/
uint32_t file_new(uint16_t parent, uint64_t size, char* contents, char has_ancestor);
uint16_t file_write(uint16_t file, uint64_t size, char *contents); // ecrase et renvoi le dernier bloc
uint16_t file_append(uint16_t file, uint64_t size, char *contents); // file doit etre le dernier bloc, renvoie le nouveau dernier bloc
void file_delete(uint16_t file);
char * file_read(uint16_t file, uint64_t size); // returns 0 if size too small

#endif