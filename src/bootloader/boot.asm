; ===============================================================================
; boot.bin отвечает за загрузку stage2.bin (Основной загрузчик). Начальный загруз
; чик ищет на диске файл BOOT.BIN и загружает его по 0x0000:0x8000, после чего пр
; ыгает по этому адресу. Использует файловую систему FAT16
; ================================================================================

bits 16
org 0x7C00

; Перепрыгивание BPB
jmp short start
nop
db "ATOM-X86"           ; OEM identifier

; ================= BPB =================
BPB_BytsPerSec: dw 512                  ; 512 байт на сектор
BPB_SecPerClus: db 8                    ; 4 сектора на кластер
BPB_RsvdSecCnt: dw 1                    ; Зарезервировано под загрузчик
BPB_NumFATs:    db 2                    ; Будет две записи FAT
BPB_RootEntCnt: dw 256                  ; Записей в root каталоге
BPB_TotSec16:   dw 32768                ; 32768 сектора (16Мб)
BPB_Media:      db 0xF8                 ; Жетский диск
BPB_FATSz16:    dw 64                   ; Размер одной FAT
BPB_SecPerTrk:  dw 63                   ; Секторов на одной дорожке
BPB_NumHeads:   dw 255                  ; Головок на одной стороне
BPB_HiddSec:    dd 0
BPB_TotSec32:   dd 0

; =========== Extended BPB ==============
BS_DrvNum:      db 0x80                 ; Номер диска
BS_Reserved1:   db 0x00                 ; Зарезервировано
BS_BootSig:     db 0x29                 ; Сигнатура
BS_VolID:       dd 0x41544F4D           ; Серийный номер 'ATOM'
BS_VolLab:      db "NO NAME    "        ; 
BS_FilSysType:  db "FAT16   "           ; Системный идентификатор

start:
    ; Сегменты
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Видеорежим
    mov ah, 0x0
    mov al, 0x3
    int 0x10

    call ADD_SIGNATURE_FAT
    call LOAD_BOOT

stage2_load_error:
    mov si, not_found 
    call print_string
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

ADD_SIGNATURE_FAT:
    pusha
    
    xor ax, ax
    mov es, ax

    mov ah, 0x2
    mov al, 6
    mov cl, 2
    mov ch, 0
    mov dh, 0
    mov dl, 0x80
    mov bx, 0x1000
    
    int 0x13 

    mov byte[es:bx], 0xF8
    add bx, 1
    mov byte[es:bx], 0xFF
    add bx, 1
    mov byte[es:bx], 0xFF
    add bx, 1
    mov byte[es:bx], 0xFF

    ;mov ah, 0x03
    ;mov bx, 0x1000
    ;int 0x13

    popa
    ret

INIT_FAT:
    xor dx, dx
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
    xor dx, dx
    div cx
    mov [RootDirSectors], ax

    mov ax, [RootDirStartSector]
    add ax, [RootDirSectors]
    mov [DataStartSector], ax

    mov ax, [BPB_TotSec16]
    sub ax, [DataStartSector]
    mov [DataSectors], ax

    ret

LOAD_FAT:
    mov si, k
    call print_string 
    mov si, lba
    mov ah, 0x42
    mov dl, 0x80

    int 0x13

    ret
    
LOAD_BOOT:
    call INIT_FAT
    call LOAD_FAT
    mov si, stage2_file
    
    mov ax, [RootDirStartSector]
    mov [lba+8], ax
    
    mov bx, 0x7E00
    xor ax, ax
    mov es, ax
    mov ds, ax
    mov cx, 0

.FIND_FILE_LOOP:
    cmp cx, 512
    je .NOT_FOUND
    mov di, bx  
    call compare_strings 
    je .FOUND
    
    add cx, 32
    add bx, 32

    jmp .FIND_FILE_LOOP

.NOT_FOUND:
    ret

.FOUND:
    mov ax, word [es:bx + 0x1A] 

    sub ax, 2
    xor cx, cx
    movzx cx, byte [BPB_SecPerClus]
    mul cx
    add ax, [DataStartSector]  
    
    mov word [lba],    0x0010
    mov word [lba+2],  0x0006
    mov word [lba+4],  0x8000
    mov word [lba+6],  0x0000 
    mov word [lba+8],  ax
    mov word [lba+10], 0x0000

    mov si, lba
    mov ah, 0x42
    mov dl, 0x80
    int 0x13

    jmp 0x0000:0x8000

    ret

compare_strings:
    pusha
    xor cx, cx

.next_char: 
    lodsb
    
    cmp cx, 11
    je .equal
    
    cmp al, [di] 
    jne .not_equal

    cmp al, 0
    je .equal
   
    inc di
    inc cx
    
    jmp .next_char

.not_equal:
    popa
    ret

.equal: 
    popa
    ret
    
lba:
    db 0x10
    db 0x00
    dw 1
    dw 0x7E00
    dw 0x0000
    dq 129          ; Тут начинается рут

RootDirStartSector: dw 0
RootDirSectors:     dw 0 
FatSectors:         dw 0
FatStartSector:     dw 0
DataStartSector:    dw 0
DataSectors:        dw 0
not_found: db "error: BOOTLOADER not found",0
stage2_file: db "BOOT    BIN",0
e: db 10,"Error!",10,0
k:db 10,"Ok",10,0

times 510 - ($ - $$) db 0
dw 0xAA55
