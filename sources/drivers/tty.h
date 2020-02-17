/* 
Gestion du terminal bas niveau, gère les couleurs, le curseur et 
quelques séquences pseudo-ANSI:

Le terminal reconnait les codes pseudo-ANSI suivant:
\033[nnf] -> couleur de premier plan nn (code VGA ci-dessus)
\033[nnb] -> couleur d'arrière plan nn (parmi les 8 première couleurs seulement!)

\033[nny] -> place le curseur sur la ligne nn entre 0 et 24
\033[nnx] -> place le curseur sur la colonne nn entre 0 et 79
\033[nnu] -> bouge le curseur vers le haut de nn (se bloque au sommet si depassement)
\033[nnd] -> bouge le curseur vers le bas de nn (se bloque en bas si depassement)
\033[nnr] -> bouge le curseur vers la droite de nn (se bloque a droite si depassement)
\033[nnl] -> bouge le curseur vers la gauche de nn (se bloque a gauche si depassement)

\033[:] -> efface l'écran et place le curseur en haut a gauche
\033[.] -> efface la ligne en cours et remet le curseur à 0.
\033[_] -> couleurs par défaut

ces codes sont cumulables:
ex printf("\033[04f00b.02v]")
   - couleur rouge sur fond noir
   - efface l'ecran
   - positionne le curseur sur la 3eme ligne
les caracteres sont reconnus après printf, donc
printf("\033[%df]", VGA_COLOR_RED) fonctionne comme on se l'attend.
*/

#ifndef TTY_H
#define TTY_H

#include <stddef.h>
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define NB_COLORS 16
#define BUFFER_SIZE 2000 // nb of terminal windows stored

/* Couleurs pour le terminal */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

void terminal_initialize(void);
uint16_t terminal_getcursor(); // row on first 8 bits, column on last 8
void terminal_movecursor(size_t row, size_t column);

void terminal_clearscreen(void);
void terminal_clearline(void);
void terminal_newline(void);

void terminal_setcolor(enum vga_color fg, enum vga_color bg);
void terminal_resetcolor(void); // white on black

void terminal_putchar(const char);

#endif
