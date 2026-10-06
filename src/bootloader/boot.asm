; =====================================================================
;  Atom OS Bootloader (Stage 1 / MBR)
;  Copyright (C) 2026 [Zhoski]. All rights reserved.
;
;  File: boot.asm
;  Description: It locates stage2 on the disk and hands over control to it.
; =====================================================================


bits 16
org 0x7C00

start:
    ; Установка сегментов и стека
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00
    
    mov [drive], dl

    ; Установка видео режима vga 80x25
    mov ah, 0x0
    mov al, 0x3
    int 0x10

    mov si, atom_boot
    call print_string

    mov ah, 0x42
    mov si, lba
    mov dl, [drive]
    int 0x13

    ;mov ax, word [0x8000]

    ;cmp ax, 0xBBAA

    ;jnz error

    jmp 0x0000:0x8000

    jmp $

error:
    mov si, err
    call print_string
    jmp $

; Вывод строки
print_string:
    pusha
loop:
    lodsb
    or al, al
    jz exit
    mov ah, 0x0E
    int 0x10
    jmp loop

exit:
    popa
    ret

; Изначально тут рут директория фс
align 4
lba:
    db 0x10
    db 0x00
    dw 6
    dw 0x8000
    dw 0x0000
    dq 1          

RootStartSector: db 2       ; Начиная с этого сектора лежат описание файлов
RootSectors:     db 16      ; Сколько секторов выделено под описание файлов
DataStartSector: db 18      ; Начиная с этого сектора лежит содержимое файлов

LoadRootAddres:  dw 0x500   ; На этот адрес загружаются сектора с описанием файлов

drive: db 0

atom_boot: db "ATOM-x86 BOOT SECTOR",13,10,0
stage2_file: db "STAGE2  BIN",0
file_not_found: db "STAGE2.BIN not found on disk",13,10,0
file_found: db "STAGE2.BIN load",13,10,0
reboot_msg: db "Press any key to reboot...",0
disk_read_error: db "Disk read error",13,10,0
err: db "STAGE2.BIN LOAD ERROR",13,10,0

times 510 - ($ - $$) db 0
dw 0xAA55