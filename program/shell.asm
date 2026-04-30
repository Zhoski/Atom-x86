bits 32
org 0x200000
global shell
shell: 
    ; ebx 1 = Вывести текст
    ; ebx 2 = Вывести переменную

    mov eax, 1    ; Вывод текста
    mov ebx, 1    ; Формат вывода (текс)
    mov ecx, msg  ; Что выводить
    mov edx, 0    ; Аргумент 3
    
    int 0x80 
    jmp $

    ret

msg: db "Hello World!",0
