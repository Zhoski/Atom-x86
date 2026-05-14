; =======================================================================
; stage2.bin - является основным загрузчиком чья задача подготовить почву
; для загрузки ядра. На данный момент загрузчик имеет обработчики исключе
; ний, работу с диском, получение карты памяти, работа с config.cfg. Загр
; узчик переключается в защищенный режим и передает управление ядру. Сраз
; у хочу сказать что на старте ядро лежит на 0x10000:0x0000. После перехо
; да в защищенный режим ядро копируется в 0x100000.
; =======================================================================
bits 16
org 0x8000

; ========================= Инициализация ==========================

start:
    cli
    ; Установка сегментных регистров
    mov ax, 0x0000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00      ; Стек под загрузчик

    in al, 0x92
    or al, 2
    out 0x92, al        ; A20 line включить 

    ; Установка видеорежима VGA TEXT 80x25 16 цветов
    mov ah, 0x00         
    mov al, 0x03
    int 0x10  
    
    call get_memmap

    call INIT_FAT  

    call kernel_load    ; Загрузка ядра 
    ;mov si, kernel_image
    ;call FIND_FILE
    
    call CONTINUE

    call kernel_launch
    
    jmp $          

; ============================ Память ================================
get_memmap:
    pusha               ; Регистры запомнить 

    mov si, ok_msg
    call print

    mov si, get_memmap_msg
    call print
    
    mov es, [memmap_segment]    ; 0x0000
    mov di, [memmap_buffer]     ; 0x0500
    xor ebx, ebx            ; ebx обнулить

.next_entry:
    mov eax, 0xE820
    mov edx, 0x534D4150
    mov ecx, 24        
    int 0x15
    
    jc .done       
    add di, 24          
        
    mov cx, [memmap_block_count]
    inc cx
    mov [memmap_block_count], cx

    test ebx, ebx       
    jz .done          
    jmp .next_entry     

.done:  
    ; Уменьшить на 1 потому что какой то баг выдает на 1 сектор больше
    mov cx, [memmap_block_count]
    dec cx
    mov [memmap_block_count], cx    

    mov si, info_msg
    call print

    mov si, memmap_total_block_msg
    call print

    mov si, memmap_block_count
    call print

    mov ax, [memmap_block_count]
    call print_ax

    mov si, parren_close
    call print

.get_usable_ram:
    ; Читать из 0x0000:0x0500
    mov bx, [memmap_buffer]
    mov es, [memmap_segment]
    xor si, si          ; Счетчик

.ram_loop:
    cmp si, [memmap_block_count]
    jz .done_read

    mov eax, [es:bx+16]
    cmp eax, 1
    jnz .skip

    mov eax, [es:bx+8]
    shr eax, 10

    add [total_usable_ram_k], eax

.skip:
    inc si
    add bx, 24
    jmp .ram_loop

.done_read:
    mov si, info_msg
    call print

    mov si, memmap_total_ram_usable
    call print

    mov ax, [total_usable_ram_k]
    call print_ax

    mov ah, 0x0E
    mov al, 'k'
    int 0x10

    mov si, parren_close
    call print

    popa
    ret

; ============================== Диск ================================

; Загрузка секторов в оперативку
; Вход es:bx al, cl
disk_read:
    pusha

    mov ah, 0x42
    mov dl, 0x80

    int 0x13

    jc .disk_read_error

    popa
    ret

.disk_read_error:
    mov si, disk_read_error
    call print

    cmp ah, 0x01
    je .invalid_op

    cmp ah, 0x04
    je .sector_not_found

    cmp ah, 0x40
    je .seek_failed

    cmp ah, 0x80
    je .time_out

.invalid_op:
    mov si, disk_invalid_op
    ;call print

    jmp .disk_read_exit

.time_out:
    mov si, disk_time_out

    jmp .disk_read_exit

.sector_not_found:
    mov si, disk_sector_not_found

    jmp .disk_read_exit

