make
# Загрузчик
nasm -f bin src/bootloader/boot.asm -o boot.bin
nasm -f bin src/bootloader/stage2.asm -o rootFS/stage2.bin
# Драйвера
gcc -m32 -ffreestanding -c src/drivers/video/vga_640_480.c -w -o vga_640_480.o
gcc -m32 -ffreestanding -c src/drivers/video/vga_80_25.c -w -o vga_80_25.o
gcc -m32 -ffreestanding -c src/drivers/video/video.c -w -o video.o
gcc -m32 -ffreestanding -c src/drivers/Keyboard/keyboard.c -w -o keyboard.o
gcc -m32 -ffreestanding -c src/drivers/disk/ata.c -w -o ata.o
gcc -m32 -ffreestanding -c src/drivers/disk/disk.c -w -o disk.o
# Процессор
gcc -m32 -ffreestanding -c src/cpu/idt.c -w -o idt.o
gcc -m32 -ffreestanding -c src/cpu/PIC.c -w -o pic.o
# Ядро
gcc -m32 -ffreestanding -c src/kernel/kernel.c -w -o kernel.o
# Сервисы ядра
gcc -m32 -ffreestanding -c src/kernel/services/services.c -w -o services.o
gcc -m32 -ffreestanding -c src/kernel/services/memory/memory.c -w -o memory.o
gcc -m32 -ffreestanding -c src/kernel/services/memory/program.c -w -o program.o
gcc -m32 -ffreestanding -c src/kernel/services/syscall/syscall.c -w -o syscall.o
gcc -m32 -ffreestanding -c src/drivers/fs/fs.c -w -o fs.o
gcc -m32 -ffreestanding -c src/drivers/fs/afs.c -w -o afs.o

# Double fault
gcc -m32 -ffreestanding -c src/kernel/services/panic/panic.c -w -o panic.o

# Прерывания
nasm -f elf32 src/interrupts/isr8.asm -o isr8.o
nasm -f elf32 src/interrupts/isr33.asm -o isr33.o
nasm -f elf32 src/interrupts/isr46.asm -o isr46.o
nasm -f elf32 src/interrupts/isr80.asm -o isr80.o
# Склеить все файлы в ядро
ld -m elf_i386 -T linker.ld kernel.o vga_640_480.o vga_80_25.o video.o keyboard.o ata.o disk.o idt.o pic.o isr8.o isr33.o isr46.o isr80.o panic.o memory.o syscall.o afs.o fs.o program.o -w -o kernel.elf

objcopy -O binary kernel.elf rootFS/kernel.bin

cd program
cd shell
make
make clean
cd ..
cd init
make
make clean
cd ..
cd setup
make
make clean
cd ..
cd NotePad
make
make clean
cd ..
cd ..

./utilities/afsm -c atom.img
./utilities/afsm -boot atom.img boot.bin
./utilities/afsm -push atom.img rootFS/stage2.bin
./utilities/afsm -push atom.img rootFS/kernel.bin
./utilities/afsm -push atom.img rootFS/shell.bin
./utilities/afsm -push atom.img rootFS/LICENSE.txt
./utilities/afsm -push atom.img rootFS/hello.bin
./utilities/afsm -push atom.img rootFS/init.bin
./utilities/afsm -push atom.img rootFS/init.cfg
./utilities/afsm -push atom.img rootFS/setup.bin
./utilities/afsm -push atom.img rootFS/notepad.bin

qemu-system-i386 -drive format=raw,file=atom.img -m 4M -icount shift=6,sleep=off -rtc clock=vm -no-reboot 

rm binaries/boot.bin
rm binaries/stage2.bin

make clean
rm *.o
rm *bin
rm rootFS/*bin
rm *elf
