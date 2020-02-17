
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "GDT.h"

/*
voir https://wiki.osdev.org/Global_Descriptor_Table#Structure
une entree de la GDT est sur 64bits:
 0-15: limite 0:15
16-31: base 0:15
32-39: base 16:23
40-47: octet d'acces
48-51: limite 16:19
52-55: drapeaux
56-63: base 24-31

base : valeur 32 bits donnant l'addresse de depart du segment
limite : 30 bits, fin des addresse, en octet ou en pages

acces:
  7: bit de presentation, vaut 1
6-5: Privileges (0 kernel - 3 user application)
  4: type du descripteur, 1 pour de segments de code ou de données, 0 pour des segments systemes
  3: executable (1 pour le code, 0 pour les données)
  2: bit de direction pour les donnees 0 pour croissant, 1 pour decroissant (base < limit)
     bit de conformite pour le code 1 -> executable depuis un privilege <=
                                    0 -> executable uniquement depuis un privilege =
  1: readable pour le segment de code, writable pour les segments de donnees
  0: bit d'acces, vaut 0 (le CPU le met a 1 pdt la lecture)

drapeaux:
  4: granularite, si 0 la limite est en octet, si 1 en Kio
  3: taille, 0 pour 16 bits, 1 pour 32 bits
  2: 0
  1: 0
*/

#include "printer.h"

static const size_t GDT_LENGTH = 4;
static const size_t GDT_SIZE = 8 * GDT_LENGTH;

asm(".data\n" 
    "gdtr: .quad 0\n"
    "GDT: .skip 36\n" // GDT_SIZE 4 quad = 36 octets 
    ".text\n"
    "get_GDT:\n\t"
    "mov   $GDT, %eax\n\t"
    "ret\n"
    "set_GDT:\n\t"
    "mov   4(%esp), %eax\n\t"
    "mov   $gdtr, %edi\n\t"
    "mov   %eax, 2(%edi)\n\t"
    "mov   8(%esp), %ax\n\t"
    "mov   %ax, (%edi)\n\t"
    "lgdt  (%edi)\n\t"
    "ret\n\t"
);

inline uint64_t gdt_entry(uint32_t base, uint32_t limit, uint8_t acces, uint8_t flags)
{
    uint64_t base64 = base;
    uint64_t limit64 = limit;
    return (limit64 & 0xffff) 
         | (base64 & 0xffff)          << 16
         | ((base64 >> 16) & 0xff)    << 32
         | ((uint64_t)acces)          << 40
         | ((limit64 >> 16) & 0b111)  << 48
         | ((uint64_t)flags & 0b1100) << 52
         | (base64 >> 24)             << 56;
}

void init_gdt(void)
{
  uint64_t *GDT = get_GDT();
  printf("%x", gdt_entry(0, 0, 0, 0));
  printf("%x", gdt_entry(0, 0xffffff, 0x9a, 0b1100)); // code
  printf("%x", gdt_entry(0, 0xffffff, 0x92, 0b1100)); // donnees
  //GDT[3] = gdt_entry(&myTSS, sizeof(myTSS), 0x89)

  //0
  //c79a000000ffff
  //c792000000ffff

  set_GDT(GDT, GDT_SIZE - 1);
}