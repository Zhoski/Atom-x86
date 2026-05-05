; Размер записи 24 байта
.f1:
db "Kernel.bin"             ; Название
times 16 - ($ - .f1) db 0   ; Добить до 16 байт
dd 0x10000                  ; Откуда грузить
dd 10                       ; Сколько секторов грузить

.f2:
db "Config.cfg" 
times 16 - ($ - .f2) db 0
dd 0x1000
dd 1

.f3 
db "Shell.bin"
times 16 - ($ - .f3) db 0
dd 0x2000
dd 20
