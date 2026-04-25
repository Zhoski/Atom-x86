; Размер записи 24 байта
.f1:
db "Kernel.bin"			; Название
times 16 - ($ - .f1) db 0	; Добить до 16 байт
dw 10				; Откуда грузить
dw 10				; Сколько грузить
				; Куда грузить
dw 0x1000			; Сегмент
dw 0x0000			; Смещение

.f2:
db "Config.cfg"	
times 16 - ($ - .f2) db 0
dw 8
dw 1
dw 0x0000
dw 0x1000

.init
times 512 - ($ - .init) db 0
