TARGET = shell.bin

$(TARGET) : shell.elf
	objcopy -O binary shell.elf $(TARGET)

shell.elf : entry.o shell.o strio.o string.o file.o memory.o
	ld -m elf_i386 -T user.ld entry.o strio.o string.o file.o memory.o shell.o -o shell.elf

shell.o : program/shell.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c program/shell.c -o shell.o

entry.o: program/entry.asm
	nasm -f elf32 program/entry.asm -o entry.o

strio.o: libs/strio.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c libs/strio.c -o strio.o

string.o: libs/string.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c libs/string.c -o string.o

file.o: libs/file.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c libs/file.c -o file.o

memory.o: libs/memory.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c libs/memory.c -o memory.o

clean :
	rm $(TARGET) *.o *.bin