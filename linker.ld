ENTRY(kmain)			/* Точка входа в ядро */
SECTIONS {
    	. = 0x10000;		/* Ядро будет загружено в 0x10000*/
	
    	.header : {		/* Первые два байта сигнатура */
    		BYTE(0xAA)
		BYTE(0xBB)
	}		

    	.text : { *(.text*) }
    	.data : { *(.data*) }
    	.bss  : { *(.bss*)  }
}
