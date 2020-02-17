/* fichier noyau */

#include "../drivers/printer.h"
#include "../drivers/keyboard.h"
#include "../drivers/disk.h"
//#include "../cpu/interrupts.h"
//#include "../cpu/timer.h"
#include "../paging/pager.h"
#include "../taskexec/lib.h"
#include "../taskexec/gdt.h"

#include "kernel.h"

const unsigned char VERSION_MAJOR = 0;
const unsigned char VERSION_MINOR = 1;
const unsigned char VERSION_PATCH = 3;

void kernel_main(void)
{
  /* Initialize linear   paging    */
  //init_mm();
	/* Initialize terminal interface */
	terminal_initialize();

	kprintf("\n");
	kprintf("                 \033[14f]  .oooooo.   \033[10f]          \033[4f]  .oooooo.   \033[9f] .oooooo..o \n");
	kprintf("                 \033[14f] d8P'  `Y8b  \033[10f]          \033[4f] d8P'  `Y8b  \033[9f]d8P'    `Y8 \n");
	kprintf("                 \033[14f]888          \033[10f] oooo d8b \033[4f]888      888 \033[9f]Y88bo.      \n");
	kprintf("                 \033[14f]888          \033[10f] `888''8P \033[4f]888      888 \033[9f] `'Y8888o.  \n");
	kprintf("                 \033[14f]888     ooooo\033[10f]  888     \033[4f]888      888 \033[9f]     `'Y88b \n");
	kprintf("                 \033[14f]`88.    .88' \033[10f]  888     \033[4f]`88b    d88' \033[9f]oo     .d8P \n");
	kprintf("                 \033[14f] `Y8bood8P'  \033[10f] d888b    \033[4f] `Y8bood8P'  \033[9f]8''88888P'  \n");
	kprintf("\033[_]\n");
  kprintf("GrOS version %ud.%ud.%ud\nClavier en AZERTY, appuyer sur F1 pour QWERTY.\n\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
  /* Initialisation de la GDT et des segments */
	init_gdt();
	kprintf("kernel : gdt loaded\n");

	/* Initialisation du TSS */
	asm("	movw $0x38, %ax \n \
	   	ltr %ax");
	kprintf("kernel : tr loaded\n");

	/* Initialisation du pointeur de pile %esp */
	/* asm("   movw $0x18, %ax \n \
                movw %ax, %ss \n \
                movl $0x20000, %esp"); */

/* Fonction qu'on appelle après avoir chargé le kernel */
	init_disk();
	while (1)		;
	//main();
}

void task1(void)
{
  char *msg = (char *) 0x40000100;	/* le message sera stock� en 0x100100 */
  msg[0] = 't';
  msg[1] = 'a';
  msg[2] = 's';
  msg[3] = 'k';
  msg[4] = '1';
  msg[5] = '\n';
  msg[6] = 0;
  asm("mov %0, %%ebx \n \
      mov $0x01, %%esi \n \
      int $0x30"::"m"(msg));

	while (1);
	return;			/* never go there */
}


int main(void)
{
  u32 *pd;
  /* Initialize  paging    */
  init_mm();
  kprintf("kernel : Paging enabled\n");

  pd = pd_create_task();
  memcpy((char *) 0x100000, (char *) &task1, 100);	/* copie de 100 instructions */
  kprintf("kernel : task created\n");

  kprintf("kernel : trying switch to user task...\n");

	/* Copie de la fonction a son adresse */
	memcpy((char*) 0x30000, &task1, 100);

	/* Saut vers la fonction task1() en mode utilisateur */
  asm ("  cli                    \n \
      		movl $0x20000, %0      \n \
      		movl %1, %%eax         \n \
      		movl %%eax, %%cr3      \n \
      		push $0x33             \n \
      		push $0x40000F00       \n \
      		pushfl                 \n \
      		popl %%eax             \n \
      		orl $0x200, %%eax      \n \
      		and $0xFFFFBFFF, %%eax \n \
      		push %%eax             \n \
      		push $0x23             \n \
      		push $0x40000000       \n \
      		movw $0x2B, %%ax       \n \
      		movw %%ax, %%ds        \n \
      		iret" : "=m"(default_tss.esp0) : "m"(pd));

	/* Jamais atteint ! */
	kprintf("Critical error, halting system\n");
	asm("hlt");
}

