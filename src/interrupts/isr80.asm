extern syscall_handler
global isr80

isr80: 
    cli                        
    push edi
    push esi
    push ebp
    push edx
    push ecx
    push ebx 


    push ds
    push es
    push fs
    push gs
 
  
    push edx
    push ecx
    push ebx
    push eax

  
    mov ax, 0x10
    mov ds, ax
    mov es, ax 

    call syscall_handler
    add esp, 4
    
    pop ebx
    pop ecx
    pop edx     
    
    pop gs
    pop fs
    pop es
    pop ds
 
    pop ebx
    pop ecx
    pop edx
    pop ebp
    pop esi
    pop edi
    
    sti                         
    iret 
