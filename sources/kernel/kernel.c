/* fichier noyau */

#include "kernel.h"
#include "shell.h"

#include "../libc/iostream.h"
#include "../libc/mem.h"
#include "../drivers/disk.h"
#include "../drivers/timer.h"
#include "../cpu/pager.h"
#include "../cpu/gdt.h"
#include "../core/process.h"

const unsigned char VERSION_MAJOR = 0;
const unsigned char VERSION_MINOR = 2;
const unsigned char VERSION_PATCH = 1;

void kernel_main(void)
{

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

	init_disk();
	//kprintf("\n\033[10f]user\033[_]$ ");
	//readline();
	while (1); // comment out to run scheduler, generates page faults
	asm("cli");

	main();
}

void task1(void)
{	kprintf("Done\n");
	char *msg = (char *) 0x1000;
	int i;

	msg[0] = 't';
	msg[1] = 'a';
	msg[2] = 's';
	msg[3] = 'k';
	msg[4] = '1';
	msg[5] = '\n';
	msg[6] = 0;

	while (1) {
		asm("mov %0, %%ebx; mov $0x01, %%esi; int $0x30":: "m"(msg));
		for (i = 0; i < 1000000; i++);
	}

	return;			/* never go there */
}

void task2(void)
{
	kprintf("Done\n");

	char *msg = (char *) 0x1000;
	int i;

	msg[0] = 't';
	msg[1] = 'a';
	msg[2] = 's';
	msg[3] = 'k';
	msg[4] = '2';
	msg[5] = '\n';
	msg[6] = 0;

	while (1) {
		asm("mov %0, %%ebx; mov $0x01, %%esi; int $0x30":: "m"(msg));
		for (i = 0; i < 1000000; i++);
	}

	return;			/* never go there */
}

int main(void)

{
	//init_gdt();
	//kprintf("in main \n"); 
	/* Initialisation du TSS */
	/*asm("	movw $0x39, %ax \n \
				ltr %ax");
	//kprintf("kernel : tr loaded\n");

	init_mm();

	//kprintf("kernel : paging enable\n");*/

	load_task((u32 *) 0x200000, (u32 *) & task1, 0x2000);
	load_task((u32 *) 0x300000, (u32 *) & task2, 0x2000);
	kprintf("kernel : scheduler enable\n");

	asm("sti");

 	while (1);
 }
