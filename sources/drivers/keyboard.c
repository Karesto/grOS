/* 
Pilote clavier, voir keyboard.h pour la documentation
*/

#include "keyboard.h"

#include "../cpu/ports.h"
#include "../cpu/interrupts.h"
#include "../libc/iostream.h"

#include <stdint.h>

#define ignore(x) (void)(x) //enleve un warning, pas d'autre interet...

/*
infos sur le clavier:
pos = scancode modifié en remplacant le bit de poids fort par 
l'indication de présence de séquence (e0 lu)

bit 0 -7 : ligne (0-3), colonne (4-7)
bit 8 -15: caractère (si défini)
bit 16-23: caractère majuscule (si défini)
bit 24-31: caractère alt gr (si défini)
*/
static const uint32_t azerty[] =
 {0xff000000, 0x00000000, 0x21263100, 0x22e93200, 0x23223323, 0x2427347b, 
  0x2528355b, 0x262d367c, 0x27e83760, 0x285f385c, 0x29e7395e, 0x2ae03040, 
  0x2b29b05d, 0x2c3d2b7d, 0x2d000000, 0x40000000, 0x41614100, 0x427a5a00, 
  0x43654500, 0x44725200, 0x45745400, 0x46795900, 0x47755500, 0x48694900, 
  0x496f4f00, 0x4a705000, 0x4b5ea800, 0x4c24a3a4, 0x4d000000, 0xa0000000, 
  0x61715100, 0x62735300, 0x63644400, 0x64664600, 0x65674700, 0x66684800, 
  0x676a4a00, 0x686b4b00, 0x696c4c00, 0x6a6d4d00, 0x6bf92500, 0x20b20000, 
  0x80000000, 0x6c2ab500, 0x82775700, 0x83785800, 0x84634300, 0x85765600, 
  0x86624200, 0x876e4e00, 0x882c3f00, 0x893b2e00, 0x8a3a2f00, 0x8b21a700, 
  0x8c000000, 0x332a0000, 0xa3000000, 0xa4000000, 0x60000000, 0x01000000, 
  0x02000000, 0x03000000, 0x04000000, 0x05000000, 0x06000000, 0x07000000, 
  0x08000000, 0x09000000, 0x0a000000, 0x31000000, 0xf5000000, 0x51370000, 
  0x52380000, 0x53390000, 0x342d0000, 0x71340000, 0x72350000, 0x73360000, 
  0x542b0000, 0x91310000, 0x92320000, 0x93330000, 0xb1300000, 0xb32e0000, 
  0xf5000000, 0xf5000000, 0x813c3e00, 0x0b000000, 0x0c000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0x94000000, 0xa6000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0x322f0000, 0xf5000000, 0xf5000000, 0xa5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0x2f000000, 0x8f000000, 0x30000000, 0xf5000000, 0xae000000, 
  0xf5000000, 0xb0000000, 0xf5000000, 0xf5000000, 0xaf000000, 0x50000000, 
  0x2e000000, 0x4e000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000};

static const uint32_t qwerty[] =
 {0xff000000, 0x00000000, 0x21312100, 0x22324000, 0x23332300, 0x24342400,
  0x25352500, 0x26365e00, 0x27372600, 0x28382a00, 0x29392800, 0x2a302900,
  0x2b2d5f00, 0x2c3d2b00, 0x2d000000, 0x40000000, 0x41715100, 0x42775700,
  0x43654500, 0x44725200, 0x45745400, 0x46795900, 0x47755500, 0x48694900,
  0x496f4f00, 0x4a705000, 0x4b5b7b00, 0x4c5d7d00, 0x4d000000, 0xa0000000,
  0x61614100, 0x62735300, 0x63644400, 0x64664600, 0x65674700, 0x66684800,
  0x676a4a00, 0x686b4b00, 0x696c4c00, 0x6a3b3a00, 0x6b272200, 0x20607e00,
  0x80000000, 0x6c5c7c00, 0x827a5a00, 0x83785800, 0x84634300, 0x85765600,
  0x86624200, 0x876e4e00, 0x886d4d00, 0x892c3c00, 0x8a2e3e00, 0x8b2f3f00,
  0x8c000000, 0x332a0000, 0xa3000000, 0xa4000000, 0x60000000, 0x01000000,
  0x02000000, 0x03000000, 0x04000000, 0x05000000, 0x06000000, 0x07000000,
  0x08000000, 0x09000000, 0x0a000000, 0x31000000, 0xf5000000, 0x51370000,
  0x52380000, 0x53390000, 0x342d0000, 0x71340000, 0x72350000, 0x73360000,
  0x542b0000, 0x91310000, 0x92320000, 0x93330000, 0xb1300000, 0xb32e0000,
  0xf5000000, 0xf5000000, 0x813c3e00, 0x0b000000, 0x0c000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0x94000000, 0xa6000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0x322f0000, 0xf5000000, 0xf5000000, 0xa5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0x2f000000, 0x8f000000, 0x30000000, 0xf5000000, 0xae000000,
  0xf5000000, 0xb0000000, 0xf5000000, 0xf5000000, 0xaf000000, 0x50000000,
  0x2e000000, 0x4e000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000,
  0xf5000000, 0xf5000000, 0xf5000000, 0xf5000000};

