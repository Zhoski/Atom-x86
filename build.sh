make
# Загрузчик
nasm -f bin src/bootloader/boot.asm -o boot.bin
nasm -f bin src/bootloader/stage2.asm -o binaries/stage2.bin
# Драйвера
gcc -m32 -march=i486 -ffreestanding -c src/drivers/video/video.c -w -o video.o
gcc -m32 -march=i486 -ffreestanding -c src/drivers/Keyboard/keyboard.c -w -o keyboard.o
gcc -m32 -march=i486 -ffreestanding -c src/drivers/disk/pata.c -w -o pata.o
# Процессор
gcc -m32 -march=i486 -ffreestanding -c src/cpu/idt.c -w -o idt.o
gcc -m32 -march=i486 -ffreestanding -c src/cpu/PIC.c -w -o pic.o
# Ядро
gcc -m32 -march=i486 -ffreestanding -c src/kernel/kernel.c -w -o kernel.o
# Сервисы ядра
gcc -m32 -march=i486 -ffreestanding -c src/kernel/services/memory/memory.c -w -o memory.o
gcc -m32 -march=i486 -ffreestanding -c src/kernel/services/memory/program.c -w -o program.o
gcc -m32 -march=i486 -ffreestanding -c src/kernel/services/syscall/syscall.c -w -o syscall.o
gcc -m32 -march=i486 -ffreestanding -c src/drivers/fs/fat16.c -w -o fs.o

# Прерывания
nasm -f elf32 src/interrupts/isr33.asm -o isr33.o
nasm -f elf32 src/interrupts/isr46.asm -o isr46.o
nasm -f elf32 src/interrupts/isr80.asm -o isr80.o
# Склеить все файлы в ядро
ld -m elf_i386 -T linker.ld kernel.o video.o keyboard.o pata.o idt.o pic.o isr33.o isr46.o isr80.o memory.o syscall.o fs.o program.o -w -o kernel.elf

objcopy -O binary kernel.elf binaries/kernel.bin

python3 afs-pack.py

qemu-system-i386 -cpu 486 -drive format=raw,file=atom.img -m 4M -icount shift=6,sleep=off -rtc clock=vm


rm binaries/boot.bin
rm binaries/stage2.bin

make clean
rm *bin
rm binaries/*bin
rm *elf
