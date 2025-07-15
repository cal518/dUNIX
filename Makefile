# Compiladores e ferramentas
CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy

# Flags de compilação
CFLAGS = -ffreestanding -O2 -Wall -Wextra -m64
LDFLAGS = -T link.ld

# Fontes e objetos
SRC_C = kernel.c
SRC_ASM = kernel_entry.asm outb.asm
OBJ = kernel.o kernel_entry.o outb.o

# Alvo principal
all: iso

# Compila o código C
kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

# Compila os ASM
kernel_entry.o: kernel_entry.asm
	$(AS) -f elf64 kernel_entry.asm -o kernel_entry.o

outb.o: outb.asm
	$(AS) -f elf64 outb.asm -o outb.o

# Linka tudo no kernel final
Mach_kernel: $(OBJ)
	$(LD) $(LDFLAGS) -o Mach_kernel $(OBJ)

# Cria ISO com GRUB e o kernel final
iso: Mach_kernel
	mkdir -p iso/boot/grub
	cp grub.cfg iso/boot/grub/
	cp Mach_kernel iso/boot/
	grub-mkrescue -o dfs64.iso iso

# Roda com QEMU
run: iso
	qemu-system-x86_64 -cdrom dfs64.iso

# Limpa tudo
clean:
	rm -f *.o *.elf Mach_kernel dfs64.iso
	rm -rf iso
