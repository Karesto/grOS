/* 
Pilote clavier, traduit l'entree en valeurs 32-bit suivantes

bit 31 : 0 si la touche est appuyer, 1 si elle est relacher
bit 30 : is_ascii, indique si les bits 0-7 sont une representation ascii
bit 29 : not_is_typed, vrai si Ctrl, Alt, ou combinaison Maj + Alt Gr...

bit 26 : Ctrl        (vrai/faux global, depend de gauche et droit)
bit 25 : Maj         (vrai/faux global, depend de gauche, droit et caps lock)
bit 24 : Arret Defil (non-implementé)
bit 23 : Verr Num    (activé 1 / desactivé 0, activé par défault)
bit 22 : Alt Gr      (appuyé 1 / relaché 0)
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
   26 -> Backspace
   27 -> Suppr
   28 -> Inser
   29 -> Tab
   30 -> Entree
   31 -> Entree Pav Num
   32 -> Espace

   40 -> Flch H
   41 -> Flch G
   42 -> Flch D
   43 -> Flch B
   44 -> Pg U
   45 -> Pg B
   46 -> Home

  255 -> Non defini
*/

#include "keyboard.h"
#include "ports.h"
#include "../cpu/interrupts.h"
#include "printer.h"

#include <stdint.h>

#define ignore(x) (void)(x) //enleve un warning, pas d'autre interet...

static uint16_t modifiers;
/* 
bit 0  -> Maj Gauche
bit 1  -> Maj Droit
bit 2  -> Caps Lock
bit 3  -> Ctrl Gauche
bit 4  -> Ctrl Droit
bit 5  -> Alt Gauche
bit 6  -> Alt Gr
bit 7  -> Ver Num
bit 8  -> Arret Defil
bit 9  -> Maj (global)
bit 10 -> Ctrl (global)
bit 11 -> Accent (vrai si ^/¨/`/~ pressé)
bit 12 -> e0 lu, attent la suite
*/

/*
infos sur le clavier:
pos = scancode modifié en remplacant le bit de poids fort par 
l'indication de présence de séquence (e0 lu)

bit 0 -7 : ligne (0-3), colonne (4-7)
bit 8 -15: caractère (si défini)
bit 16-23: caractère majuscule (si défini)
bit 24-31: caractère alt gr (si défini)
*/

unsigned char a = 'é';
static const uint32_t azerty[] =
 {0xff, 0x0, 0x0, 0x0,     0x00, 0x0, 0x0, 0x0,     0x21, '&', '1', 0x0,     
  0x22, 0x82, '2', 0x0,     0x23, '"', '3', '#',     0x24, '\'', '4', '{',     
  0x25, '(', '5', '[',     0x26, '-', '6', '|',     0x27, 'è', '7', '`',     
  0x28, '_', '8', '\\',    0x29, 'ç', '9', '^',     0x2a, 'à', '0', '@',     
  0x2b, ')', '°', ']',     0x2c, '=', '+', '}',     0x2d, 0x0, 0x0, 0x0,     
  0x40, 0x0, 0x0, 0x0,     0x41, 'a', 'A', 0x0,     0x42, 'z', 'Z', 0x0,     
  0x43, 'e', 'E', 0x0,     0x44, 'r', 'R', 0x0,     0x45, 't', 'T', 0x0,     
  0x46, 'y', 'Y', 0x0,     0x47, 'u', 'U', 0x0,     0x48, 'i', 'I', 0x0,     
  0x49, 'o', 'O', 0x0,     0x4a, 'p', 'P', 0x0,     0x4b, '^', '¨', 0x0,     
  0x4c, '$', '£', '¤',     0x4d, 0x0, 0x0, 0x0,     0xa0, 0x0, 0x0, 0x0,     
  0x61, 'q', 'Q', 0x0,     0x62, 's', 'S', 0x0,     0x63, 'd', 'D', 0x0,     
  0x64, 'f', 'F', 0x0,     0x65, 'g', 'G', 0x0,     0x66, 'h', 'H', 0x0,     
  0x67, 'j', 'J', 0x0,     0x68, 'k', 'K', 0x0,     0x69, 'l', 'L', 0x0,     
  0x6a, 'm', 'M', 0x0,     0x6b, 'ù', '%', 0x0,     0x20, '²', 0x0, 0x0,     
  0x80, 0x0, 0x0, 0x0,     0x6c, '*', 'µ', 0x0,     0x82, 'w', 'W', 0x0,     
  0x83, 'x', 'X', 0x0,     0x84, 'c', 'C', 0x0,     0x85, 'v', 'V', 0x0,     
  0x86, 'b', 'B', 0x0,     0x87, 'n', 'N', 0x0,     0x88, ',', '?', 0x0,     
  0x89, ';', '.', 0x0,     0x8a, ':', '/', 0x0,     0x8b, '!', '§', 0x0,     
  0x8c, 0x0, 0x0, 0x0,     0x33, '*', 0x0, 0x0,     0xa3, 0x0, 0x0, 0x0,     
  0xa4, 0x0, 0x0, 0x0,     0x60, 0x0, 0x0, 0x0,     0x01, 0x0, 0x0, 0x0,     
  0x02, 0x0, 0x0, 0x0,     0x03, 0x0, 0x0, 0x0,     0x04, 0x0, 0x0, 0x0,     
  0x05, 0x0, 0x0, 0x0,     0x06, 0x0, 0x0, 0x0,     0x07, 0x0, 0x0, 0x0,     
  0x08, 0x0, 0x0, 0x0,     0x09, 0x0, 0x0, 0x0,     0x0a, 0x0, 0x0, 0x0,     
  0x31, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0x51, '7', 0x0, 0x0,     
  0x52, '8', 0x0, 0x0,     0x53, '9', 0x0, 0x0,     0x34, '-', 0x0, 0x0,     
  0x71, '4', 0x0, 0x0,     0x72, '5', 0x0, 0x0,     0x73, '6', 0x0, 0x0,     
  0x54, '+', 0x0, 0x0,     0x91, '1', 0x0, 0x0,     0x92, '2', 0x0, 0x0,     
  0x93, '3', 0x0, 0x0,     0xb1, '0', 0x0, 0x0,     0xb3, '.', 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0x81, '<', '>', 0x0,     
  0x0b, 0x0, 0x0, 0x0,     0x0c, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0x94, 0x0, 0x0, 0x0,     0xa6, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0x32, '/', 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xa5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0x2f, 0x0, 0x0, 0x0,     0x8f, 0x0, 0x0, 0x0,     
  0x30, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xae, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xb0, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xaf, 0x0, 0x0, 0x0,     0x50, 0x0, 0x0, 0x0,     
  0x2e, 0x0, 0x0, 0x0,     0x4e, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     0xf5, 0x0, 0x0, 0x0,     
  0xf5, 0x0, 0x0, 0x0};

