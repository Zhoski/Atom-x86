TARGET = ../../rootFS/setup.bin

$(TARGET) : setup.elf
	objcopy -O binary setup.elf $(TARGET)

setup.elf: setup.o strio.o file.o entry.o
	ld -m elf_i386 -T ../general/user.ld entry.o strio.o file.o setup.o -o setup.elf

setup.o: setup.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c setup.c -o setup.o

entry.o: ../general/entry.asm
	nasm -f elf32 ../general/entry.asm -o entry.o

file.o: ../libs/file.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c ../libs/file.c -o file.o

strio.o: ../libs/strio.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c ../libs/strio.c -o strio.o

clean :
	rm *.o *.bin *.elf