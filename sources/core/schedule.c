#include "schedule.h"

#include "../libc/types.h"
#include "../cpu/gdt.h"
#include "../core/process.h"
#include "../libc/iostream.h"

void schedule(void)
{

	//kprintf("Je schedule...");
	u32* stack_ptr;
	u32 esp0, eflags;
	u16 ss, cs;

	/* Stocke dans stack_ptr le pointeur vers les registres sauvegardes */
	asm("mov (%%ebp), %%eax; mov %%eax, %0" : "=m" (stack_ptr) : );

	/* Si il n'y a pas de processus charge et qu'au moins un est pret, on le charge */
	if (current == 0 && n_proc) {
		current = &p_list[0];
	}
	/*
	 * Si il y a un seul processus (qu'on laisse tourner) ou aucun
	 * processus, on retourne directement.
	 */
	else if (n_proc <= 1) {
		return;
	}
	/* Si il y a au moins deux processus, on commute vers le suivant */
	else if (n_proc>1) {
		/* Sauver les registres du processus courant */
		current->regs.eflags = stack_ptr[16];
		current->regs.cs  = stack_ptr[15];
		current->regs.eip = stack_ptr[14];
		current->regs.eax = stack_ptr[13];
		current->regs.ecx = stack_ptr[12];
		current->regs.edx = stack_ptr[11];
		current->regs.ebx = stack_ptr[10];
		current->regs.ebp = stack_ptr[8];
		current->regs.esi = stack_ptr[7];
		current->regs.edi = stack_ptr[6];
		current->regs.ds = stack_ptr[5];
		current->regs.es = stack_ptr[4];
		current->regs.fs = stack_ptr[3];
		current->regs.gs = stack_ptr[2];

		current->regs.esp = stack_ptr[17];
		current->regs.ss = stack_ptr[18];

		/*
		 * La fonction l'interruption et la fonction schedule()
		 * empilent un grand nombre de registres sur la pile.
		 * L'instruction ci-dessous permet de repartir sur une pile
		 * noyau propre une fois la commutation effectuee.
		 */
		default_tss.esp0 = (u32) (stack_ptr + 19);

		/* Choix du nouveau processus (un simple roundrobin) */
		if (n_proc > current->pid+1)
			current = &p_list[current->pid+1];
		else
			current = &p_list[0];
	}

	/*
	 * Empile les registres ss, esp, eflags, cs et eip necessaires a la
	 * commutation. Ensuite, la fonction do_switch() restaure les
	 * registres, la table de page du nouveau processus courant et commute
	 * avec l'instruction iret.
	 */
	ss = current->regs.ss;
	cs = current->regs.cs;
	eflags = (current->regs.eflags | 0x200) & 0xFFFFBFFF;
	esp0 = default_tss.esp0;

	asm("	mov %0, %%esp; \
		push %1; \
		push %2; \
		push %3; \
		push %4; \
		push %5; \
		push %6; \
		ljmp $0x08, $do_switch; " \
		:: \
		"m" (esp0), \
		"m" (ss), \
		"m" (current->regs.esp), \
		"m" (eflags), \
		"m" (cs), \
		"m" (current->regs.eip), \
		"m" (current)
	);

}
