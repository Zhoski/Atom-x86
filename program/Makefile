TARGET = ../binaries/hello.bin

$(TARGET) : hello.elf
	objcopy -O binary hello.elf $(TARGET)

hello.elf : entry.o hello.o strio.o
	ld -m elf_i386 -T ../user.ld entry.o strio.o hello.o -o hello.elf

hello.o : hello.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c hello.c -o hello.o

entry.o: entry.asm
	nasm -f elf32 entry.asm -o entry.o

strio.o: ../libs/strio.c
	gcc -m32 -ffreestanding -fno-pic -O1 -c ../libs/strio.c -o strio.o

clean :
	rm *.o *.bin