
.global do_switch

do_switch:
	//recuper l adresse de *current
	mov (%esp), %esi
	pop %eax			// depile @current

	//prepare les registres

	pushl 8(%esi)    //eax
	pushl 12(%esi)	 //ecx
	pushl 16(%esi)	 //edx
	pushl 20(%esi)	 //ebx
	pushl 24(%esi)	 //ebp
	pushl 28(%esi)	 //esi
	pushl 32(%esi)	 //edi
	pushl 48(%esi)	 //ds
	pushl 50(%esi)	 //es
	pushl 52(%esi)	 //fs
	pushl 54(%esi)	 //gs


	 //enleve le mask du PIC doesn t work yet
	mov $0x20,%al
	out %al, $0x20


	// charge table des pages
	mov 56(%esi), %eax
	mov %eax, %cr3

	// charge les registres
	pop %gs
	pop %fs
	pop %es
	pop %ds
	pop %edi
	pop %esi
	pop %ebp
	pop %ebx
	pop %edx
	pop %ecx
	pop %eax

	// retourne
	iret
