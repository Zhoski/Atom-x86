ENTRY(_start)

SECTIONS 
{
    . = 0x300000;           
    .text : 
    {
        . = ALIGN(4);
        entry.o(.text)
        *(.text)        
        *(.text*)           
    }

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
    }

    .data :
    {
        . = ALIGN(4);
        *(.data)         
        *(.data*)          
    }

    .bss :
    {
        . = ALIGN(4);
        *(.bss)
        *(.bss*)
    }
} 
