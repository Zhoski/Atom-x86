TARGET = ../../rootFS/init.bin

$(TARGET) : init.elf
	objcopy -O binary init.elf $(TARGET)

init.elf: init.o strio.o file.o entry.o
	ld -m elf_i386 -T ../general/user.ld entry.o strio.o file.o init.o -o init.elf

init.o: init.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c init.c -o init.o

entry.o: ../general/entry.asm
	nasm -f elf32 ../general/entry.asm -o entry.o

file.o: ../libs/file.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c ../libs/file.c -o file.o

strio.o: ../libs/strio.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c ../libs/strio.c -o strio.o

clean :
	rm *.o *.bin *.elf