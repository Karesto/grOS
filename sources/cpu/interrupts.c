/* Voir idt.h pour la documentation */

#include "interrupts.h"

#include <stdint.h>

#include "../cpu/pager.h"
#include "../libc/iostream.h"
#include "../cpu/ports.h"

void set_idt_gate(int n, uint32_t handler) {
    idt[n].low_offset = handler & 0xFFFF;
    idt[n].sel = KERNEL_CS;
    idt[n].always0 = 0;
    idt[n].flags = 0x8E;
    idt[n].high_offset = (handler >> 16) & 0xFFFF;
}


/* Même fonction qu'en haut, mais cette fois permet de modifier les flags*/
void init_idt_desc(int n,uint16_t select, uint32_t handler, uint16_t type)
{
	idt[n].low_offset = (handler & 0xffff);
	idt[n].sel = select;
    idt[n].always0 = 0;
	idt[n].flags = type;
	idt[n].high_offset = (handler & 0xffff0000) >> 16;
	return;
}

void set_idt() {
    idt_reg.base = (uint32_t)&idt; //addresse de l'idt;
    idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
    /* Don't make the mistake of loading &idt -- always load &idt_reg */
    __asm__ __volatile__("lidt (%0)" : : "r" (&idt_reg));
}





/*
ISR reservées par le CPU,
définis dans interrupts.s
*/
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void _asm_syscalls();

/* Pas de boucle car on a besoin des adresses */
void isr_install() {
    set_idt_gate(0, (uint32_t)isr0);
    set_idt_gate(1, (uint32_t)isr1);
    set_idt_gate(2, (uint32_t)isr2);
    set_idt_gate(3, (uint32_t)isr3);
    set_idt_gate(4, (uint32_t)isr4);
    set_idt_gate(5, (uint32_t)isr5);
    set_idt_gate(6, (uint32_t)isr6);
    set_idt_gate(7, (uint32_t)isr7);
    set_idt_gate(8, (uint32_t)isr8);
    set_idt_gate(9, (uint32_t)isr9);
    set_idt_gate(10, (uint32_t)isr10);
    set_idt_gate(11, (uint32_t)isr11);
    set_idt_gate(12, (uint32_t)isr12);
    set_idt_gate(13, (uint32_t)isr13);
    set_idt_gate(14, (uint32_t)isr14);
    set_idt_gate(15, (uint32_t)isr15);
    set_idt_gate(16, (uint32_t)isr16);
    set_idt_gate(17, (uint32_t)isr17);
    set_idt_gate(18, (uint32_t)isr18);
    set_idt_gate(19, (uint32_t)isr19);
    set_idt_gate(20, (uint32_t)isr20);
    set_idt_gate(21, (uint32_t)isr21);
    set_idt_gate(22, (uint32_t)isr22);
    set_idt_gate(23, (uint32_t)isr23);
    set_idt_gate(24, (uint32_t)isr24);
    set_idt_gate(25, (uint32_t)isr25);
    set_idt_gate(26, (uint32_t)isr26);
    set_idt_gate(27, (uint32_t)isr27);
    set_idt_gate(28, (uint32_t)isr28);
    set_idt_gate(29, (uint32_t)isr29);
    set_idt_gate(30, (uint32_t)isr30);
    set_idt_gate(31, (uint32_t)isr31);

    // Remap the PIC
    port_byte_out(0x20, 0x11);
    io_wait();
    port_byte_out(0xA0, 0x11);
    io_wait();
    port_byte_out(0x21, 0x20);
    io_wait();
    port_byte_out(0xA1, 0x28);
    io_wait();
    port_byte_out(0x21, 0x04);
    io_wait();
    port_byte_out(0xA1, 0x02);
    io_wait();
    port_byte_out(0x21, 0x01);
    io_wait();
    port_byte_out(0xA1, 0x01);
    io_wait();
    port_byte_out(0x21, 0x0);
    io_wait();
    port_byte_out(0xA1, 0x0);
    io_wait();

    // Install the IRQs
    set_idt_gate(32, (uint32_t)irq0);
    set_idt_gate(33, (uint32_t)irq1);
    set_idt_gate(34, (uint32_t)irq2);
    set_idt_gate(35, (uint32_t)irq3);
    set_idt_gate(36, (uint32_t)irq4);
    set_idt_gate(37, (uint32_t)irq5);
    set_idt_gate(38, (uint32_t)irq6);
    set_idt_gate(39, (uint32_t)irq7);
    set_idt_gate(40, (uint32_t)irq8);
    set_idt_gate(41, (uint32_t)irq9);
    set_idt_gate(42, (uint32_t)irq10);
    set_idt_gate(43, (uint32_t)irq11);
    set_idt_gate(44, (uint32_t)irq12);
    set_idt_gate(45, (uint32_t)irq13);
    set_idt_gate(46, (uint32_t)irq14);
    set_idt_gate(47, (uint32_t)irq15);
    init_idt_desc(48 ,0x08, (uint32_t)_asm_syscalls, 0xEF);

    set_idt(); // chargement de l'idt avec lidt
}

/* To print the message which defines every exception */
char *exception_messages[] = {
    "Division par zero",
    "Debug",
    "Interruption non masquable",
    "Point d'arrêt (Breakpoint)",
    "Into Detected Overflow",
    "Out of Bounds",
    "Opcode invalide",
    "Pas de coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Mauvais TSS",
    "Segment non present",
    "Faute de la pile (Stack Fault)",
    "General Protection Fault",
    "Faute de pagination (Page Fault)",
    "Interruption inconnue",

    "Faute du coprocessor",
    "Verification de l'alignment",
    "Verification machine",
    "Reservé",
    "Reservé",
    "Reservé",
    "Reservé",
    "Reservé",

    "Reservé",
    "Reservé",
    "Reservé",
    "Reservé",
    "Reservé",
    "Reservé",
    "Reservé",
    "Reservé"
};
#define ignore(x) (void)(x) //enleve un warning, pas d'autre interet...
void isr_handler(registers_t r) {
  if (r.int_no == 14)
  {
    isr_PF_exc();
  }
  else{
    ignore(r);
    kprintf("\033[4f]Interruption recue:\033[_] %02ud %ud\n  %s\n", r.int_no, r.err_code, exception_messages[r.int_no]);
    return;}
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

void irq_handler(registers_t r) {
    /*
    On envoie un EOI sur les PICs pour qu'ils ne renvoient
    pas d'autre interruption
    */
    if (r.int_no >= 40) port_byte_out(0xA0, 0x20); /* slave */
    port_byte_out(0x20, 0x20); /* master */

    /* Handle the interrupt in a more modular way */
    //kprintf("Something happened irq\n");
    if (interrupt_handlers[r.int_no] != 0) {
        isr_t handler = interrupt_handlers[r.int_no];
        handler(r);
    }
}

void print_debug(void)
{
    kprintf("\033[4f]DEBUGGGGGGGG!!!!!\n");
}
