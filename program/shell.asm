bits 32
org 0x300000
global shell
shell:
    ; ebx 1 = Вывести текст
    ; ebx 2 = Вывести переменную 

    mov eax, 3
    mov ebx, 1
    mov ecx, 0xA
    int 0x80

    mov ecx, logo
    call print_string

    mov ecx, logo2
    call print_string
    
    mov eax, 3
    mov ebx, 1
    mov ecx, 0xF
    int 0x80

    mov ecx, help_msg
    call print_string

    call get_user_name

    mov ecx, user_name
    call print_string

    mov ecx, prompt
    call print_string

    ;jmp $

    ;mov eax, 60
    ;int 0x80 

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

get_user_name:
    mov eax, 4
    mov ebx, 1
    mov ecx, 0x1002 
    mov esi, user_name
    xor edx, edx

.get_loop:
    cmp edx, 32
    jz .done

    mov eax, 4
    int 0x80
    
    mov [esi], eax 
     
    inc edx
    inc ecx
    inc esi

    jmp .get_loop

.done:  
    ret

print_string: 
    pushad
    mov eax, 1
    mov ebx, 1
    int 0x80
    popad
    ret

new_string: db " ",10,0

logo:   db "    ___   __                      _  ______  _____",10
        db "   /   | / /_____  ____ ___      | |/ / __ \/ ___/",10
        db "  / /| |/ __/ __ \/ __ `__ \_____|   / /_/ / __ \ ",10
        db " / ___ / /_/ /_/ / / / / / /_____/   \__, / /_/ / ",10,0
logo2:  db "/_/  |_\__/\____/_/ /_/ /_/     /_/|_/____/\____/ ",10,10,10,0  
help_msg: db "Type 'help' to get a list of commands.",10,0
prompt: db "$> ",0

; Буфферы
user_name: times 32 db 0
