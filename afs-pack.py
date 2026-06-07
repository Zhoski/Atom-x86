import os
import struct

# --- НАСТРОЙКИ ГЕОМЕТРИИ ATOMFS (ИЗ ТВОЕГО PAINT) ---
DISK_SIZE = 16 * 1024 * 1024  # 16 Мегабайт
SECTOR_SIZE = 512

BOOT_SECTOR = 0
ROOT_START_SECTOR = 2         # Таблица файлов стартует со 2 сектора (четное выравнивание!)
ROOT_SECTORS = 16              # Ровно 16 секторов = 8 Килобайт (под кэш L1 i486)
DATA_START_SECTOR = 18         # Мясо файлов начнется строго с 18 сектора (тоже чётное!)

def create_atom_image(source_dir, output_img, boot_bin_path):
    # 1. Создаем чистую болванку диска на 16 МБ, залитую нулями
    disk = bytearray(DISK_SIZE)
    
    # 2. Читаем твой ассемблерный загрузчик и пихаем в сектор 0
    if os.path.exists(boot_bin_path):
        with open(boot_bin_path, 'rb') as f:
            boot_code = f.read()
            disk[0:len(boot_code)] = boot_code
    else:
        print(f"Предупреждение: {boot_bin_path} не найден, сектор 0 залит нулями.")

    # Проверяем папку с программами
    if not os.path.exists(source_dir):
        os.makedirs(source_dir)
        print(f"Создана пустая папка '{source_dir}'. Закинь туда shell.bin, game.bin и т.д.")
        return

    # Получаем список файлов на ПК
    files = sorted([f for f in os.listdir(source_dir) if os.path.isfile(os.path.join(source_dir, f)) and f != 'boot.bin'])
    
    current_free_data_sector = DATA_START_SECTOR
    root_offset = ROOT_START_SECTOR * SECTOR_SIZE

    # 3. ЦИКЛ СБОРКИ КАРТОЧЕК И ЗАПИСИ МЯСА ФАЙЛОВ
    for file_name in files:
        file_path = os.path.join(source_dir, file_name)
        file_size_bytes = os.path.getsize(file_path)
        
        # Переводим размер в честные 512-байтные секторы (округление вверх)
        file_sectors = (file_size_bytes + SECTOR_SIZE - 1) // SECTOR_SIZE
        
        # Разбиваем имя на Имя (8 байт) и Расширение (3 байта)
        name_part, ext_part = os.path.splitext(file_name)
        ext_part = ext_part.replace('.', '').upper()[:3].ljust(3)
        name_part = name_part.upper()[:8].ljust(8)
        
        # Формируем твои идеальные 16 байт: 8s (имя), 3s (расширение), H (старт), H (размер), B (атрибут)
        attribute = 0x01  
        file_entry = struct.pack('<8s3sHHB', 
                                 name_part.encode('ascii'), 
                                 ext_part.encode('ascii'), 
                                 current_free_data_sector, 
                                 file_sectors, 
                                 attribute)
        
        disk[root_offset:root_offset+16] = file_entry
        root_offset += 16
        
        data_offset = current_free_data_sector * SECTOR_SIZE
        with open(file_path, 'rb') as f:
            file_data = f.read()
            disk[data_offset:data_offset+len(file_data)] = file_data
            
        current_free_data_sector += file_sectors

    total_sectors = DISK_SIZE // SECTOR_SIZE
    superblock = struct.pack('<IIIII', total_sectors, ROOT_START_SECTOR, ROOT_SECTORS, DATA_START_SECTOR, current_free_data_sector)
    
    sb_offset = (BOOT_SECTOR * SECTOR_SIZE) + 0x1D0
    disk[sb_offset:sb_offset+len(superblock)] = superblock

    # 5. ВЫПЛЁВЫВАЕМ ГОТОВЫЙ ОБРАЗ ДИСКА
    with open(output_img, 'wb') as f:
        f.write(disk)
        
    print(f"Готово! Образ {output_img} успешно собран.")
    print(f"Всего упаковано файлов: {len(files)}")
    print(f"Следующий файл запишется с сектора: {current_free_data_sector}")

# ЗАПУСК СКРИПТА
# Сканирует папку 'binaries', берет 'boot.bin' и собирает диск 'os.img'
create_atom_image('binaries', 'atom.img', 'boot.bin')