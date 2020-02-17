/* entree et sortie au travers divers ports */

#include "ports.h"

#include <stdint.h>

// lire un octe du port specifié
uint8_t port_byte_in (uint16_t port) {
    uint8_t result;
    /* Inline assembler syntax
     * !! Notice how the source and destination registers are switched from NASM !!
     *
     * '"=a" (result)'; set '=' the C variable '(result)' to the value of register e'a'x
     * '"d" (port)': map the C variable '(port)' into e'd'x register
     *
     * Inputs and outputs are separated by colons
     */
    __asm__("inb %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

// ecrire un octet dans le port designe
void port_byte_out (uint16_t port, uint8_t data) {
    __asm__ __volatile__("outb %%al, %%dx" : : "a" (data), "d" (port));
}

// deux octets
uint16_t port_word_in (uint16_t port) {
    uint16_t result;
    __asm__("inw %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

// deux octets
void port_word_out (uint16_t port, uint16_t data) {
    __asm__ __volatile__("outw %%ax, %%dx" : : "a" (data), "d" (port));
}


void io_wait(void) {
    /* TODO: This is probably fragile. */
    asm volatile ( "jmp 1f\n\t"
                   "1:jmp 2f\n\t"
                   "2:" );
}
