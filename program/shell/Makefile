TARGET = ../../rootFS/shell.bin

$(TARGET) : shell.elf
	objcopy -O binary shell.elf $(TARGET)

shell.elf : entry.o shell.o strio.o string.o file.o mmemory.o
	ld -m elf_i386 -T ../general/user.ld entry.o strio.o string.o file.o mmemory.o shell.o -o shell.elf

shell.o : shell.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c shell.c -o shell.o

entry.o: ../general/entry.asm
	nasm -f elf32 ../general/entry.asm -o entry.o

strio.o: ../libs/strio.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c ../libs/strio.c -o strio.o

string.o: ../libs/string.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c ../libs/string.c -o string.o

file.o: ../libs/file.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c ../libs/file.c -o file.o

mmemory.o: ../libs/memory.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c ../libs/memory.c -o mmemory.o

clean :
	rm *.o *.bin *.elf