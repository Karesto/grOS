BLD=build
ISO=iso
BOOT=$(ISO)/boot
SRC=sources
FLAGS=-g -c -O0 -std=gnu99 -ffreestanding -Wall -Wextra #-I $(SRC)
OBJS=$(addprefix $(BLD)/, boot.o tty.o iostream.o ports.o interrupts.o interrupt.o\
                         timer.o keyboard.o disk.o kernel.o pager.o gdt.o mem.o\
						 buffer.o syscall.o schedule.o process.o schedd.o \
						 string.o linked_list.o filesystem.o fileio.o shell.o)
CPATH=~/opt/cross/bin/i686-elf
#chemin partiel (contient le debut du nom des fichiers)
#complété par -gcc -as -ld etc..
CC=$(CPATH)-gcc #compilateur croisé i686-elf

$(BLD)/GrOS.bin: $(BLD) $(OBJS) $(BLD)/disk.iso
	$(CPATH)-gcc -T $(SRC)/boot/linker.ld -o $@ -ffreestanding -Ofast -nostdlib $(OBJS) -lgcc

GrOS.iso: $(BOOT) $(BLD)/GrOS.bin
	cp $(BLD)/GrOS.bin $(BOOT)/GrOS.bin
	cp $(SRC)/boot/grub.cfg $(BOOT)/grub/grub.cfg
	grub-mkrescue -o $@ $(ISO)

$(BLD)/disk.iso:
	qemu-img create -f raw $@ 10M


run: $(BLD)/GrOS.bin
	qemu-system-i386 -monitor stdio -kernel $< -drive file=$(BLD)/disk.iso,format=raw
debug: $(BLD)/GrOS.bin
	qemu-system-i386 -kernel $< -s -S -d guest_errors -drive file=$(BLD)/disk.iso,format=raw
clean:
	rm -rf $(BLD) $(ISO) GrOS.iso

.PHONY: clean run debug

$(BLD)/%.o: $(SRC)/boot/%.s
	$(CPATH)-as -o $@ $<
$(BLD)/%.o: $(SRC)/cpu/%.s
	$(CPATH)-as -o $@ $<
$(BLD)/%.o: $(SRC)/core/%.s
	$(CPATH)-as -o $@ $<



$(BLD)/%.o: $(SRC)/libc/%.c $(SRC)/libc/%.h
	$(CC) $(FLAGS) -o $@ $<
$(BLD)/%.o: $(SRC)/core/%.c $(SRC)/core/%.h
	$(CC) $(FLAGS) -o $@ $<
$(BLD)/%.o: $(SRC)/paging/%.c $(SRC)/paging/%.h
	$(CC) $(FLAGS) -o $@ $<
$(BLD)/%.o: $(SRC)/cpu/%.c $(SRC)/cpu/%.h
	$(CC) $(FLAGS) -o $@ $<
$(BLD)/%.o: $(SRC)/drivers/%.c $(SRC)/drivers/%.h
	$(CC) $(FLAGS) -o $@ $<
$(BLD)/%.o: $(SRC)/kernel/%.c
	$(CC) $(FLAGS) -o $@ $<
$(BLD)/%.o: $(SRC)/%.c
	$(CC) $(FLAGS) -o $@ $<

$(BLD):
	mkdir $(BLD)
$(BOOT):
	mkdir -p $(BOOT)/grub
