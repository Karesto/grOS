/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

/*
Declare a multiboot header that marks the program as a kernel. see multiboot standard.
Must be in the first 8 KiB of kernel file for bootloader, aligned at a 32-bit boundary.
*/
.section .multiboot //own section, so it can be explicitly placed in first 8 KiB
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*
Création d'une pile de 16 KiB pour pouvoir executer du code C
*/
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

/*
Le script linker.ld specifie que _start est le point d'entree du kernel
*/
.section .text
.global _start
.type _start, @function
_start:
	/*
	Le bootloader nous place en 32-bit mode protegé sur une machine x86
	Interruptions et pagination desactivé.
	Le kernel a un control total sur le CPU et ne peut utiliser que
	les fonction hardware:
	 - pas de bibliotheque standard hormis celle qu'on a coder.
	 - pas de restriction de securite
	 - pas d'outils de deboggage
	*/

	mov $stack_top, %esp // pointeur de pile

	/*
	Initialisation de l'état du processeur
	 - definition de la GDT
	 - reactivation de la pagination
	Pas encore d'instruction flottantes, d'exception ou constructeurs globaux (C++).
	*/

	// Chargement de la GDT (definie dans le segment .data
	movl   $GDT, %eax
    movl   $gdtr, %edi
    movl   %eax, 2(%edi)
    movw   $63, %ax // 31 = 8*8-1 pour une GDT a 8 entrees (8 pour 64 bits)
    movw   %ax, (%edi)
    lgdtl  (gdtr)
	//initialisation des segments
    movw   $0x10, %ax
    movw   %ax, %ds
    movw   %ax, %es
    movw   %ax, %fs
    movw   %ax, %gs
    ljmp   $0x08, $next
next:
	call terminal_initialize // drivers/tty
	// lancement des interruptions
	call isr_install   // cpu/interrupts
	sti
	call init_keyboard // drivers/keyboard
	push $50
	call init_timer    // drivers/timer
	pop %eax

	// chargement de la GDT et des segments
	call init_gdt // taskexec/gdt
	movw $0x39, %ax
	ltr %ax

	call init_mm //paging/pager

	//call init_disk // drivers/disk

	//
	// Appel au kernel haut-niveau. La pile doit être alignée sur 16-byte au call.
	call kernel_main // kernel/kernel

	/*
	Si le système n'a plus rien a faire, on entre en boucle infinie
	1) cli desactive les interruption
	2) Attendre l'interruption suivante avec hlt (halt instruction).
	   Etant desactivée, cela verrouille l'ordinateur
	3) Si hlt se reveille (interruption non masquable)
	*/
	call print_debug // cpu/interrupts
	cli
1:	hlt
	jmp 1b

/*
Definir la taille de _start comme la position courant '.' moins son debut.
Utile pour le deboggage ou le traceback d'appels.
*/
.size _start, . - _start

.section .data
// Définition de la GDT
GDT:    .quad 0x0000000000000000 // blank
        .quad 0x00df9b000000ffff // code
        .quad 0x00df93000000ffff // donnees
        .quad 0x00d0970000000000 // piles
        .quad 0x00d0ff0300000001 // ucode
        .quad 0x00d0f30300000001 // udata
        .quad 0x00d0f70000000000 // ustack
        .quad 0x0000e91084600067 // tss
/*GDT:    .quad 0x0000000000000000 // blank
        .quad 0x00d79b000000ffff // code
        .quad 0x00d793000000ffff // donnees
        .quad 0x00d0970000000000 // piles
        .quad 0x00d0ff0300000001 // ucode
        .quad 0x00d0f30300000001 // udata
        .quad 0x00d0f70000000000 // ustack
        .quad 0xf400e9a018600067 // tss
GDT:	.quad 0x0
		.quad 0xc79a000000ffff // code
		.quad 0xc792000000ffff // data
		.quad 0x0*/
gdtr:	.quad 0
