; =====================================================================
;  Atom OS Bootloader (Stage 1 / MBR)
;  Copyright (C) 2026 [Zhoski]. All rights reserved.
;
;  File: boot.asm
;  Description: AtomFS LBA block parser & Stage 2 loader
; =====================================================================


bits 16
org 0x7C00

start:
    ; Сегменты
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00
    
    mov [drive], dl

    ; Видеорежим
    mov ah, 0x0
    mov al, 0x3
    int 0x10

    mov si, atom_boot
    call print_string

    call OPEN_FILE

    jmp $

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

REBOOT:
    mov si, reboot_msg
    call print_string

    mov ah, 0x00
    int 0x16

    int 0x19

; Грузим 16 секторов начиная со 2-го по адресу 0x0000:0x0500
LOAD_ROOT_TO_MEM:
    pusha

    mov ah, 0x42
    mov si, lba
    mov dl, [drive]
    int 0x13

    popa
    ret

.error:
    mov si, disk_read_error
    call print_string
    jmp $


OPEN_FILE:
    call LOAD_ROOT_TO_MEM

    ; ES:BX на начало таблицы в памяти
    xor ax, ax
    mov es, ax
    mov bx, [LoadRootAddres]
    
    mov dx, 512     ; Максимальное количество файлов

    mov si, stage2_file

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
    mov si, file_not_found
    call print_string
    call REBOOT

.FOUND:
    mov ax, [es:bx + 13]
    mov dx, [es:bx + 11]

    mov word [lba],    0x0010
    mov word [lba+2],  ax
    mov word [lba+4],  0x8000
    mov word [lba+6],  0x0000 
    mov word [lba+8],  dx
    mov word [lba+10], 0x0000

    mov ah, 0x42
    mov si, lba
    mov dl, [drive]
    int 0x13

    jmp 0x0000:0x8000

    jmp $

lba:
    db 0x10
    db 0x00
    dw 16
    dw 0x0500
    dw 0x0000
    dq 2          

RootStartSector: db 2
RootSectors:     db 16 
DataStartSector: db 18

LoadRootAddres:  dw 0x500

drive: db 0

atom_boot: db "ATOM-x86 BOOT SECTOR",13,10,0
stage2_file: db "STAGE2  BIN",0
file_not_found: db "STAGE2.BIN not found on disk",13,10,0
file_found: db "File found",13,10,0
reboot_msg: db "Press any key to reboot...",0
disk_read_error: db "Disk read error",13,10,0

times 510 - ($ - $$) db 0
dw 0xAA55