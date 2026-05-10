bits 16
org 0x7C00

jmp short start         ; Прыжок в старт
nop
db "ATOM.FS1"           ; Название

BPB: 
    dw 512              ; Байт в секторе
    db 4                ; Секторов в кластере
    dw 8                ; Зарезервировано
    db 2                ; FAT будет 2
    dw 512              ; 
    dw 0                ; HDA 1.44M
    db 0xF8             ; Жетский диск
    dw 256              ; Занято секторов
    dw 63               ; BPB_SecPerTrk
    dw 255              ; BPB_NumHeads
    dd 0
    dd 204800

EXTENDEF_BPB:
    db 0x80             ; HDD
    db 0x00             ; Зарезервировано
    db 0x29             ; Сигнатура
    dd 0x12             ; ID тома
    db "NO NAME    "     
    db "FAT16   "   


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

    mov si, message
    call print_string

    xor ax, ax
    mov es, ax

    ; Загрузка большого загрузчика по 0x0000:0x8000 из 2 сектора
    mov ah, 0x2
    mov al, 8
    mov cl, 2
    mov ch, 0
    mov dh, 0
    mov dl, 0x80
    mov bx, 0x8000
    int 0x13

    jc stage2_load_error

    mov si, ok_msg
    call print_string

    ; Запуск большого загрузчика
    jmp 0x0000:0x8000

stage2_load_error:
    mov si, error_msg
    call print_string
    jmp $

find_file:
    

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

message: db "Booting ",0
error_msg: db "[ FAIL ]",13,10,0
ok_msg: db "[ OK ]",13,10,0
stage2_file: db "stage2.bin",0

times 510 - ($ - $$) db 0
dw 0xAA55
