# Загрузчик
nasm -f bin src/bootloader/boot.asm -o boot.bin
nasm -f bin src/bootloader/table.asm -o table.bin
nasm -f bin src/bootloader/stage2.asm -o stage2.bin

# Драйвера
gcc -m32 -ffreestanding -c src/drivers/vga.c -o vga.o
gcc -m32 -ffreestanding -c src/drivers/keyboard.c -o keyboard.o

# Процессор
gcc -m32 -ffreestanding -c src/cpu/idt.c -o idt.o
gcc -m32 -ffreestanding -c src/cpu/PIC.c -o pic.o
# Ядро
gcc -m32 -ffreestanding -c src/kernel/kernel.c -o kernel.o
# Сервисы ядра
gcc -m32 -ffreestanding -c src/kernel/services/Memory/pagging.c -o pagging.o

# Прерывания
nasm -f elf32 src/interrupts/isr33.asm -o isr33.o


# Склеить все файлы в ядро
ld -m elf_i386 -T linker.ld kernel.o vga.o keyboard.o idt.o pic.o isr33.o pagging.o -o kernel.elf

objcopy -O binary kernel.elf kernel.bin

dd if=/dev/zero of=disk.img bs=512 count=2048
dd if=boot.bin of=disk.img bs=512 seek=0 count=1 conv=notrunc
dd if=stage2.bin of=disk.img bs=512 seek=1 count=6 conv=notrunc
dd if=config.bin of=disk.img bs=512 seek=8 count=1 conv=notrunc
dd if=kernel.bin of=disk.img bs=512 seek=10 conv=notrunc

#qemu-system-x86_64 -hda disk.img -m 16M
qemu-system-x86_64 -drive format=raw,file=disk.img -m 16M

rm kernel.bin
rm boot.bin
rm stage2.bin
rm vga.o
rm kernel.o
rm kernel.elf
