print("Путь к файлу: ")
path = input()
print("Сигнатура: ")
signature = int(input())
print("С какого байта: ")
start = int(input())

with open(path, "r+b") as f:
    f.seek(start)             
    f.write(bytes([signature]))
