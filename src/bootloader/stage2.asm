bits 16
org 0x8000

; ========================= Инициализация ==========================

start:
	; Установка сегментных регистров
	mov ax, 0x0000
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7C00		; Стек под загрузчик

	in al, 0x92
	or al, 2
	out 0x92, al		; A20 line включить	

	mov si, msg
	call print

	mov si, ok_msg
	call print
	
	call found_file
	call kernel_load	; Загрузка ядра	
	
	call get_memmap
	;call check_first_boot ; Проверка на первый запуск

	mov si, lba_config		; Грузим конфиг
	call disk_read			; 
	mov bx, [lba_config+4]		; 0x1000
	mov es, [lba_config+6]		; 0x0000
	mov ah, [es:bx]			; читаем байт по 0x0000:0x1000

	mov si, get_cfg_msg
	call print

	cmp ah, 0xFF		; Сигнатура конфига
	jnz .config_error	; Если сигнатура не найдена
	
	mov si, ok_msg
	call print

	add bx, 1
	
	mov ah, [es:bx]
	cmp ah, 0x00		; Если 0x0000:0x1001 это 0
	jz first_boot		; Прыжок в первый запуск
	
	jmp base_boot		; Елси не первый то в базу

	jmp $

.config_error:

	mov si, fail_msg
	call print

	mov si, cfg_error
	call print

	mov si, cfg_error_signature
	call print

	mov si, cfg_error_addit
	call print

	jmp $

; Базовая загрузка если 1-й байт в конфиге 1
base_boot:
	mov si, hi_user
	call print	

	; bx установить на 0x1001
    	mov bx, [lba_config+4]	 	; 0x1000
	add bx, 2			; 0x1002
    	mov si, user_name 		; Куда грузить данные 

	; Сегменты в 0, cx обнулить
    	xor cx, cx          
    	mov es, [lba_config+6]	; 0x0000

	; Запись в si с 0x0000:0x1002	
.loop:
	; Копируем имя пользователя из кoнфига 32 символа
	cmp cx, 32          
    	je .exit
    
    	mov al, [es:bx] ; в al текущий символ bx
    	mov [si], al    ; в буфер имени al
    
    	; Инкрименты
    	inc bx            
    	inc si             
    	inc cx              
    	jmp .loop

.exit: 
	mov si, user_name
	call print
	
	mov al, '!'
	mov ah, 0x0E
	int 0x10

	mov si, new_string
	call print

   	jmp terminal	


; Случай если это первый запуск и требуются конфиги
first_boot:
	mov si, sys_boot_first
	call print

	mov si, sys_boot_user
	call print

	mov si, prompt
	call print	

	; es в ноль bx на 1 байт конфига
	mov es, [lba_config+6]	 	; 0x0000
	mov bx, [lba_config+4]		; 0x1000

	; Ставим в 1 байт конфига 1
	add bx, 1
	mov byte [es:bx], 0x1
	
set_user_name:
	; в di буфер имени
	mov di, user_name
	xor cx, cx

.loop:
	; Читаем пока не будет enter или cx > 32
	mov ah, 0x0
	int 0x16

	cmp cx, 32
	je .done_read

	cmp al, 0x0D
	jz .done_read

	cmp al, 0x08
	jz .name_backspace

	mov [di], al
	inc di
	
	mov ah, 0x0E
	int 0x10

	jmp .loop

.name_backspace:
	cmp di, 0
	je .loop
    	dec di
    	dec cx
	mov al, 0
	mov [di], al
    	mov ah, 0x0E
    	mov al, 0x08
    	int 0x10
    	mov al, ' '
    	int 0x10
   	mov al, 0x08
   	int 0x10
    	jmp .loop

.done_read:
	mov si, new_string
	call print

.exit
	mov si, hi_user
	call print

	mov si, user_name
	call print
		
	mov ah, 0x0E
	mov al, '!'
	int 0x10

	mov si, new_string
	call print

	mov es, [lba_config+6]		; 0x0000
	mov bx, [lba_config+4]		; 0x1000
	add bx, 2			; Теперь запись в 0x0000:0x1002

	mov si, user_name		; Грузим из буффера в es:bx

.write_name_cfg:
	; Запись имени в конфиг пока не дойдем до 0
	mov al, [si]
	cmp al, 0

	je set_password

	mov [es:bx], al
	
	inc si
	inc bx
	
	jmp .write_name_cfg

