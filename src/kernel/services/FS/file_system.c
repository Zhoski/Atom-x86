#include "file_system.h"
#include "../../../drivers/VGA/vga.h"

file file_table[MAX_FILE];

void init_table() {
    uint32_t page_ptr = 0;
    for(uint32_t page_i = 0;page_i < MAX_FILE;page_i++) {
        if(!memread(TABLE_BASE_ADRES+page_ptr)) {break;}
        uint32_t page_offset = 0; 
        // Получение имени
        for(;page_offset < 16;page_offset++) {
                file_table[page_i].name[page_offset] = memread(TABLE_BASE_ADRES+page_offset+page_ptr);
        }
        
        // Адрес в RAM
        file_table[page_i].sector_entry = memread_dd(TABLE_BASE_ADRES+page_offset+page_ptr);
        page_offset += 4;
        // Размер в секторах
        file_table[page_i].size_in_sector = memread_dd(TABLE_BASE_ADRES+page_offset+page_ptr);

        page_ptr += SIZE_ONE_RECORD;
    } 
}
