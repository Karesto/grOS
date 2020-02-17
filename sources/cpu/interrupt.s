// Defini dans interrupts.c
.global isr_handler
.global irq_handler
.global do_syscalls
.global print_debug

// Code commun aux interruption
isr_common_stub:
    // Savegarde de l etat du CPU
	pusha             // sauvegarde %edi,%esi,%ebp,%esp,%ebx,%edx,%ecx,%eax sur la pile
	movw  %ds, %ax    // %ds correspond aux 16bits de poids faible de %eax
	pushl %eax        // save the data segment descriptor
	movl  $0x10, %eax // descripteur du segment de données du kernel
	movw  %ax, %ds
	movw  %ax, %es
	movw  %ax, %fs
	movw  %ax, %gs
    // Appel de la fonction C
	call  isr_handler

    // On restore l etat
	popl  %eax
	movw  %ax, %ds
	movw  %ax, %es
	movw  %ax, %fs
	movw  %ax, %gs
	popa
	add   $4, %esp // On enlève le code d erreur et numero ISR poussé
	sti            // On rétabli les interruptions
	iret           // On enlève CS, EIP, EFLAGS, SS, and ESP

// Common IRQ code. Identical to ISR code except for the  call
// and the  pop ebx
irq_common_stub:
    pusha
    movw %ds, %ax
    pushl %eax
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    call irq_handler // Different than the ISR code
    popl %ebx         // Different than the ISR code
    movw %bx, %ds
    movw %bx, %es
    movw %bx, %fs
    movw %bx, %gs
    popa
    add $4, %esp
    sti
    iret


syscall_common_stub:
    pusha
    movw %ds, %ax
    pushl %eax
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    push %eax                 // transmission du numero d appel
    call do_syscalls // appel de syscall qui gère la fougère
    popl %eax
    movw %bx, %ds
    movw %bx, %es
    movw %bx, %fs
    movw %bx, %gs
    popa
    add $4, %esp
    sti
    iret



// On défini un handleur par interruption
// Certaines interruptions placent un code d erreurs sur la pile
// Pour la consistence de la pile, on en poussera un vide pour les autres

// les isr doivent être visibles depuis le C
.global isr0
.global isr1
.global isr2
.global isr3
.global isr4
.global isr5
.global isr6
.global isr7
.global isr8
.global isr9
.global isr10
.global isr11
.global isr12
.global isr13
.global isr14
.global isr15
.global isr16
.global isr17
.global isr18
.global isr19
.global isr20
.global isr21
.global isr22
.global isr23
.global isr24
.global isr25
.global isr26
.global isr27
.global isr28
.global isr29
.global isr30
.global isr31

// IRQs
.global irq0
.global irq1
.global irq2
.global irq3
.global irq4
.global irq5
.global irq6
.global irq7
.global irq8
.global irq9
.global irq10
.global irq11
.global irq12
.global irq13
.global irq14
.global irq15

// syscalls
.global _asm_syscalls


// 0: Division par 0
isr0:
    cli
    pushw $0
    pushw $0
    jmp isr_common_stub

// 1: Exception de Debug
isr1:
    cli
    pushw $0
    pushw $1
    jmp isr_common_stub

// 2: Interruption non masquable
isr2:
    cli
    pushw $0
    pushw $2
    jmp isr_common_stub

// 3: Int 3 Exception
isr3:
    cli
    pushw $0
    pushw $3
    jmp isr_common_stub

// 4: INTO Exception
isr4:
    cli
    pushw $0
    pushw $4
    jmp isr_common_stub

// 5: Out of Bounds Exception
isr5:
    cli
    pushw $0
    pushw $5
    jmp isr_common_stub

// 6: Opcode Invalide
isr6:
    cli
    pushw $0
    pushw $6
    jmp isr_common_stub

// 7: Coprocessor Not Available Exception
isr7:
    cli
    pushw $0
    pushw $7
    jmp isr_common_stub

// 8: Double Fault Exception (a un code d erreur)
isr8:
    cli
    pushw $8
    jmp isr_common_stub

// 9: Coprocessor Segment Overrun Exception
isr9:
    cli
    pushw $0
    pushw $9
    jmp isr_common_stub

