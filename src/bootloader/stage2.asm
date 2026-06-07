; =============================================================================
;  Atom OS Bootloader (Stage 1 / MBR)
;  Copyright (C) 2026 [Zhoski]. All rights reserved.
;
;  File: stage2.asm
;  Description: Loading the kernel, switching to PM, getting system information
; =============================================================================
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

    mov si, bootMsg
    call print
    
    call get_memmap

    call kernel_load    ; Загрузка ядра 

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

    ; Записываем в bootInfo количество свободной оперативной памяти
    xor ax, ax
    mov es, ax
    mov bx, bootInfo                
    mov ax, [total_usable_ram_k]
    mov [es:bx], ax

    ; Записываем в bootInfo где искать карту памяти для ядра
    mov ax, [memmap_segment]                   
    mov [es:bx+2], ax
    
    mov ax, [memmap_buffer]
    mov [es:bx+4], ax

    popa
    ret
    

; ============================== Диск ================================

; Загрузка секторов в оперативку
; Вход es:bx al, cl
disk_read:
    pusha

    mov ah, 0x42
    mov dl, [drive]

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
; Грузим 16 секторов начиная со 2-го по адресу 0x0000:0x0500
LOAD_ROOT_TO_MEM:
    pusha

    mov ah, 0x42
    mov si, LBA_ROOT
    mov dl, [drive]
    int 0x13

    popa
    ret

.error:
    mov si, disk_read_error
    call print
    jmp $


OPEN_FILE:
    call LOAD_ROOT_TO_MEM

    ; ES:BX на начало таблицы в памяти
    xor ax, ax
    mov es, ax
    mov bx, [LoadRootAddres]
    
    mov dx, 512     ; Максимальное количество файлов

.FIND_FILE_LOOP:
    test dx, dx
    jz .FILE_NOT_FOUND

    mov di, bx

    push si
    mov cx, 11
    repe cmpsb
    pop si

    jz .FOUND

    dec dx
    add bx, 16

    jmp .FIND_FILE_LOOP

.FILE_NOT_FOUND:
    mov ax, 1           ; Код: файл не найден
    ret

.FOUND:
    mov ax, [es:bx + 13]
    mov bx, [es:bx + 11]

    mov word [LBA_FILE],    0x0010
    mov word [LBA_FILE+2],      ax
    mov word [LBA_FILE+8],      bx
    mov word [LBA_FILE+10], 0x0000

    mov ax, 0           ; Код: Файл найден

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
    db 0x00
    dw 16
    dw 0x0500
    dw 0x0000
    dq 2


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

; =========================== Переменные =============================
file_found: db "File found",13,10,0
file_not_found: db "File not found",13,10,0
kernel_image: db "KERNEL  BIN",0
; Адреса

; E820 карта памяти
memmap_buffer: dw 0x0500
memmap_segment: dw 0x0000

memmap_block_count: dw 0
total_usable_ram_k: dw 0

; Системные сообщения
get_memmap_msg: db "Getting memory map ",13,10,0
memmap_total_block_msg: db "Total block [ ",0
memmap_total_ram_usable:db "Total usable ram [ ",0

disk_error_msg: db "> Disk load error",13,10,0
disk_load_error: db "Disk load error",13,10,0
disk_write_error: db "Disk write error",13,10,0

; Статусы
ok_msg:   db "[  OK  ] ",0
fail_msg: db "[ FAIL ] ",0
info_msg: db "[ INFO ] ",0

kernel_load_status: db "Kernel load ",13,10,0
kernel_load_error:  db "Kernel load error: ",0
kernel_signature_error: db "Kernel is damaged",13,10,0
kernel_not_found_error: db "KERNEL.BIN not found on disk",13,10,0
kernel_signature_addit: db "Additionaly: Kernel file must begin with the signature 0xAABB"

reboot_msg: db "Press any key to reboot...",0
continue_msg: db "Press any key to continue...",0

; Ошибки 
disk_read_error:  db "Disk read error: ",0
disr_write_error: db "Disk write error: ",0

; Кода ошибок
disk_time_out:          db "Time out",13,10,0
disk_sector_not_found:  db "Sector not found",13,10,0
disk_seek_failed:       db "Seek Failed",13,10,0
disk_invalid_op:        db "Invalid function or invalid parameter",13,10,0
drive: db 0x80

RootStartSector: db 2
RootSectors:     db 16 
DataStartSector: db 18
LoadRootAddres:  dw 0x500

new_string: db 13,10,0
tab:    db " ",0
parren_close: db " ]",13,10,0

bios_error_msg: db "Bios error",13,10,0
bootMsg: db "================================ ATOM-BOOT v0.1 ================================",13,10,0

; Массив для информации о системе
bootInfo: times 256 dw 0

section .data

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
    dw 0xFFFF       ; Limit Low
    dw 0x0000       ; Base Low
    db 0x00         ; Base Mid
    db 0b10010010   ; Acces
    db 0b11001111   ; Flags
    db 0x00         ; Base High
    

gdt_end:    

gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; ========================= Ядро =====================================
kernel_load:
    mov si, kernel_image
    call OPEN_FILE

    test ax, ax
    jnz .kernel_not_found

    mov word [LBA_FILE+4], 0x0000
    mov word [LBA_FILE+6], 0x1000

    mov si, LBA_FILE
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

    mov si, fail_msg
    call print

    mov si, kernel_load_error
    call print 

    mov si, kernel_signature_error
    call print

    call REBOOT

    jmp $

    ret

; Загрузка ядра
kernel_launch:
    ; Копируем bootInfo в адрес 0x1000
    xor ax, ax
    mov es, ax
    mov ds, ax

    mov si, bootInfo
    mov di, 0x1000
    mov cx, 256

    cld

    repe movsw

    jmp .after

.after:
    ; Переключение видеорежима на 640x480 16 цветов 
    mov ah, 0x00
    mov al, 0x12
    int 0x10 

.skip:

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
    
    ;call kmain
    jmp 0x8:0x100000