.seek_failed:
    mov si, disk_seek_failed

.disk_read_exit:
    call print

    popa
    ret

; Загрузка диска из оперативки
; Вход es:bx al, cl
disk_write:
    pusha
    
    mov ah, 0x43
    mov dl, 0x80
    int 0x13

    jc .disk_write_error

    popa
    ret

.disk_write_error
    mov si, disk_write_error
    ;call print
    
    popa
    ret

; =========================== Система в/в =============================
print:
    pusha
.loop:
    lodsb
    or al, al
    jz .exit
    mov ah, 0x0E
    int 0x10
    jmp .loop

.exit:
    popa
    ret

print_hex_32:
    pushad
    mov edx, eax        ; Копируем число в EDX, его портить не будем
    mov cx, 8           ; 8 цифр
.loop:
    rol edx, 4          ; Вращаем EDX!
    mov al, dl          ; Берем младшие 4 бита из EDX (после вращения они там)
    and al, 0x0F        ; Маска
    
    cmp al, 10
    jl .is_digit
    add al, 7
.is_digit:
    add al, '0'
    
    mov ah, 0x0E        ; Теперь AX можно портить
    mov bx, 0x0007      ; Режим: страница 0, цвет (для графики)
    int 0x10
    
    loop .loop
    popad
    ret

print_ax:
    pusha
    mov cx, 10
    xor bx, bx

.split_loop:
    xor dx, dx
    div cx
    add dl, '0'
    push dx
    inc bx
    test ax, ax
    jnz .split_loop

.print_loop:
    pop ax
    mov ah, 0x0E
    int 0x10
    dec bx
    jnz .print_loop

.print_done:
    popa
    ret

compare_strings:
    pusha
    xor cx, cx

.next_char:
    lodsb
    cmp al, [di]
    jne .not_equal
    cmp al, 0
    je .equal
    inc di
    jmp .next_char

.not_equal:
    popa
    ret

.equal:
    popa
    ret

; ================= Драйвер для работы с файлами ==================

INIT_FAT:
    ; Грузим загрузочный сектор по 0x7E00
    mov word [LBA_FILE+2], 1
    mov word [LBA_FILE+4], 0x7E00
    mov word [LBA_FILE+6], 0x0000
    mov word [LBA_FILE+8], 0

    mov si, LBA_FILE
    call disk_read

    ; Теперь в 0x7E00 лежит загрузочный сектор
    
    mov ax, word  [es:bx+0xB]
    mov [BPB_BytsPerSec],  ax
    mov al, byte  [es:bx+0xD]
    mov [BPB_SecPerClus],  al
    mov ax, word  [es:bx+0xE]
    mov [BPB_RsvdSecCnt], ax
    mov al, byte [es:bx+0x10]
    mov [BPB_NumFATs],     al
    mov ax, word [es:bx+0x11]
    mov [BPB_RootEntCnt],  ax
    mov ax, word [es:bx+0x13]
    mov [BPB_TotSec16],    ax
    mov ax, word [es:bx+0x16]
    mov [BPB_FATSz16],     ax

    mov ax, [BPB_RsvdSecCnt]
    mov [FatStartSector], ax

    mov ax, [BPB_FATSz16]
    movzx cx, byte [BPB_NumFATs]
    mul cx
    mov [FatSectors], ax

    mov ax, [FatSectors]
    add ax, [FatStartSector]
    mov [RootDirStartSector], ax

    mov ax, [BPB_RootEntCnt]
    mov cx, 32
    mul cx

    add ax, [BPB_BytsPerSec]
    sub ax, 1
    
    mov cx, [BPB_BytsPerSec]
    div cx
    mov [RootDirSectors], ax

    mov ax, [RootDirStartSector]
    add ax, [RootDirSectors]
    mov [DataStartSector], ax 

    mov ax, [BPB_TotSec16]
    sub ax, [DataStartSector]
    mov [DataSectors], ax
    
    mov ax, [RootDirStartSector]
    mov word [LBA_ROOT+8], ax

    mov word [LBA_ROOT+10], 0 

    ret