char special(uint8_t position) {
  switch (position) {
    case 0xff: return   0; // Error
    case 0x01: return   1; // F1
    case 0x02: return   2; // F2
    case 0x03: return   3; // F3
    case 0x04: return   4; // F4
    case 0x05: return   5; // F5
    case 0x06: return   6; // F6
    case 0x07: return   7; // F7
    case 0x08: return   8; // F8
    case 0x09: return   9; // F9
    case 0x0a: return  10; // F10
    case 0x0b: return  11; // F11
    case 0x0c: return  12; // F12
    case 0x31: return  13; // Verr Num
    case 0x60: return  14; // Verr Maj
    case 0xa0: return  15; // Ctrl G
    case 0xa6: return  16; // Ctrl D
    case 0x80: return  17; // Maj G
    case 0x8c: return  18; // Maj D
    case 0xa3: return  19; // Alt G
    case 0xa5: return  20; // Alt Gr
    case 0x00: return  25; // Esc
    case 0x2d: return  26; // Backspace
    case 0x4e: return  27; // Suppr
    case 0x2e: return  28; // Inser
    case 0x40: return  29; // Tab
    case 0x4d: return  30; // Entree
    case 0x94: return  31; // Entree Pav Num
    case 0xa4: return  32; // Espace
    case 0x8f: return  40; // Flch H
    case 0xae: return  41; // Flch G
    case 0xb0: return  42; // Flch D
    case 0xaf: return  43; // Flch B
    case 0x30: return  44; // Pg U
    case 0x50: return  45; // Pg B
    case 0x2f: return  46; // Home
    default  : return 255; // Non-defini
  }
}

static inline void set_modifiers(uint8_t pos, uint8_t is_press) {
    if (pos == 2 || pos == 7) { // Verr Maj et Verr Num
        if (is_press)
            modifiers ^= 1 << pos;
    }
    else {
        modifiers &= (0xffff ^ (1 << pos)); // on enleve le flag actuel
        modifiers |= (is_press << pos);
    }
}

