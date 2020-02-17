/*
Module implémentant la table des interruptions (IDT)
voir https://wiki.osdev.org/IDT
Gère les interruption de processeur
*/

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

/* Segment selectors */
#define KERNEL_CS 0x08

/*
Une entrée de l'IDT (interrupt gate)
On utilise la structure IA-32 https://wiki.osdev.org/IDT#Structure_IA-32
*/
typedef struct {
    uint16_t low_offset; /* Lower 16 bits of handler function address */
    uint16_t sel;        /* Kernel segment selector */
    uint8_t always0;     /* Toujours nul */
    /* Drapeaux:
       - Bit 7:    Interruption presente ou non
       - Bits 6-5: Niveau de privilege de l'appeleur (0=kernel..3=user)
       - Bit 4:    0 for interrupt gates
       - Bits 3-0: bits 1110 = decimal 14 = "32 bit interrupt gate" */
    uint8_t flags;
    uint16_t high_offset; /* Higher 16 bits of handler function address */
} __attribute__((packed)) idt_gate_t ;

/* Pointeur vers l'IDT, donné en argument a lidt */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_register_t;

#define IDT_ENTRIES 256

idt_gate_t idt[IDT_ENTRIES];
idt_register_t idt_reg;

void set_idt_gate(int n, uint32_t handler);
void init_idt_desc(int n,uint16_t select, uint32_t handler, uint16_t type);
void set_idt();

/* Structure qui concatene de nombreux registres  */
typedef struct {
   uint32_t ds; /* selecteur du segment de données */
   uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* poussés par pusha. */
   uint16_t int_no, err_code; /* Numero de l'interruption et code d'erreur (si applicable) */
   uint32_t eip, cs, eflags, useresp, ss; /* poussées par le processeur automatiquement */
} registers_t;

void isr_install(); // créer l'IDT a partir des isr0-31 reservés par le processeur
void isr_handler(registers_t r); // gere l'interruption

typedef void (*isr_t)(registers_t);

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler);
void irq_handler(registers_t r);

void print_debug(void);

#endif