; Грузит 1 сектор рут каталога по 0x7E00
LOAD_ROOT: 
    push si
    push ax
    
    mov ax, [RootDirStartSector]
    add ax, [RootDirSectorOff]
    mov word [LBA_ROOT+8], ax

    mov si, LBA_ROOT
    call disk_read
    
    pop  ax
    pop  si
    ret

; Вход: si (Имя файла)
; Выход: si (Указатель на таблицу LBA, если файл найден), ax (Код) 
; ax = 0 (Файл найден)
; ax = 1 (Файл не найден)
FIND_FILE:
    push bx
    push cx
    push dx
    push di
    push fs
    
    call LOAD_ROOT

    mov bx, 0x7E00
    mov dx, [BPB_RootEntCnt]

    xor ax, ax
    mov es, ax
    mov ds, ax
    mov fs, ax

.FIND_LOOP:
    test dx, dx
    jz .NOT_FOUND 

    push ax
    mov al, [ToLoadRootSector]
    cmp al, 16
    pop ax

    jz .LOAD_NEXT_ROOT_SECTOR
    jmp .SKIP_LOAD

.LOAD_NEXT_ROOT_SECTOR:
    pusha
    xor ax, ax
    mov [ToLoadRootSector], al

    mov ax, [RootDirSectorOff]
    inc ax
    mov [RootDirSectorOff], ax

    call LOAD_ROOT

    popa

.SKIP_LOAD:
    mov di, bx

    push si
    mov cx, 11
    repe cmpsb
    pop si

    jz .FOUND

    dec dx
    add bx, 32

    push ax
    mov ax, [ToLoadRootSector]
    inc ax
    mov [ToLoadRootSector], ax
    pop ax

    jmp .FIND_LOOP

.FOUND:
    ;mov si, file_found
    ;call print

    xor ax, ax
    mov [ToLoadRootSector], ax
    mov [RootDirSectorOff], ax

    mov ax, word [es:bx+0x1A]       ; Записываем в ax начальный кластер файла
    sub ax, 2
    movzx cx, byte [BPB_SecPerClus]
    mul cx
    add ax, [DataStartSector]       ; Теперь в ax номер начального сектора

    mov word [FirstSectorofCluster], ax

    mov ax, word [es:bx+0x1C]       ; Записываем в ax размер файла в байтах
    mov cx, [BPB_BytsPerSec]
    div cx                          ; Делим ax на размер сектора

    test dx, dx                     ; Если dx не 0, то то округляем и добавляем 1 сектор
    jnz .add

    jmp .skip

.add:
    add ax, 1

.skip:
    mov [FileSecSize], ax

    mov ax, [FileSecSize]
    mov [LBA_FILE+2], ax
    mov ax, [FirstSectorofCluster]
    mov [LBA_FILE+8], ax

    mov si, LBA_FILE

    movzx ax, [FoundCode]

    pop fs
    pop di
    pop dx
    pop cx
    pop bx
    ret

.NOT_FOUND:
    ;mov si, file_not_found
    ;call print

    movzx ax, [NotFoundCode]
    
    pop fs
    pop di
    pop dx
    pop cx
    pop bx
    ret

LBA_FILE:
    db 0x10
    db 0
    dw 0
    dw 0
    dw 0
    dq 0

LBA_ROOT:
    db 0x10
    db 0
    dw 1
    dw 0x7E00
    dw 0x0000
    dq 0


REBOOT:
    mov si, info_msg
    call print
    
    mov si, reboot_msg
    call print

    ; Курсор за видимость экрана
    mov ah, 0x02
    mov bh, 0
    mov dh, 26
    mov dl, 0
    int 0x10

    mov ah, 0x00
    int 0x16

    int 0x19

