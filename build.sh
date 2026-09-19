make
# Загрузчик
nasm -f bin src/bootloader/boot.asm -o boot.bin
nasm -f bin src/bootloader/stage2.asm -o rootFS/stage2.bin
# ==============================================================================
# ДРАЙВЕРЫ 
# ==============================================================================
gcc -m32 -ffreestanding -c src/drivers/video/vga_640_480.c -w -o vga_640_480.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/drivers/video/vga_80_25.c -w -o vga_80_25.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/drivers/video/video.c -w -o video.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/drivers/keyboard/keyboard.c -w -o keyboard.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/drivers/disk/ata.c -w -o ata.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/drivers/disk/sata.c -w -o sata.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include


gcc -m32 -ffreestanding -c src/drivers/disk/disk.c -w -o disk.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/drivers/timer/timer.c -w -o timer.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/drivers/pci/pci.c -w -o pci.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

# ==============================================================================
# ПРОЦЕССОР И ПРЕРЫВАНИЯ
# ==============================================================================
gcc -m32 -ffreestanding -c src/cpu/idt.c -w -o idt.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/cpu/pic.c -w -o pic.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include


nasm -f elf32 src/interrupts/isr8.asm -o isr8.o
nasm -f elf32 src/interrupts/isr32.asm -o isr32.o
nasm -f elf32 src/interrupts/isr33.asm -o isr33.o
nasm -f elf32 src/interrupts/isr46.asm -o isr46.o
nasm -f elf32 src/interrupts/isr80.asm -o isr80.o

# ==============================================================================
# ЯДРО И ВНУТРЕННИЕ СЕРВИСЫ
# ==============================================================================
gcc -m32 -ffreestanding -c src/kernel/kernel.c -w -o kernel.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/kernel/services/services.c -w -o services.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/kernel/services/memory/memory.c -w -o memory.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/kernel/services/memory/program.c -w -o program.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/kernel/services/syscall/syscall.c -w -o syscall.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/kernel/services/panic/panic.c -w -o panic.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/fs/fs.c -w -o fs.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

gcc -m32 -ffreestanding -c src/fs/afs.c -w -o afs.o \
    -I./src/cpu/include -I./src/drivers/include -I./src/kernel/include -I./src/lib/include -I./src/fs/include

# ==============================================================================
# ЛИНКОВКА 
# ==============================================================================
ld -m elf_i386 -T linker.ld kernel.o services.o vga_640_480.o vga_80_25.o video.o keyboard.o \
                 ata.o sata.o disk.o timer.o pci.o idt.o pic.o isr8.o isr32.o isr33.o isr46.o isr80.o panic.o memory.o \
                 syscall.o afs.o fs.o program.o -w -o kernel.elf

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
cd notepad
make
make clean
cd ..
cd ..

./utilities/afsm -c atom9.img
./utilities/afsm -boot atom9.img boot.bin
./utilities/afsm -push atom9.img rootFS/stage2.bin
./utilities/afsm -push atom9.img rootFS/kernel.bin
./utilities/afsm -push atom9.img rootFS/shell.bin
./utilities/afsm -push atom9.img rootFS/LICENSE.txt
./utilities/afsm -push atom9.img rootFS/hello.bin
./utilities/afsm -push atom9.img rootFS/init.bin
./utilities/afsm -push atom9.img rootFS/init.cfg
./utilities/afsm -push atom9.img rootFS/setup.bin
./utilities/afsm -push atom9.img rootFS/notepad.bin

qemu-system-x86_64 -m 16M -no-reboot \
                   -device ich9-ahci,id=ahci \
                   -drive file=atom9.img,if=none,id=disk \
                   -device ide-hd,bus=ahci.0,drive=disk


rm binaries/boot.bin
rm binaries/stage2.bin

make clean
rm *.o
rm *bin
rm rootFS/*bin
rm *elf
