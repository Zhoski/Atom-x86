make
# Загрузчик
nasm -f bin src/bootloader/boot.asm -o boot.bin
nasm -f bin src/bootloader/stage2.asm -o stage2.bin
# Драйвера
gcc -m32 -ffreestanding -c src/drivers/video/video.c -w -o video.o
gcc -m32 -ffreestanding -c src/drivers/Keyboard/keyboard.c -w -o keyboard.o
gcc -m32 -ffreestanding -c src/drivers/disk/pata.c -w -o pata.o
# Процессор
gcc -m32 -ffreestanding -c src/cpu/idt.c -w -o idt.o
gcc -m32 -ffreestanding -c src/cpu/PIC.c -w -o pic.o
# Ядро
gcc -m32 -ffreestanding -c src/kernel/kernel.c -w -o kernel.o
# Сервисы ядра
gcc -m32 -ffreestanding -c src/kernel/services/memory/memory.c -w -o memory.o
gcc -m32 -ffreestanding -c src/kernel/services/memory/process.c -w -o process.o
gcc -m32 -ffreestanding -c src/kernel/services/memory/program.c -w -o program.o
gcc -m32 -ffreestanding -c src/kernel/services/memory/allocate.c -w -o allocate.o
gcc -m32 -ffreestanding -c src/kernel/services/syscall/syscall.c -w -o syscall.o
gcc -m32 -ffreestanding -c src/drivers/fs/fat16.c -w -o fs.o


# Прерывания
nasm -f elf32 src/interrupts/isr33.asm -o isr33.o
nasm -f elf32 src/interrupts/isr46.asm -o isr46.o
nasm -f elf32 src/interrupts/isr80.asm -o isr80.o
# Программы
#nasm -f bin program/shell.asm -o shell.bin

# Склеить все файлы в ядро
ld -m elf_i386 -T linker.ld kernel.o video.o keyboard.o pata.o idt.o pic.o isr33.o isr46.o isr80.o allocate.o memory.o process.o syscall.o fs.o program.o -w -o kernel.elf

objcopy -O binary kernel.elf kernel.bin

dd if=/dev/zero of=atom_os.img bs=512 count=32768
dd if=boot.bin of=atom_os.img bs=512 seek=0 count=1 conv=notrunc

mcopy -i atom_os.img stage2.bin ::/BOOT.BIN
mcopy -i atom_os.img kernel.bin ::/KERNEL.BIN
mcopy -i atom_os.img shell.bin  ::/SHELL.BIN
mcopy -i atom_os.img LICENSE.txt   ::/LICENSE.TXT

qemu-system-x86_64 -drive format=raw,file=atom_os.img -m 16M

make clean
