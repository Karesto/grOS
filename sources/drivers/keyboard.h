/* 
Pilote clavier, traduit l'entree en valeurs 32-bit suivantes

bit 31 : 0 si la touche est appuyer, 1 si elle est relacher
bit 30 : is_ascii, indique si les bits 0-7 sont une representation ascii
bit 29 : not_is_typed, vrai si Ctrl, Alt, ou combinaison Maj + Alt Gr...

bit 27 : Alt Gr      (vrai/faux global, depend de Alt D ou Ctrl+Alt G)
bit 26 : Ctrl        (vrai/faux global, depend de gauche et droit)
bit 25 : Maj         (vrai/faux global, depend de gauche, droit et caps lock)
bit 24 : Arret Defil (non-implementé)
bit 23 : Verr Num    (activé 1 / desactivé 0, activé par défault)
bit 22 : Alt Droit   (appuyé 1 / relaché 0)
bit 21 : Alt Gauche  (appuyé 1 / relaché 0) 
bit 20 : Ctrl Droit  (appuyé 1 / relaché 0)
bit 19 : Ctrl Gauche (appuyé 1 / relaché 0)
bit 18 : Caps Lock   (activé 1 / desactivé 0, desactivé par défault)
bit 17 : Maj Droit   (appuyé 1 / relaché 0)
bit 16 : Maj Gauche  (appuyé 1 / relaché 0)

bit 15 - 13 : numero de ligne   (0 (haut) - 6 bas, 7 pour non-def)
bit 12 - 8  : numero de colonne (0 (gauche) - 20 (droite), 21 pour non def)
            le pave num est entre 17 - 20
bit 7  - 0  : 
    si bit 30 = 1 -> representation ascii
    sinon         -> code touche speciale

les codes de touches speciales sont les suivants:
    0 -> Error
    1 -> F1
    2 -> F2
    3 -> F3
    4 -> F4
    5 -> F5
    6 -> F6
    7 -> F7
    8 -> F8
    9 -> F9
   10 -> F10
   11 -> F11
   12 -> F12

   13 -> Verr Num
   14 -> Verr Maj
   15 -> Ctrl G
   16 -> Ctrl D
   17 -> Maj G
   18 -> Maj D
   19 -> Alt G
   20 -> Alt Gr

   25 -> Esc
   26 -> Retour
   27 -> Suppr
   28 -> Inser
   29 -> Tab
   30 -> Entree
   32 -> Espace

   40 -> Flch H
   41 -> Flch G
   42 -> Flch D
   43 -> Flch B
   44 -> Pg U
   45 -> Pg B
   46 -> Debut
   47 -> Fin

  255 -> Non defini
*/


#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

// fonction associant les interruptions clavier au code C
void init_keyboard();

//lecture des codes de touche
uint8_t keycode_pos(uint32_t keycode);
uint8_t keycode_row(uint32_t keycode);
uint8_t keycode_column(uint32_t keycode);
void keycode_print(uint32_t keycode);

char keycode_ispressed(uint32_t keycode); // vrai si appuyé, faux si relaché
char keycode_isascii(uint32_t keycode);


// 0 pour qwerty, 1 pour azerty
void keyboard_set(char mode); 
char keyboard_get();

// renvoie le code decrivant la touche correpondant a pos
char special(uint8_t pos);


#endif