CONTINUE:
    mov si, info_msg
    call print

    mov si, continue_msg
    call print

    ; Курсор за видимость экрана
    mov ah, 0x02
    mov bh, 0
    mov dh, 26
    mov dl, 0
    int 0x10

    mov ah, 0x00
    int 0x16

    ret

RootDirSectorOff:       dw 0        
ToLoadRootSector:       db 0

FileSecSize:            dw 0        ; Размер файла в секторах
FirstSectorofCluster:   dw 0        ; Начальный сектор файла

BPB_BytsPerSec:         dw 0        ; Байт в секторе
BPB_SecPerClus:         db 0        ; Секторов в кластере
BPB_RsvdSecCnt:         dw 0        ; Зарезервировано секторов
BPB_NumFATs:            db 0        ; число таблиц FAT
BPB_RootEntCnt:         dw 0        ; Записей в рут каталоге
BPB_TotSec16:           dw 0        ; Секторов на диске
BPB_FATSz16:            dw 0        ; Размер одной таблицы FAT

RootDirStartSector:     dw 0
RootDirSectors:         dw 0 
FatSectors:             dw 0
FatStartSector:         dw 0
DataStartSector:        dw 0
DataSectors:            dw 0

NotFoundCode:           db 1
FoundCode:              db 0

; =========================== Переменные =============================
kernel: db "Kernel.bin",0
config: db "Config.cfg",0
shell:  db "Shell.bin", 0
table: db "table.bin",0
file_found: db "File found",13,10,0
file_not_found: db "File not found",13,10,0
kernel_image: db "KERNEL  BIN",0
; Адреса

; E820 карта памяти
memmap_buffer: dw 0x0500
memmap_segment: dw 0x0000

memmap_block_count: dw 0
total_usable_ram_k: dw 0

; Конфиг
config_segment: dw 0x0000
config_offset:  dw 0x1000

config_base: dw 0x1000

; Системные сообщения
msg: db "Bootloader stage 2 load ",0

get_cfg_msg: db "Getting config system ",0
cfg_error:   db "Config read error: ",0
cfg_error_signature: db "Config is missing or damaged",13,10,0
cfg_error_addit:     db "Additionally: The config file must begin with the signature 0xFF",13,10,0

get_memmap_msg: db "Getting memory map ",13,10,0
memmap_total_block_msg: db "Total block [ ",0
memmap_total_ram_usable:db "Total usable ram [ ",0
memmap_msg: db "  Base Address             Lenght             Type",13,10,0

help_msg: db    "> clear  --- reset screen",13,10,"> user   --- write current user",13,10,"> start --- load kernel",13,10,"> memmap --- write memory map",13,10,0

disk_error_msg: db "> Disk load error",13,10,0
disk_load_error: db "Disk load error",13,10,0
disk_write_error: db "Disk write error",13,10,0

; Статусы
ok_msg:   db "[  OK  ] ",0
fail_msg: db "[ FAIL ] ",0
info_msg: db "[ INFO ] ",0

kernel_load_msg: db "Load kernel from hard disk...",13,10,0
kernel_load_status: db "Kernel load ",13,10,0
kernel_load_error:  db "Kernel load error: ",0
kernel_signature_error: db "Kernel is missing or damaged",13,10,0
kernel_not_found_error: db "KERNEL.BIN not found on disk",13,10,0
kernel_signature_addit: db "Additionaly: Kernel file must begin with the signature 0xAABB"

reboot_msg: db "Press any key to reboot...",0
continue_msg: db "Press any key to continue...",0

; Ошибки 
disk_read_error:  db "Disk read error: ",0
disr_write_error: db "Disk write error: ",0

; Кода ошибок
disk_time_out:      db "Time out",13,10,0
disk_sector_not_found:  db "Sector not found",13,10,0
disk_seek_failed:   db "Seek Failed",13,10,0
disk_invalid_op:    db "Invalid function or invalid parameter",13,10,0

