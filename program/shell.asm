bits 32
org 0x300000
global shell
shell:
    ; ebx 1 = Вывести текст
    ; ebx 2 = Вывести переменную

    mov eax, 3          ; VGA
    mov ebx, 1          ; Установить атрибут
    mov ecx, 0x0004     ; Красный на черном фоне
    int 0x80   

    mov eax, 1    ; Вывод текста
    mov ebx, 1    ; Формат вывода (текс)
    mov ecx, msg  ; Что выводить
    mov edx, 0     
    
    int 0x80

    mov eax, 3          ; VGA
    mov ebx, 1          ; Установить атрибут
    mov ecx, 0x000F     ; Белый на черном фоне
    int 0x80 

    ;jmp $

    mov eax, 60
    int 0x80
    
    mov eax, 1
    mov ecx, msg_
    int 0x80


    mov eax, 1
    mov ebx, 2
    mov ecx, esp
    int 0x80

    mov eax, 1    ; Вывод текста
    mov ebx, 1    ; Формат вывода (текс)
    mov ecx, new_string  ; Что выводить
    mov edx, 0    

    jmp loop

    jmp $

    ret

loop:
    mov eax, 2
    mov ebx, 1
    int 0x80
    mov ecx, eax
    mov eax, 1
    mov ebx, 3
    int 0x80
    jmp loop

print_string:
    

msg: db "Hello World",10,0
msg_: db "(In program) stack: ",0
new_string: db " ",10,0