static const uint32_t *keyboard = azerty;
static char is_azerty = 1;

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
bit 11 -> alt gr (global)
bit 14 -> Accent (vrai si ^/¨/`/~ pressé)
bit 15 -> e0 lu, attent la suite */
static uint16_t modifiers = 1 << 7; // Verr Num activé au démarrage

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

static void keyboard_callback(registers_t regs) {
    /* The PIC leaves us the scancode in port 0x60 */
    ignore(regs);
    uint8_t scancode = port_byte_in(0x60);
    if (scancode == 0xE0) {
        //if (modifiers & (1 << 15)) {
            //return; //error
        //}
        modifiers |= 1 << 15; // sets bit 15
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
    if (modifiers & (1 << 15)) {// e0 pressé avant ?
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
        case 0xb8: // 6 Alt Droit
            set_modifiers(6, is_press);
            break; 
        case 0x45: // 7 Verr Num
            set_modifiers(7, is_press);
            break;
    }
    set_modifiers(8, ((modifiers & (1 << 2)) != 0) ^ ((modifiers & 0b11) != 0)); // Maj global
    set_modifiers(9, (modifiers & 0b11000) != 0); //Ctrl global
    set_modifiers(10, (((modifiers & (1 << 6)) != 0) | (((modifiers & (1 << 9)) != 0) & ((modifiers & (1 << 5)) != 0)))); //alt gr
    //kprintf(" t %d\n", modifiers & (1 << 10));
    uint32_t key_val = keyboard[scancode];
    key |= (modifiers & 0x7ff) << 16;    // copie de modificateurs (maj, alt, ctrl...)
    key |= (key_val & 0xff000000) >> 16; // copie ligne/colonne
    char fetch = 16; // 0 normal, 1 maj, 2 alt gr
    if (((((modifiers & (1 << 9)) != 0) ^ ((modifiers & (1 << 5)) != 0))) // Ctrl xor Alt
        | (((modifiers & (1 << 8)) != 0) & ((modifiers & (1 << 10)) != 0))) // maj et alt gr simultané
        key |= (1 << 29);
    else if ((modifiers & (1 << 8)) != 0)
        fetch = 8; // maj
    else if ((modifiers & (1 << 10)) != 0)
        fetch = 0; // alt gr
    char a = (key_val & (0xff << fetch)) >> fetch;
    if (a == 0 && fetch != 16) // pas de caractere Maj ou Alt Gr, on prend celui par defaut
        a = (key_val & (0xff << 16)) >> 16;
    char is_keypad = (keycode_column(key) >= 17) & ((modifiers & (1 << 7)) == 0);
    if ((a == 0) | is_keypad) // pas de caractere ascii ou pave sans Verr Num
        key |= special((key_val & 0xff000000) >> 24); // descriptif de caractere special
    else {
        key |= a; // on rajoute le caractere ascii en bout
        key |= (1 << 30);
    }
    key_pressed(key);

    modifiers &= 0b0011111111111111; // resets accent and e0 flag
}

void init_keyboard() {
    register_interrupt_handler(IRQ1, keyboard_callback); 
}

uint8_t keycode_pos(uint32_t keycode) {
    return (keycode >> 8) & 0xff;
}
uint8_t keycode_row(uint32_t keycode) {
    return (keycode >> 13) & 0x7;
}
uint8_t keycode_column(uint32_t keycode) {
    return (keycode >> 8) & 0x1f;
}
char keycode_ispressed(uint32_t keycode) {
    return 1 ^ (keycode >> 31);
}
char keycode_isascii(uint32_t keycode) {
    return keycode >> 30;
}

void keyboard_set(char mode) {
    if (mode) {
        keyboard = azerty;
        is_azerty = 1;
    }
    else {
        keyboard = qwerty;
        is_azerty = 0;        
    }
}
char keyboard_get() { return is_azerty; }

void keycode_print(uint32_t code) {
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
        case 0: return kprintf("Erreur");
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
        case 26: return kprintf("Retour");
        case 27: return kprintf("Suppr");
        case 28: return kprintf("Inser");
        case 29: return kprintf("Tab");
        case 30: return kprintf("Entree");
        case 32: return kprintf("Espace");
        case 40: return kprintf("Flch H");
        case 41: return kprintf("Flch G");
        case 42: return kprintf("Flch D");
        case 43: return kprintf("Flch B");
        case 44: return kprintf("Pg U");
        case 45: return kprintf("Pg B");
        case 46: return kprintf("Debut");
        case 47: return kprintf("Fin");
        default: return kprintf("Inconnu");
      }
    }
}

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
    case 0x2d: return  26; // Retour
    case 0x4e: 
    case 0xb3: return  27; // Suppr
    case 0x2e: 
    case 0xb1: return  28; // Inser
    case 0x40: return  29; // Tab
    case 0x4d:
    case 0x94: return  30; // Entree
    case 0xa4: return  32; // Espace
    case 0x8f:
    case 0x52: return  40; // Flch H
    case 0xae: 
    case 0x71: return  41; // Flch G
    case 0xb0: 
    case 0x73: return  42; // Flch D
    case 0xaf: 
    case 0x92: return  43; // Flch B
    case 0x30: 
    case 0x53: return  44; // Pg U
    case 0x50: 
    case 0x93: return  45; // Pg B
    case 0x2f: 
    case 0x51: return  46; // Debut
    case 0x91: return  47; // Fin
    default  : return 255; // Non-defini
  }
}