void print_key(uint32_t code)
{
    if (code & (1 << 31))
        return ; // key release, ignore
    if (code & (1 << 30)) { // is_ascii
        if (code & (1 << 29)) { // combinaison
            if (code & (1 << 25))
                kprintf("Ctrl-");
            if (code & (1 << 24))
                kprintf("Maj-");
            if (code & (1 << 22))
                kprintf("AltGr-");
            if (code & (1 << 21))
                kprintf("Alt-");                
        }
        kprintf("%c", (uint8_t)code);
    }
    else { 
      switch ((uint8_t)code) {
        case 0: return kprintf("Error");
        case 1: return kprintf("F1");
        case 2: return kprintf("F2");
        case 3: return kprintf("F3");
        case 4: return kprintf("F4");
        case 5: return kprintf("F5");
        case 6: return kprintf("F6");
        case 7: return kprintf("F7");
        case 8: return kprintf("F8");
        case 9: return kprintf("F9");
        case 10: return kprintf("F10");
        case 11: return kprintf("F11");
        case 12: return kprintf("F12");
        case 13: return kprintf("Verr Num");
        case 14: return kprintf("Verr Maj");
        case 15: return kprintf("Ctrl G");
        case 16: return kprintf("Ctrl D");
        case 17: return kprintf("Maj G");
        case 18: return kprintf("Maj D");
        case 19: return kprintf("Alt G");
        case 20: return kprintf("Alt Gr");
        case 25: return kprintf("Esc");
        case 26: return kprintf("Backspace");
        case 27: return kprintf("Suppr");
        case 28: return kprintf("Inser");
        case 29: return kprintf("Tab");
        case 30: return kprintf("Entree");
        case 31: return kprintf("Entree Pav Num");
        case 32: return kprintf("Espace");
        case 40: return kprintf("Flch H");
        case 41: return kprintf("Flch G");
        case 42: return kprintf("Flch D");
        case 43: return kprintf("Flch B");
        case 44: return kprintf("Pg U");
        case 45: return kprintf("Pg B");
        case 46: return kprintf("Home");
        default: return kprintf("Unknown");
      }
    }
}

static void keyboard_callback(registers_t regs) {
    /* The PIC leaves us the scancode in port 0x60 */
    ignore(regs);
    uint8_t scancode = port_byte_in(0x60);
    if (scancode == 0xE0) {
        if (modifiers & (1 << 12)) {
            //return; //error
        }
        modifiers |= 1 << 12; // sets bit 13
        return;               // waits for next
    }
    if (scancode == 0) {
        return; //error
    }
    uint32_t key = 0;
    char is_press = 1;
    if (scancode & 0x80) {// cle relaché
        key = (1 << 31);
        is_press = 0;
        scancode &= 0x7f; // supprime le flag 0x80
    }
    if (modifiers & (1 << 12)) {// e0 pressé avant ?
        scancode |= 0x80;       // on rajoute l'info en utilisant le flag liberé
    }
    // mise à jour de modifiers:
    switch (scancode) {
        case 0x2a: // 0 Maj Gauche
            set_modifiers(0, is_press);
            break; 
        case 0x36: // 1 Maj Droit
            set_modifiers(1, is_press);
            break; 
        case 0x3a: // 2 Verr Maj
            set_modifiers(2, is_press);
            break; 
        case 0x1d: // 3 Ctrl Gauche
            set_modifiers(3, is_press);
            break; 
        case 0x9d: // 4 Ctrl Droit
            set_modifiers(4, is_press);
            break; 
        case 0x38: // 5 Alt Gauche
            set_modifiers(5, is_press);
            break; 
        case 0xb8: // 6 Alt Gr
            set_modifiers(6, is_press);
            break; 
        case 0x45: // 7 Verr Num
            set_modifiers(7, is_press);
            break;
    }
    set_modifiers(8, ((modifiers & (1 << 2)) != 0) ^ ((modifiers & 0b11) != 0)); // Maj global
    set_modifiers(9, (modifiers & 0b11000) != 0); //Ctrl global
    uint16_t base = ((uint16_t)4) * ((uint16_t)scancode);
    //uint8_t key_val = azerty[base];
    key |= (modifiers & 0x3ff) << 16; // copie de modificateurs (maj, alt, ctrl...)
    key |= (azerty[base]) << 8;       // copie ligne/colonne
    char offset = 1;                  // 1 normal, 2 maj, 3 alt gr
    if (((modifiers & 0b1000100000) != 0) // Ctrl or Alt
        | (((modifiers & (1 << 9)) != 0) & ((modifiers & (1 << 6)) != 0))) // maj et alt gr simultané
        key |= (1 << 29);
    else if (modifiers & (1 << 8))
        offset = 2; // maj
    else if (modifiers & (1 << 6))
        offset = 2; // alt gr
    char a = azerty[base + offset];
    if (a == 0 && base != 1) // pas de caractere Maj ou Alt Gr, on prend celui par defaut
        a = azerty[base + 1];
    if (a == 0) // valeur nul, pas de caractere ascii
        key |= special(azerty[base]); // descriptif de caractere special
    else {
        key |= a; // on rajoute le caractere ascii en bout
        key |= (1 << 30);
    }
    
    //kprintf("0x%02x ", scancode);
    print_key(key);
    //kprintf("\n");
    //print_letter(scancode);
    modifiers &= 0b1110011111111111; // resets accent and e0 flag
}

void init_keyboard() {
    register_interrupt_handler(IRQ1, keyboard_callback); 
}