; Консоль команды
prompt: db "> ",0
hi_user: db "Hi ",0

k_msg: db "k",0
m_msg: db "M",0

new_string: db 13,10,0
tab:    db "      ",0

; Буфферы
c_buffer: times 64 db 0 
user_name: times 32 db 0
user_password: times 32 db 0

; Массив для информации о системе
bootInfo: times 256 dw 0
; Шрифт
;bios_font: times 1024 dq 0

section .data
; Command
c_help: db "help",0
c_clear: db "clear",0
c_user: db "user", 0
c_memmap: db "memmap",0
c_start:  db "start",0

; Специально
parren_close: db " ]",13,10,0

; ========================== DAP таблицы =============================
dap:
    db 0x10
    db 0x00
    dw 0
    dw 0x0000
    dw 0x0000
    dq 0

; ====================== Таблица десрипторов =========================
gdt_start:
    ; Null Descriptor
    dq 0x0000000000000000

    ; Kernel Mode Code Segment
    dw 0xFFFF   ; Limit Low
    dw 0x0000   ; Base Low
    db 0x00     ; Base Mid
    db 0b10011010   ; Acces
    db 0b11001111   ; Flags
    db 0x00     ; Base High

    ; Kernel Mode Data Segment
    dw 0xFFFF   ; Limit Low
    dw 0x0000   ; Base Low
    db 0x00     ; Base Mid
    db 0b10010010   ; Acces
    db 0b11001111   ; Flags
    db 0x00     ; Base High
    

gdt_end:    

gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; ========================= Ядро =====================================
; Загрузка ядра из 10 сектора в 0x10000
kernel_load:
    mov si, kernel_image
    call FIND_FILE

    test ax, ax
    jnz .kernel_not_found

    mov word [LBA_FILE+4], 0x0000
    mov word [LBA_FILE+6], 0x1000

    call disk_read

    mov bx, 0x0000
    mov ax, 0x1000
    mov es, ax

    mov ax, word [es:bx]
    cmp ax, 0xBBAA
    jnz .kernel_signature

    mov si, ok_msg
    call print
    mov si, kernel_load_status
    call print

    ret

.kernel_not_found:
    mov si, fail_msg
    call print

    mov si, kernel_load_status
    call print

    mov si, fail_msg
    call print

    mov si, kernel_not_found_error
    call print

    jmp REBOOT

.kernel_signature:
    mov si, fail_msg
    call print

    mov si, kernel_load_status
    call print

    mov si, kernel_load_error
    call print

    mov si, kernel_signature_error
    call print

    jmp $

    ret

; Загрузка ядра
kernel_launch:
    ; Передача всех данных ядру
    
    ; Загрузка конфигов в 0x0000:0x1000
    ;mov si, config
    ;call found_file
    ;call disk_read

    ; По памяти 0x0000:0x0500 уже лежит таблица с картой памяти
    ; В 0x0000:0x0F00 количество блоков от Е820
    ;xor ax, ax
    ;mov es, ax
    ;mov bx, 0xF00
    ;mov ax, [memmap_block_count]
    ;mov [es:bx], ax

    ; Теперь в 0x0000:0x0500 карта памяти 0x0000:0xF00 сколько блоков 0x0000:0x1000 конфиги
    ; Теперь ядро без суматохи прочитает эти адреса

; Переключение в защищенный режим
switch_to_PM:
    cli     ; офф прерывания

    lgdt [gdt_ptr]  ; gdt в проц
    mov eax, cr0    ; cr0 в 1
    or eax, 1
    mov cr0, eax
    jmp 0x8:PMentry ; Прыжок в защищенный режим

bits 32
PMentry: 
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov fs, ax
    mov es, ax
    mov gs, ax
    mov esp, 0x90000

    mov esi, 0x10000   
    mov edi, 0x100000   
    mov ecx, 16384     
    rep movsd  

    jmp 0x8:0x100000