; Установка пароля
set_password:
	mov si, sys_boot_pass
	call print
	
	mov si, user_name
	call print
	
	mov si, prompt
	call print

	mov si, user_password	; Буффер куда грузить

.pass_loop:
	mov ah, 0x0
	int 0x16

	cmp al, 0x0D
	jz .pass_write_cfg

	cmp al, 0x08
	jz .pass_backspace

	mov [si], al
	inc si

	jmp .pass_loop

.pass_backspace:
	cmp si, 0
	je .pass_loop
    	dec si
    	dec cx
	mov al, 0
	mov [si], al
    	mov ah, 0x0E
    	mov al, 0x08
    	int 0x10
    	mov al, ' '
    	int 0x10
   	mov al, 0x08
   	int 0x10
    	jmp .pass_loop
	

.pass_write_cfg:
	; es в ноль bx на 33 байт конфига
	mov si, user_password
	mov es, [lba_config+6]		; 0x0000
	mov bx, [lba_config+4]		; 0x1000
	add bx, 34			; Тут пароль на 0x0000:0x1022

.pass_write_loop:
	mov al, [si]
	cmp al, 0
	jz .done

	mov [es:bx], al
	
	inc bx
	inc si

	jmp .pass_write_loop

.done:
	mov si, new_string
	call print
	
	; es в ноль bx на начало конфига
	mov es, [lba_config+6]
	mov bx, [lba_config+4]

	mov si, lba_config
	call disk_write

	jmp terminal

.error:
	mov si, disk_error_msg
	call print

	jmp $

; =========================== Терминал ===============================
terminal:
	mov si, user_name
	call print
	mov si, prompt
	call print

	mov di, c_buffer
.loop:
	; Считываем нажатие пока не будет enter
	mov ah, 0x0
	int 0x16

	cmp al, 0x0D
	je .do_enter

	cmp al, 0x08
	je .do_backspace

	mov [di], al
	inc di

	mov ah, 0x0E
	int 0x10

	jmp .loop

.do_enter:
	mov si, new_string
	call print

	mov dl, 0
	
	inc dh
	jmp .done_read

.do_backspace:
	cmp di, 0
	je .loop
    	dec di
    	dec cx
	mov al, 0
	mov [di], al
    	mov ah, 0x0E
    	mov al, 0x08
    	int 0x10
    	mov al, ' '
    	int 0x10
   	mov al, 0x08
   	int 0x10
    	jmp .loop

.done_read:
	jmp .execute

.execute:
	; Исполнение команд
	pusha
	mov si, c_buffer
	mov di, c_help

	call compare_strings
	je .do_help

	mov si, c_buffer
	mov di, c_clear

	call compare_strings
	je .do_clear

	mov si, c_buffer
	mov di, c_user
	
	call compare_strings
	je .do_user

	mov si, c_buffer
	mov di, c_memmap

	call compare_strings
	je .do_memmap

	mov si, c_buffer
	mov di, c_start
	
	call compare_strings
	je .do_start

	popa

; Коммандный буффер в ноль
.reset_buffer:
	; Буффер в нулину
	mov di, c_buffer ; Сюда буффер   
    	xor al, al       ; al в 0
    	mov cx, 64       ; Сколько байтов обнуляить 
    	cld              ; CF в 0
    	rep stosb           
    	mov di, c_buffer 

    	jmp terminal     ; Обратно в терминал


; Command
.do_help:
	mov si, help_msg
	call print

	jmp .reset_buffer

.do_clear:
	mov ah, 0x0
	mov al, 0x03
	int 0x10

	jmp .reset_buffer

.do_user:
	mov si, user_name
	call print

	mov si, new_string
	call print

	jmp .reset_buffer

.do_memmap:
	mov si, memmap_msg
	call print

	mov es, [memmap_segment]
	mov bx, [memmap_buffer]
	xor di, di
	xor dx, dx

	xor ecx, ecx	; Будет в роли размера блока

.mem_loop:

	; Старшие 4 байта базового адреса
	mov eax, [es:bx+4]
	call print_hex_32

	; Младшие 4 байта базового адреса
	mov eax, [es:bx]
	call print_hex_32

	mov si, tab
	call print

	; Старшие 4 байта размера
	mov eax, [es:bx+12]
	call print_hex_32
	;mov ecx, [es:bx+8]
	;mov eax, [es:bx]
	;sub ecx, eax
	;mov eax, ecx
	;call print_eax

	; Младшие 4 байта размера
	mov eax, [es:bx+8]
	call print_hex_32

	mov si, tab
	call print

	; 4 байта типа памяти
	mov eax, [es:bx+16]

	cmp eax, 1
	jz .usable

	cmp eax, 2
	jz .reserved

	cmp eax, 3
	jz .acpi_rec

	cmp eax, 4
	jz .acpi_nvs 

	cmp eax, 5
	jz .bad_memory

	jmp .usable

