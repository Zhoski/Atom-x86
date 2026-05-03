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

    ;mov eax, [to_new_line]
    ;dec eax
    ;mov [to_new_line], eax

    ;mov eax, 1
    ;mov ebx, 2
    ;mov ecx, [to_new_line]
    ;int 0x80

    ;mov ecx, user_name
    ;call print_string

    ;mov ecx, prompt
    ;call print_string

    ;jmp $

    ;mov eax, 60
    ;int 0x80 

    jmp _shell

    ret

_shell:
    call reset_buffer
    call get_user_name

    mov ecx, user_name
    call print_string

    mov ecx, prompt
    call print_string

    mov esi, command_buffer
    xor edx, edx

.shell_loop:
    mov eax, 2
    mov ebx, 1
    int 0x80
    
    cmp eax, 0
    jz .skip

    cmp eax, 0x0A
    jz .enter

    mov [esi], eax
    inc esi
    inc edx

    pushad
    mov ecx, eax
    mov eax, 1
    mov ebx, 3
    int 0x80
    popad

.skip:
    jmp .shell_loop

.enter: 
    jmp execute

parser:
    mov esi, command_buffer

.parser_loop:
    cmp byte [esi], 0
    jz .end

    cmp byte [esi], ' '
    jnz .skip
    
    mov byte [esi], 0

.skip:
    inc esi
    jmp .parser_loop

.end:
    ret

find_next_arg:
    xor edx, edx 
.find_loop:
    ;mov eax, 1
    ;mov ebx, 3
    ;mov ecx, [esi]
    ;int 0x80

    inc esi
    inc edx
    cmp byte[esi], 0  
    jnz .find_loop 

    ret

execute: 
    call parser

    mov esi, command_buffer
    mov edi, help
    call compare_strings
    je .do_help

    mov esi, command_buffer
    mov edi, memread
    call compare_strings
    je .do_memread

    mov ecx, new_string
    call print_string

    jmp _shell
    

.do_help:
    mov ecx, list
    call print_string
    jmp _shell

.do_memread:
    mov ecx, new_string
    call print_string

    mov esi, command_buffer
    call find_next_arg      ; Найти аргумент, в этом случаи адрес
    
    inc esi

    push esi
    call hex_string_to_int  ; Следущий аргумент 
    pop esi

    mov ecx, eax
    
    push ecx

    call find_next_arg
    inc esi
    call string_to_int
    mov [count], eax

    pop ecx
    
    xor edx, edx

    push ecx
    push edx 
    mov eax, 1
    mov ebx, 4
    mov edx, 7
    int 0x80

    mov ecx, adres
    call print_string

    pop edx
    pop ecx

.while:
    cmp edx, [count]
    jz .end

    push edx
    mov edx, [to_new_line]
    cmp edx, 0
    pop edx

    jz .new_line
    jmp .skip

.new_line:
    push ecx
    mov ecx, new_string
    call print_string
    pop ecx

    mov eax, 16
    mov [to_new_line], eax

    push ecx
    push edx 
    mov eax, 1
    mov ebx, 4
    mov edx, 7
    int 0x80

    mov ecx, adres
    call print_string

    pop edx
    pop ecx

.skip:
    push ecx
    mov eax, 4
    mov ebx, 1
    int 0x80
    pop ecx

    push ecx
    push edx

    mov ecx, eax
    mov eax, 1
    mov ebx, 4
    mov edx, 1
    int 0x80
    
    mov eax, 1
    mov ebx, 3
    mov ecx, ' '
    int 0x80

    pop edx
    pop ecx

    inc ecx
    inc edx 
    
    mov eax, [to_new_line]
    dec eax
    mov [to_new_line], eax

    jmp .while

.end:
    mov ecx, new_string
    call print_string

    jmp _shell

; Вход: EAX = 1002 (десятичное)
; Выход: EAX = 0x1002 (шестнадцатеричное значение)
hex_string_to_int:
    xor eax, eax        ; Обнуляем результат

.loop:
    movzx ecx, byte [esi] ; Берем символ
    inc esi               ; Сразу двигаем указатель на следующий
    
    test cl, cl           ; Конец строки?
    jz .done

    ; Переводим символ в значение
    cmp cl, '0'
    jl .done              ; Мусор (меньше '0')
    cmp cl, '9'
    jbe .is_digit         ; Это цифра 0-9

    ; Если не цифра, проверяем на буквы A-F
    and cl, 0xDF          ; Делаем букву заглавной (маленькая 'a' станет 'A')
    cmp cl, 'A'
    jl .done              ; Мусор между '9' и 'A'
    cmp cl, 'F'
    jg .done              ; Мусор больше 'F'

    sub cl, 'A' - 10      ; 'A' -> 10, 'B' -> 11 и т.д.
    jmp .add_to_res

.is_digit:
    sub cl, '0'           ; '0' -> 0, '1' -> 1

.add_to_res:
    shl eax, 4            ; Умножаем текущий результат на 16 (сдвиг влево на 4 бита)
    or  eax, ecx          ; Добавляем новую цифру в младшие 4 бита
    jmp .loop

.done:
    ret
string_to_int:
    xor eax, eax
    xor ecx, ecx

.loop:
    mov cl, [esi]
    cmp cl, 0
    jz .done

    cmp cl, '0'
    jl .done        
    cmp cl, '9'
    jg .done         

    sub cl, '0'       
    
    imul eax, 10      
    
    add eax, ecx       
    inc esi            
    jmp .loop

.done:
    ret

reset_buffer:
    mov edi, command_buffer ; Сюда буффер   
    xor al, al              ; al в 0
    mov cx, 64              ; Сколько байтов обнуляить 
    cld                     ; CF в 0
    rep stosb           
    mov edi, command_buffer  
    ret

compare_strings:
    pushad
    xor ecx, ecx

.next_char:
    lodsb
    cmp al, [edi]
    jne .not_equal

    cmp al, 0
    je .equal

    inc edi
    jmp .next_char

.not_equal:
    popad
    ret

.equal:
    popad
    ret

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

; Список команд
list:   db 10, "+-----------------------------------------------+",10
        db     "| help - out list of commands                   |",10
        db     "| clear - clear screen                          |",10
        db     "+-----------------------------------------------+",10,0

; Команды
help: db "help",0
memread: db "memread",0
adres: db "|    ",0

; Переменные
count: dd 0
to_new_line: dd 16

; Буфферы
user_name: times 32 db 0
user_pass: times 32 db 0
command_buffer: times 64 db 0
hex_buffer: times 16 db 0
