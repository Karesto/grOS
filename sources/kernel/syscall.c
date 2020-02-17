
#include "syscall.h"

#include "../libc/types.h"
#include "../libc/iostream.h"
#include "../cpu/gdt.h"

void do_syscalls(int sys_num)
{
	char *u_str;
	int i;
	kprintf("Syscall number %d .", sys_num);
	if (sys_num == 1) {
		asm("mov %%ebx, %0": "=m"(u_str) :);
		for (i = 0; i < 100000; i++);	/* temporisation */
		kprintf(u_str);
	} else {
		kprintf("unknown syscall %d\n", sys_num);
	}

	return;
}