.usable:
	mov si, memmap_usable
	call print
	jmp .mem_type_end

.reserved:
	mov si, memmap_reserved
	call print
	jmp .mem_type_end

.acpi_rec:
	mov si, memmap_acpi_rec
	call print
	jmp .mem_type_end

.acpi_nvs:
	mov si, memmap_acpi_nvs
	call print
	jmp .mem_type_end

.bad_memory:
	mov si, memmap_bad_memory
	call print

.mem_type_end:
   	mov si, new_string
	call print
	
	cmp dx, [memmap_block_count]
	jz .reset_buffer
	
	add bx, 24
	inc dx
    	jmp .mem_loop

.do_start:
	jmp kernel_launch

; ============================ Память ================================
get_memmap:
	pusha				; Регистры запомнить
	
	mov si, get_memmap_msg
	call print

	mov si, ok_msg
	call print
	
	mov es, [memmap_segment]	; 0x0000
	mov di, [memmap_buffer] 	; 0x0500
	xor ebx, ebx     		; ebx обнулить

.next_entry:
    	mov eax, 0xE820
    	mov edx, 0x534D4150
    	mov ecx, 24        
    	int 0x15
    
    	jc .done       
    	add di, 24          
    	
	mov cx, [memmap_block_count]
	inc cx
	mov [memmap_block_count], cx

    	test ebx, ebx       
    	jz .done          
    	jmp .next_entry     

.done:	
	; Уменьшить на 1 потому что какой то баг выдает на 1 сектор больше
	mov cx, [memmap_block_count]
	dec cx
	mov [memmap_block_count], cx	

	mov si, memmap_total_block_msg
	call print

	mov si, memmap_block_count
	call print

	mov ax, [memmap_block_count]
	call print_ax

	mov si, parren_close
	call print

.get_usable_ram:
	; Читать из 0x0000:0x0500
	mov bx, [memmap_buffer]
	mov es, [memmap_segment]
	xor si, si			; Счетчик

.ram_loop:
	cmp si, [memmap_block_count]
	jz .done_read

	mov eax, [es:bx+16]
	cmp eax, 1
	jnz .skip

	mov eax, [es:bx+8]
	shr eax, 10

	add [total_usable_ram_k], eax

.skip:
	inc si
	add bx, 24
	jmp .ram_loop

.done_read:
	mov si, memmap_total_ram_usable
	call print

	mov ax, [total_usable_ram_k]
	call print_ax

	mov ah, 0x0E
	mov al, 'k'
	int 0x10

	mov si, parren_close
	call print

	popa
	ret

; ============================== Диск ================================

; Загрузка диска в оперативку
; Вход es:bx al, cl
disk_read:
	pusha

	mov ah, 0x42
	mov dl, 0x80

	int 0x13

	jc .disk_read_error

	popa
	ret

.disk_read_error:
	mov si, disk_read_error
	call print

	cmp ah, 0x01
	je .invalid_op

	cmp ah, 0x04
	je .sector_not_found

	cmp ah, 0x40
	je .seek_failed

	cmp ah, 0x80
	je .time_out

.invalid_op:
	mov si, disk_invalid_op
	call print

	jmp .disk_read_exit

.time_out:
	mov si, disk_time_out

	jmp .disk_read_exit

.sector_not_found:
	mov si, disk_sector_not_found

	jmp .disk_read_exit

.seek_failed:
	mov si, disk_seek_failed

.disk_read_exit:
	call print

	popa
	ret

; Загрузка диска из оперативки
; Вход es:bx al, cl
disk_write:
	pusha
	
	mov ah, 0x43
	mov dl, 0x80
	int 0x13

	jc .disk_write_error

	popa
	ret

.disk_write_error
	mov si, disk_write_error
	call print
	
	popa
	ret

; =========================== Система в/в =============================
print:
	pusha
.loop:
	lodsb
	or al, al
	jz .exit
	mov ah, 0x0E
	int 0x10
	jmp .loop

