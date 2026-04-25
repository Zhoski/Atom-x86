bits 16
org 0x7C00

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

times 510 - ($ - $$) db 0
dw 0xAA55