// 10: Bad TSS Exception (a un code d erreur)
isr10:
    cli
    pushw $10
    jmp isr_common_stub

// 11: Segment Not Present Exception (a un code d erreur)
isr11:
    cli
    pushw $11
    jmp isr_common_stub

// 12: Stack Fault Exception (a un code d erreur)
isr12:
    cli
    pushw $12
    jmp isr_common_stub

// 13: General Protection Fault Exception (a un code d erreur)
isr13:
    cli
    pushw $13
    jmp isr_common_stub

// 14: Page Fault Exception (a un code d erreur)
isr14:
    cli
    pushw $14
    jmp isr_common_stub

// 15: Reserved Exception
isr15:
    cli
    pushw $0
    pushw $15
    jmp isr_common_stub

// 16: Floating Point Exception
isr16:
    cli
    pushw $0
    pushw $16
    jmp isr_common_stub

// 17: Alignment Check Exception
isr17:
    cli
    pushw $0
    pushw $17
    jmp isr_common_stub

// 18: Machine Check Exception
isr18:
    cli
    pushw $0
    pushw $18
    jmp isr_common_stub

// 19: Reserved
isr19:
    cli
    pushw $0
    pushw $19
    jmp isr_common_stub

// 20: Reserved
isr20:
    cli
    pushw $0
    pushw $20
    jmp isr_common_stub

// 21: Reserved
isr21:
    cli
    pushw $0
    pushw $21
    jmp isr_common_stub

// 22: Reserved
isr22:
    cli
    pushw $0
    pushw $22
    jmp isr_common_stub

// 23: Reserved
isr23:
    cli
    pushw $0
    pushw $23
    jmp isr_common_stub

// 24: Reserved
isr24:
    cli
    pushw $0
    pushw $24
    jmp isr_common_stub

// 25: Reserved
isr25:
    cli
    pushw $0
    pushw $25
    jmp isr_common_stub

// 26: Reserved
isr26:
    cli
    pushw $0
    pushw $26
    jmp isr_common_stub

// 27: Reserved
isr27:
    cli
    pushw $0
    pushw $27
    jmp isr_common_stub

// 28: Reserved
isr28:
    cli
    pushw $0
    pushw $28
    jmp isr_common_stub

// 29: Reserved
isr29:
    cli
    pushw $0
    pushw $29
    jmp isr_common_stub

// 30: Reserved
isr30:
    cli
    pushw $0
    pushw $30
    jmp isr_common_stub

// 31: Reserved
isr31:
    cli
    pushw $0
    pushw $31
    jmp isr_common_stub


// IRQ handlers
irq0:
	cli
	pushw $0
	pushw $32
	jmp irq_common_stub

irq1:
	cli
	pushw $1
	pushw $33
	jmp irq_common_stub

irq2:
	cli
	pushw $2
	pushw $34
	jmp irq_common_stub

irq3:
	cli
	pushw $3
	pushw $35
	jmp irq_common_stub

irq4:
	cli
	pushw $4
	pushw $36
	jmp irq_common_stub

irq5:
	cli
	pushw $5
	pushw $37
	jmp irq_common_stub

irq6:
	cli
	pushw $6
	pushw $38
	jmp irq_common_stub

irq7:
	cli
	pushw $7
	pushw $39
	jmp irq_common_stub

irq8:
	cli
	pushw $8
	pushw $40
	jmp irq_common_stub

irq9:
	cli
	pushw $9
	pushw $41
	jmp irq_common_stub

irq10:
	cli
	pushw $10
	pushw $42
	jmp irq_common_stub

irq11:
	cli
	pushw $11
	pushw $43
	jmp irq_common_stub

irq12:
	cli
	pushw $12
	pushw $44
	jmp irq_common_stub

irq13:
	cli
	pushw $13
	pushw $45
	jmp irq_common_stub

irq14:
	cli
	pushw $14
	pushw $46
	jmp irq_common_stub

irq15:
	cli
	pushw $15
	pushw $47
	jmp irq_common_stub

_asm_syscalls:
	jmp syscall_common_stub
