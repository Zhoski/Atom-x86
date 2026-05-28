TARGET = shell.bin

$(TARGET) : shell.elf
	objcopy -O binary shell.elf $(TARGET)

shell.elf : entry.o shell.o libio.o string.o
	ld -m elf_i386 -T user.ld entry.o libio.o string.o shell.o -o shell.elf

shell.o : program/shell.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c program/shell.c -o shell.o

entry.o: program/entry.asm
	nasm -f elf32 program/entry.asm -o entry.o

libio.o: libs/libio.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c libs/libio.c -o libio.o

string.o: libs/string.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c libs/string.c -o string.o

clean :
	rm $(TARGET) *.o *.bin