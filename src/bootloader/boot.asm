; ==============================================================================
; boot.bin отвечает за загрузку stage2.bin (Основной загрузчик)
; boot.bin читает 6 секторов начиная с 2 (где лежит stage2.bin) по адресу 0x8000
; Также отвечает за разметку диска под FAT16
; P.S: Пока не будет доделана файловая система загрузчик имеет строгую привязку
; к секторам, но после окончания работ по FAT16 загрузчик будет грузить не сектор
; а файл на прямую по названию "stage2.bin"
; ===============================================================================

bits 16
org 0x7C00

; Перепрыгивание BPB
jmp short start
nop
db "ATOM-x86"           ; OEM identifier

; ========= BPB =========
dw 512                  ; 512 байт на сектор
db 4                    ; 4 сектора на кластер
dw 8                    ; Зарезервировано под загрузчик
db 2                    ; Будет две записи FAT
dw 256                  ; Записей в root каталоге
dw 32768                ; 32768 сектора (16Мб)
db 0xF8                 ; Жетский диск
dw 128                  ; Размер одной FAT
dw 63                   ; Секторов на одной дорожке
dw 255                  ; Головок на одной стороне
dd 0
dd 0

; ===== Extended BPB =====
db 0x80                 ; Номер диска
db 0x00                 ; Зарезервировано
db 0x29                 ; Сигнатура
dd "ATOM"               ; Серийный номер
db "NO NAME    "        ; 
db "FAT16   "           ; Системный идентификатор

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
    mov al, 6
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
stage2_file: db "STAGE2 BIN",0
root: dw 520                        ; Тут начинается root с учетом резрва в 8 секторов

times 510 - ($ - $$) db 0
dw 0xAA55
