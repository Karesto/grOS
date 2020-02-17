/*
Gere les fichiers dossier coté utilisateur
structure du systeme de fichiers:
                   utilisateur 
                      |
                   chemin (char *), contenu (char *), dossier et fichiers
libc/fileio           |
                   addresse disques (uint16_t), contenu (char *), que des fichiers
kernel/filesystem     |
                   blocs formatés (1 par secteur, 512 char/bloc), doublement chainés
drivers/disk          |
                   disque dur

Contenu d'un dossier:
<taille:64> <nb fichier:16> <liste de fichiers> (contenant . et ..)
fichier: <propio:9> <permissions:6> <is_file:1> <bloc debut:16> <bloc fin:16> <size:64> <nom:char*>
 - is_file (0 dossier/1 fichier)
 - proprio: identifiant unique, 0 pour root 
 - permission de la forme
    - 5-3 owner (rwx)
    - 2-0 other (rwx)
 - bloc debut (non-nul)
 - bloc fin   (non-nul, utile pour append)
 - size, taille en octet
 - nom, char* terminé par 0
Par simplicité les sous-dossiers sont traité comme des fichiers, 
mais les champs fin et taille sont sans signification, pour éviter de réécrire
tout l'arbre lors d'une modification. La taille est stocké dans l'entête de dossier.

Le dossier root est dans le bloc 1.
les blocs libres sont doublement chainés les uns à la suite des autres, 
le bloc 0 contient les addresses du premier et dernier bloc libre

Les procédures (mkdir, rm, copy...) renvoie des char indiquant leur effet:
 - 0 pour succès
 - 1 pour fichier non trouvé/ chemin invalide
 - 2 permission refusée
les autres num dépendent de la procédure
*/

#ifndef FILEIO_H
#define FILEIO_H

#include <stdint.h>

typedef struct {
    char *path; //parent path, true path is path + "/" + name
    char *name;
    uint8_t permissions;
    uint8_t is_file; // 0 for directory, 1 for file
    uint16_t owner;
    uint16_t addr_start;
    uint16_t addr_end;
    uint64_t size; // en octet, necessaire pour la lecture d'un fichier
} file_t;

typedef struct {
    char *path; //actual dir path
    uint16_t addr;
    uint16_t size; // en nombre de fichiers
    file_t *contents;
} directory_t;

typedef struct {
    uint64_t size;
    char *contents;
} file_contents_t;

directory_t ROOT;
directory_t CWD;

file_t * info(char *);

directory_t ls(char *);
void cd(char *);
char *getcwd();

char exists(char *);
char is_file(char *); // 1 fichier, 0 dossier ou inexistant

char rm(char *); // returns 3 for tried to remove a non-empty directory
char cp(char * source, char * dest); // 0 ok - 1 error when reading - 2 err when writing
char mv(char * source, char * dest);

char mkdir(char *); // cree dernier dossier
char mkfile(char *); // 0 ok, 1 invalid path

file_contents_t readfile(char *); // returns 0 of directory or not found
// Both return 3 if writing to directory
char append(char *path, char *contents, uint64_t size);
char write(char *path, char *contents, uint64_t size);

#endif