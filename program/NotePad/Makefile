TARGET = ../../rootFS/notepad.bin

$(TARGET) : notepad.elf
	objcopy -O binary notepad.elf $(TARGET)

notepad.elf : entry.o notepad.o strio.o string.o file.o mmemory.o
	ld -m elf_i386 -T ../general/user.ld entry.o strio.o string.o file.o mmemory.o notepad.o -o notepad.elf

notepad.o : notepad.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c notepad.c -o notepad.o

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