.exit:
	popa
	ret

print_hex_32:
    	pushad
    	mov edx, eax        ; Копируем число в EDX, его портить не будем
    	mov cx, 8           ; 8 цифр
.loop:
    	rol edx, 4          ; Вращаем EDX!
    	mov al, dl          ; Берем младшие 4 бита из EDX (после вращения они там)
    	and al, 0x0F        ; Маска
    
    	cmp al, 10
    	jl .is_digit
    	add al, 7
.is_digit:
    	add al, '0'
    
    	mov ah, 0x0E        ; Теперь AX можно портить
    	mov bx, 0x0007      ; Режим: страница 0, цвет (для графики)
    	int 0x10
    
    	loop .loop
    	popad
    	ret

print_ax:
	pusha
	mov cx, 10
	xor bx, bx

.split_loop:
	xor dx, dx
	div cx
	add dl, '0'
	push dx
	inc bx
	test ax, ax
	jnz .split_loop

.print_loop:
	pop ax
	mov ah, 0x0E
	int 0x10
	dec bx
	jnz .print_loop

.print_done:
	popa
	ret

compare_strings:
	pusha
	xor cx, cx

.next_char:
    	lodsb
    	cmp al, [di]
    	jne .not_equal
    	cmp al, 0
    	je .equal
    	inc di
    	jmp .next_char

.not_equal:
	popa
    	ret

.equal:
	popa
    	ret

; ================= Драйвер для работы с файлами ==================

files_table:
	incbin "table.bin"

found_file:
	mov di, files_table
	mov si, config

.find_loop:
	xor ax, ax
	mov al, [di]

	cmp al, 0
	jz .file_not_found
	
	push si
	mov si, di
	call print
	pop si

	call compare_strings
	jz .file_found
	
	add di, 24
	jmp .find_loop

	;jmp .file_not_found

.file_found:
	mov si, file_found
	call print	

	jmp .done

.file_not_found:
	mov si, file_not_found
	call print

.done:
	ret

; =========================== Переменные =============================

kernel: db "Kernel.bin",0
config: db "Config.cfg",0
file_found: db "File found",13,10,0
file_not_found: db "File not found",13,10,0
; Адреса

; E820 карта памяти
memmap_buffer: dw 0x0500
memmap_segment: dw 0x0000

memmap_block_count: dw 0
total_usable_ram_k: dw 0

; Конфиг
config_segment: dw 0x0000
config_offset:  dw 0x1000

; Системные сообщения
msg: db "Bootloader stage 2 load ",0

get_cfg_msg: db "Getting config system ",0
cfg_error:   db "Config read error: ",0
cfg_error_signature: db "Config is missing or damaged",13,10,0
cfg_error_addit:     db "Additionally: The config file must begin with the signature 0xFF",13,10,0

get_memmap_msg: db "Getting memory map ",0
memmap_total_block_msg: db "Total block [ ",0
memmap_total_ram_usable:db "Total usable ram [ ",0
memmap_msg: db "  Base Address             Lenght             Type",13,10,0

memmap_reserved:   db 	"Reserved",0
memmap_usable:     db 	" Usable",0
memmap_acpi_rec:   db 	"ACPI Reclaimable",0
memmap_acpi_nvs:   db 	"ACPI NVS",0
memmap_bad_memory: db 	"Bad memory",0

help_msg: db 	"> clear  --- reset screen",13,10,"> user   --- write current user",13,10,"> start --- load kernel",13,10,"> memmap --- write memory map",13,10,0

disk_error_msg: db "> Disk load error",13,10,0
disk_load_error: db "Disk load error",13,10,0
disk_write_error: db "Disk write error",13,10,0
sys_boot_first: db "> The system is running for the first time, let's configure it",13,10,0

sys_boot_user: db "> Enter your username (max 32 symbol)",13,10,0
sys_boot_pass: db "> Enter your password (max 32 symbol)",13,10,0

; Статусы
ok_msg:   db "[ OK ]",13,10,0
fail_msg: db "[ FAIL ]",13,10,0

kernel_load_msg: db "Load kernel from hard disk...",13,10,0
kernel_load_status: db "Kernel load ",0
kernel_load_error:  db "Kernel load error: ",0
kernel_signature_error: db "Kernel is missing or damaged",13,10,0
kernel_signature_addit: db "Additionaly: Kernel file must begin with the signature 0xAABB"

; Ошибки 
disk_read_error:  db "Disk read error: ",0
disr_write_error: db "Disk write error: ",0

