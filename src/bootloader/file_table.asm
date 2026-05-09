; Одна записать 24 байта
.f1:
db "Kernel.bin"             ; Название
times 16 - ($ - .f1) db 0   ; Добить до 16 байт
dd 100                      ; Откуда грузить
dd 30                       ; Сколько грузить

.f2:
db "Config.cfg" 
times 16 - ($ - .f2) db 0
dd 8
dd 1

.f3 
db "Shell.bin"
times 16 - ($ - .f3) db 0
dd 40
dd 20