; Кода ошибок
disk_time_out:	  	db "Time out",13,10,0
disk_sector_not_found: 	db "Sector not found",13,10,0
disk_seek_failed: 	db "Seek Failed",13,10,0
disk_invalid_op:	db "Invalid function or invalid parameter",13,10,0

; Консоль команды
prompt: db "> ",0
hi_user: db "Hi ",0

k_msg: db "k",0
m_msg: db "M",0

new_string: db 13,10,0
tab:	db "      ",0

; Буфферы
c_buffer: times 64 db 0 
user_name: times 32 db 0
user_password: times 32 db 0

section .data
; Command
c_help: db "help",0
c_clear: db "clear",0
c_user: db "user", 0
c_memmap: db "memmap",0
c_start:  db "start",0

; Специально
parren_close: db " ]",13,10,0

; ========================== DAP таблицы =============================
; Конфиг
lba_config:
	db 0x10			; Размер
	db 0x00			; Зарезервировано
	dw 1			; Читать 1 сектор
	dw 0x1000		; куда грузить
	dw 0x0000		;
	dq 8 			; Сектор начала

lba_kernel:
	db 0x10			; Размер
	db 0x00			; Зарезервировано
	dw 16			; Читать 16 секторов
	dw 0x0000		; 0x1000:0x0000 64k
	dw 0x1000		; 
	dq 10			; Сектор начала

dap:
	db 0x10
	db 0x00
	dw 0
	dw 0x0000
	dw 0x0000
	dq 0

; ====================== Таблица десрипторов =========================
gdt_start:
	; Null Descriptor
	dq 0x0000000000000000

	; Kernel Mode Code Segment
	dw 0xFFFF	; Limit Low
	dw 0x0000	; Base Low
	db 0x00		; Base Mid
	db 0b10011010   ; Acces
	db 0b11001111	; Flags
	db 0x00		; Base High

	; Kernel Mode Data Segment
	dw 0xFFFF	; Limit Low
	dw 0x0000	; Base Low
	db 0x00		; Base Mid
	db 0b10010010	; Acces
	db 0b11001111	; Flags
	db 0x00		; Base High
	

gdt_end:	

gdt_ptr:
	dw gdt_end - gdt_start - 1
	dd gdt_start

; ========================= Ядро =====================================
; Загрузка ядра из 10 сектора в 0x10000
kernel_load:
	mov si, kernel_load_msg
	call print

	mov si, kernel_load_status
	call print

	mov si, lba_kernel
	call disk_read

	; Проверка на целостность
	mov es, [lba_kernel+6]
	mov bx, [lba_kernel+4]
	mov al, [es:bx+1]
	mov ah, [es:bx]

	cmp ax, 0xAABB		; Сигнатура ядра

	jnz .kernel_load_error  ; Если первые два байта не AABB ядра нет или она повреждено

	mov si, ok_msg
	call print
	ret

.kernel_load_error:
	mov si, fail_msg
	call print

	mov si, kernel_load_error
	call print

	mov si, kernel_signature_error
	call print

	jmp $

	ret

; Загрузка ядра
kernel_launch:
	; Передача всех данных ядру
	
	; Загрузка конфигов в 0x0000:0x1000
	mov si, lba_config
	call disk_read

	; По памяти 0x0000:0x0500 уже лежит таблица с картой памяти
	; В 0x0000:0x0F00 количество блоков от Е820
	xor ax, ax
	mov es, ax
	mov bx, 0xF00
	mov ax, [memmap_block_count]
	mov [es:bx], ax

	; Теперь в 0x0000:0x0500 карта памяти 0x0000:0xF00 сколько блоков 0x0000:0x1000 конфиги
	; Теперь ядро без суматохи прочитает эти адреса

; Переключение в защищенный режим
switch_to_PM:
	cli		; офф прерывания

	lgdt [gdt_ptr]	; gdt в проц
	mov eax, cr0	; cr0 в 1
	or eax, 1
	mov cr0, eax
	jmp 0x8:PMentry	; Прыжок в защищенный режим

bits 32
PMentry:
	mov ax, 0x10
	mov ds, ax
	mov ss, ax
	mov fs, ax
	mov es, ax
	mov gs, ax
	mov esp, 0x90000

	jmp 0x8:0x10002		; Сюда потомучто первые два байта это сигнатура!!!!!!!!!!!!!!!!!!

