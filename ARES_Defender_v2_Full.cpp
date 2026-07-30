// ARES_Defender_v3_FULL.cpp - Все функции v2.0 + MBR + Self-Protection + Monitoring
// Компиляция: cl /EHsc /MT /O2 /D_UNICODE /DUNICODE ARES_Defender_v3_FULL.cpp ^
//    user32.lib kernel32.lib advapi32.lib shell32.lib ole32.lib psapi.lib ^
//    ntdll.lib ws2_32.lib iphlpapi.lib shlwapi.lib comctl32.lib ^
//    crypt32.lib wininet.lib oleaut32.lib uuid.lib wintrust.lib ^
//    /link /SUBSYSTEM:WINDOWS

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0602
#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <wininet.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>
#include <regex>
#include <taskschd.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <srrestoreptapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "wintrust.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ========== ВСЕ СТРУКТУРЫ ==========
struct ProcessInfo {
    DWORD pid, ppid;
    std::wstring name, path, commandLine, owner;
    DWORD priority;
    bool isCritical, isSuspended, isProtected, isHidden;
    DWORD threadCount;
    HANDLE hProcess;
    double cpuUsage;
    SIZE_T memoryUsage;
    FILETIME createTime, userTime, kernelTime;
    std::vector<DWORD> threads;
};

struct ServiceInfo {
    std::wstring name, displayName, path, dependencies, description;
    DWORD state, startType;
    bool isRunning, isProtected;
    DWORD pid;
};

struct StartupItem {
    std::wstring name, path, location, hash;
    bool enabled, isMalicious;
    double riskScore;
    FILETIME modified;
};

struct NetworkConnection {
    DWORD pid;
    std::wstring localAddr, remoteAddr, processName, protocol;
    DWORD localPort, remotePort, state;
    bool isSuspicious;
};

struct FileInfo {
    std::wstring path, name, md5, sha1, sha256, version, company;
    DWORD attributes;
    ULONGLONG size;
    FILETIME created, modified, accessed;
    bool isSystem, isHidden, isReadOnly, isPacked, isSigned;
    double entropy;
};

struct RegistryChange {
    std::wstring key, value, data;
    DWORD action;
    FILETIME timestamp;
};

struct RootkitIndicator {
    std::wstring description, details;
    DWORD severity;
    bool detected;
};

struct QuarantineItem {
    std::wstring originalPath, quarantinePath, md5, reason;
    FILETIME timestamp;
    DWORD size;
};

struct SecurityEvent {
    DWORD eventId, severity;
    std::wstring source, description, user;
    FILETIME timestamp;
};

struct ProcessModule {
    std::wstring name, path, version;
    DWORD baseAddress, size;
    bool isSigned;
};

struct MBRBackup {
    std::wstring path;
    FILETIME timestamp;
    DWORD size;
    bool isValid;
};

// ========== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ==========
HWND g_hMainWnd = NULL, g_hStatusBar = NULL, g_hTabControl = NULL;
HWND g_hListViews[13] = {};
HINSTANCE g_hInst = NULL;
bool g_bAdmin = false, g_bMonitoring = false, g_bSelfProtect = false;
HANDLE g_hMutex = NULL, g_hMonitorThread = NULL, g_hSelfProtectThread = NULL;
std::set<std::wstring> g_protectedFolders;
std::map<DWORD, double> g_prevCpuTimes;
std::vector<MBRBackup> g_mbrBackups;
std::vector<QuarantineItem> g_quarantine;
std::vector<SecurityEvent> g_securityEvents;

// ========== ВСЕ УТИЛИТЫ (полный набор) ==========
bool IsAdmin() {
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    BOOL result = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0,0,0,0,0,0, &AdministratorsGroup);
    if (!result) return false;
    BOOL isAdmin = FALSE;
    CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin);
    FreeSid(AdministratorsGroup);
    return isAdmin != FALSE;
}

void SetStatusText(const wchar_t* text) {
    if (g_hStatusBar) SendMessageW(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)text);
}

std::wstring GetLastErrorStr() {
    wchar_t buf[256];
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buf, 256, NULL);
    return std::wstring(buf);
}

std::wstring TimeToString(FILETIME ft) {
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);
    wchar_t buf[64];
    wsprintfW(buf, L"%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
    return std::wstring(buf);
}

std::wstring GetFileMD5(const std::wstring& path) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return L"";
    if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CloseHandle(hFile); return L"";
    }
    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0); CloseHandle(hFile); return L"";
    }
    BYTE buffer[4096];
    DWORD bytesRead;
    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
        CryptHashData(hHash, buffer, bytesRead, 0);
    CloseHandle(hFile);
    BYTE hash[16];
    DWORD hashLen = 16;
    CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    wchar_t hashStr[33];
    for (int i = 0; i < 16; i++) wsprintfW(&hashStr[i*2], L"%02x", hash[i]);
    return std::wstring(hashStr);
}

std::wstring GetFileSHA1(const std::wstring& path) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return L"";
    if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CloseHandle(hFile); return L"";
    }
    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0); CloseHandle(hFile); return L"";
    }
    BYTE buffer[4096];
    DWORD bytesRead;
    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
        CryptHashData(hHash, buffer, bytesRead, 0);
    CloseHandle(hFile);
    BYTE hash[20];
    DWORD hashLen = 20;
    CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    wchar_t hashStr[41];
    for (int i = 0; i < 20; i++) wsprintfW(&hashStr[i*2], L"%02x", hash[i]);
    return std::wstring(hashStr);
}

std::wstring GetFileSHA256(const std::wstring& path) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return L"";
    if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        CloseHandle(hFile); return L"";
    }
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0); CloseHandle(hFile); return L"";
    }
    BYTE buffer[4096];
    DWORD bytesRead;
    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
        CryptHashData(hHash, buffer, bytesRead, 0);
    CloseHandle(hFile);
    BYTE hash[32];
    DWORD hashLen = 32;
    CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    wchar_t hashStr[65];
    for (int i = 0; i < 32; i++) wsprintfW(&hashStr[i*2], L"%02x", hash[i]);
    return std::wstring(hashStr);
}

double CalculateEntropy(const std::wstring& path) {
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0.0;
    BYTE buffer[4096];
    DWORD bytesRead;
    std::map<BYTE, DWORD> freq;
    DWORD total = 0;
    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        for (DWORD i = 0; i < bytesRead; i++) {
            freq[buffer[i]]++;
            total++;
        }
    }
    CloseHandle(hFile);
    if (total == 0) return 0.0;
    double entropy = 0.0;
    for (auto& pair : freq) {
        double p = (double)pair.second / total;
        entropy -= p * log2(p);
    }
    return entropy;
}

bool IsPacked(const std::wstring& path) {
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    IMAGE_DOS_HEADER dos;
    IMAGE_NT_HEADERS nt;
    DWORD bytesRead;
    if (!ReadFile(hFile, &dos, sizeof(dos), &bytesRead, NULL) || bytesRead != sizeof(dos)) {
        CloseHandle(hFile); return false;
    }
    if (dos.e_magic != IMAGE_DOS_SIGNATURE) {
        CloseHandle(hFile); return false;
    }
    SetFilePointer(hFile, dos.e_lfanew, NULL, FILE_BEGIN);
    if (!ReadFile(hFile, &nt, sizeof(nt), &bytesRead, NULL) || bytesRead != sizeof(nt)) {
        CloseHandle(hFile); return false;
    }
    CloseHandle(hFile);
    const wchar_t* packedSections[] = {
        L".upx", L".UPX", L"UPX0", L"UPX1", L".aspack", L".ASPack",
        L".nsp", L".NsP", L".MPRESS", L".mPRESS", L".packed", L".Packed",
        L".enigma", L".Enigma", L".vmp", L".VMP", L".themida", L".Themida"
    };
    IMAGE_SECTION_HEADER* sections = (IMAGE_SECTION_HEADER*)((BYTE*)&nt + sizeof(nt));
    for (int i = 0; i < nt.FileHeader.NumberOfSections; i++) {
        char secName[9] = {0};
        memcpy(secName, sections[i].Name, 8);
        wchar_t wName[9];
        MultiByteToWideChar(CP_ACP, 0, secName, -1, wName, 9);
        for (int j = 0; j < 18; j++) {
            if (wcsstr(wName, packedSections[j]) != NULL)
                return true;
        }
    }
    return false;
}

bool IsFileSigned(const std::wstring& path) {
    WINTRUST_FILE_INFO fileInfo;
    ZeroMemory(&fileInfo, sizeof(fileInfo));
    fileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    fileInfo.pcwszFilePath = path.c_str();
    
    WINTRUST_DATA wintrustData;
    ZeroMemory(&wintrustData, sizeof(wintrustData));
    wintrustData.cbStruct = sizeof(WINTRUST_DATA);
    wintrustData.dwUIChoice = WTD_UI_NONE;
    wintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    wintrustData.dwUnionChoice = WTD_CHOICE_FILE;
    wintrustData.pFile = &fileInfo;
    wintrustData.dwStateAction = WTD_STATEACTION_VERIFY;
    
    GUID actionGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    LONG status = WinVerifyTrust(NULL, &actionGUID, &wintrustData);
    
    wintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(NULL, &actionGUID, &wintrustData);
    return status == ERROR_SUCCESS;
}

std::wstring GetFileVersion(const std::wstring& path) {
    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSizeW(path.c_str(), &handle);
    if (size == 0) return L"";
    std::vector<BYTE> buffer(size);
    if (!GetFileVersionInfoW(path.c_str(), handle, size, buffer.data()))
        return L"";
    VS_FIXEDFILEINFO* versionInfo = NULL;
    UINT len = 0;
    if (!VerQueryValueW(buffer.data(), L"\\", (LPVOID*)&versionInfo, &len))
        return L"";
    wchar_t version[32];
    wsprintfW(version, L"%d.%d.%d.%d",
        HIWORD(versionInfo->dwFileVersionMS),
        LOWORD(versionInfo->dwFileVersionMS),
        HIWORD(versionInfo->dwFileVersionLS),
        LOWORD(versionInfo->dwFileVersionLS));
    return std::wstring(version);
}

std::wstring GetFileCompany(const std::wstring& path) {
    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSizeW(path.c_str(), &handle);
    if (size == 0) return L"";
    std::vector<BYTE> buffer(size);
    if (!GetFileVersionInfoW(path.c_str(), handle, size, buffer.data()))
        return L"";
    wchar_t* company = NULL;
    UINT len = 0;
    if (!VerQueryValueW(buffer.data(), L"\\StringFileInfo\\040904B0\\CompanyName", (LPVOID*)&company, &len))
        return L"";
    return std::wstring(company, len);
}

// ========== 1. ПРОЦЕССЫ (все функции v2.0) ==========
std::vector<ProcessInfo> EnumerateProcesses() {
    std::vector<ProcessInfo> result;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return result;
    std::map<DWORD, std::vector<DWORD>> processThreads;
    THREADENTRY32 te;
    te.dwSize = sizeof(THREADENTRY32);
    if (Thread32First(snapshot, &te)) {
        do {
            processThreads[te.th32OwnerProcessID].push_back(te.th32ThreadID);
        } while (Thread32Next(snapshot, &te));
    }
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);
    if (Process32FirstW(snapshot, &pe)) {
        do {
            ProcessInfo info = {};
            info.pid = pe.th32ProcessID;
            info.ppid = pe.th32ParentProcessID;
            info.name = pe.szExeFile;
            info.threads = processThreads[info.pid];
            info.threadCount = (DWORD)info.threads.size();
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | 
                PROCESS_TERMINATE | PROCESS_SUSPEND_RESUME | PROCESS_SET_INFORMATION, FALSE, info.pid);
            if (hProc) {
                WCHAR path[MAX_PATH];
                DWORD size = MAX_PATH;
                if (QueryFullProcessImageNameW(hProc, 0, path, &size)) info.path = path;
                info.priority = GetPriorityClass(hProc);
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc)))
                    info.memoryUsage = pmc.WorkingSetSize;
                
                // GetProcessInformation requires _WIN32_WINNT >= 0x0602, already defined
                DWORD protection = 0;
                if (GetProcessInformation(hProc, ProcessProtectionLevelInfo, &protection, sizeof(protection)))
                    info.isProtected = (protection != 0);
                    
                FILETIME createTime, exitTime, kernelTime, userTime;
                if (GetProcessTimes(hProc, &createTime, &exitTime, &kernelTime, &userTime)) {
                    info.createTime = createTime;
                    info.kernelTime = kernelTime;
                    info.userTime = userTime;
                    if (g_prevCpuTimes.find(info.pid) != g_prevCpuTimes.end()) {
                        ULONGLONG totalTime = ((ULONGLONG)kernelTime.dwHighDateTime << 32) | kernelTime.dwLowDateTime;
                        totalTime += ((ULONGLONG)userTime.dwHighDateTime << 32) | userTime.dwLowDateTime;
                        info.cpuUsage = (totalTime - g_prevCpuTimes[info.pid]) / 10000.0;
                    }
                    g_prevCpuTimes[info.pid] = ((ULONGLONG)kernelTime.dwHighDateTime << 32) | kernelTime.dwLowDateTime;
                    g_prevCpuTimes[info.pid] += ((ULONGLONG)userTime.dwHighDateTime << 32) | userTime.dwLowDateTime;
                }
                info.hProcess = hProc;
            }
            result.push_back(info);
        } while (Process32NextW(snapshot, &pe));
    }
    CloseHandle(snapshot);
    return result;
}

bool TerminateProcessEx(DWORD pid) {
    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProc) return false;
    bool result = TerminateProcess(hProc, 1) != 0;
    CloseHandle(hProc);
    return result;
}

bool SuspendProcess(DWORD pid) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;
    THREADENTRY32 te;
    te.dwSize = sizeof(THREADENTRY32);
    if (Thread32First(hSnapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread) { SuspendThread(hThread); CloseHandle(hThread); }
            }
        } while (Thread32Next(hSnapshot, &te));
    }
    CloseHandle(hSnapshot);
    return true;
}

bool ResumeProcess(DWORD pid) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;
    THREADENTRY32 te;
    te.dwSize = sizeof(THREADENTRY32);
    if (Thread32First(hSnapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread) { ResumeThread(hThread); CloseHandle(hThread); }
            }
        } while (Thread32Next(hSnapshot, &te));
    }
    CloseHandle(hSnapshot);
    return true;
}

bool SetProcessCritical(DWORD pid, bool critical) {
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    typedef NTSTATUS(NTAPI* RtlSetProcessIsCritical_t)(HANDLE, BOOLEAN, BOOLEAN);
    auto RtlSetProcessIsCritical = (RtlSetProcessIsCritical_t)GetProcAddress(ntdll, "RtlSetProcessIsCritical");
    if (!RtlSetProcessIsCritical) return false;
    HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (!hProc) return false;
    NTSTATUS status = RtlSetProcessIsCritical(hProc, critical ? TRUE : FALSE, FALSE);
    CloseHandle(hProc);
    return status == 0;
}

bool SetProcessPriority(DWORD pid, DWORD priorityClass) {
    HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (!hProc) return false;
    bool result = SetPriorityClass(hProc, priorityClass) != 0;
    CloseHandle(hProc);
    return result;
}

bool SetProcessAffinity(DWORD pid, DWORD_PTR mask) {
    HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (!hProc) return false;
    bool result = SetProcessAffinityMask(hProc, mask) != 0;
    CloseHandle(hProc);
    return result;
}

bool GetProcessMemoryDump(DWORD pid, const std::wstring& path) {
    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProc) return false;
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) { CloseHandle(hProc); return false; }
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    MEMORY_BASIC_INFORMATION mbi;
    DWORD_PTR addr = 0;
    DWORD written;
    while (addr < (DWORD_PTR)si.lpMaximumApplicationAddress) {
        if (VirtualQueryEx(hProc, (LPCVOID)addr, &mbi, sizeof(mbi)) == 0) break;
        if (mbi.State == MEM_COMMIT && (mbi.Protect & PAGE_READWRITE || mbi.Protect & PAGE_EXECUTE_READWRITE)) {
            std::vector<BYTE> buffer(mbi.RegionSize);
            SIZE_T read;
            if (ReadProcessMemory(hProc, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &read)) {
                WriteFile(hFile, buffer.data(), (DWORD)read, &written, NULL);
            }
        }
        addr += mbi.RegionSize;
    }
    CloseHandle(hFile);
    CloseHandle(hProc);
    return true;
}

bool HideProcess(DWORD pid) {
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    typedef NTSTATUS(NTAPI* NtSetInformationProcess_t)(HANDLE, DWORD, PVOID, ULONG);
    auto NtSetInformationProcess = (NtSetInformationProcess_t)GetProcAddress(ntdll, "NtSetInformationProcess");
    if (!NtSetInformationProcess) return false;
    HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (!hProc) return false;
    DWORD hideFlag = 1;
    NTSTATUS status = NtSetInformationProcess(hProc, 0x1D, &hideFlag, sizeof(hideFlag));
    CloseHandle(hProc);
    return status == 0;
}

bool ProtectProcess(DWORD pid) {
    HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (!hProc) return false;
    DWORD protection = PROCESS_PROTECTION_LEVEL_WINDOWS;
    bool result = SetProcessInformation(hProc, ProcessProtectionLevelInfo, &protection, sizeof(protection)) != 0;
    CloseHandle(hProc);
    return result;
}

std::vector<ProcessModule> GetProcessModules(DWORD pid) {
    std::vector<ProcessModule> result;
    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProc) return result;
    HMODULE modules[1024];
    DWORD needed;
    if (EnumProcessModules(hProc, modules, sizeof(modules), &needed)) {
        for (DWORD i = 0; i < needed / sizeof(HMODULE); i++) {
            ProcessModule mod;
            WCHAR name[MAX_PATH], path[MAX_PATH];
            if (GetModuleBaseNameW(hProc, modules[i], name, MAX_PATH)) mod.name = name;
            if (GetModuleFileNameExW(hProc, modules[i], path, MAX_PATH)) mod.path = path;
            MODULEINFO modInfo;
            if (GetModuleInformation(hProc, modules[i], &modInfo, sizeof(modInfo))) {
                mod.baseAddress = (DWORD)(ULONG_PTR)modInfo.lpBaseOfDll;
                mod.size = modInfo.SizeOfImage;
            }
            mod.isSigned = IsFileSigned(mod.path);
            mod.version = GetFileVersion(mod.path);
            result.push_back(mod);
        }
    }
    CloseHandle(hProc);
    return result;
}

// ========== 2. СЛУЖБЫ (все функции v2.0) ==========
std::vector<ServiceInfo> EnumerateServices() {
    std::vector<ServiceInfo> result;
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm) return result;
    DWORD bytesNeeded, servicesReturned;
    EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
        NULL, 0, &bytesNeeded, &servicesReturned, NULL, NULL);
    std::vector<BYTE> buffer(bytesNeeded);
    if (EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
        buffer.data(), bytesNeeded, &bytesNeeded, &servicesReturned, NULL, NULL)) {
        auto services = (ENUM_SERVICE_STATUS_PROCESSW*)buffer.data();
        for (DWORD i = 0; i < servicesReturned; i++) {
            ServiceInfo info;
            info.name = services[i].lpServiceName;
            info.displayName = services[i].lpDisplayName;
            info.state = services[i].ServiceStatusProcess.dwCurrentState;
            info.startType = services[i].ServiceStatusProcess.dwServiceType;
            info.isRunning = (info.state == SERVICE_RUNNING);
            info.pid = services[i].ServiceStatusProcess.dwProcessId;
            SC_HANDLE svc = OpenServiceW(scm, info.name.c_str(), SERVICE_QUERY_CONFIG);
            if (svc) {
                DWORD sizeNeeded;
                QueryServiceConfigW(svc, NULL, 0, &sizeNeeded);
                std::vector<BYTE> cfgBuffer(sizeNeeded);
                if (QueryServiceConfigW(svc, (LPQUERY_SERVICE_CONFIGW)cfgBuffer.data(), sizeNeeded, &sizeNeeded)) {
                    auto cfg = (LPQUERY_SERVICE_CONFIGW)cfgBuffer.data();
                    info.path = cfg->lpBinaryPathName;
                    if (cfg->lpDependencies && *cfg->lpDependencies) info.dependencies = cfg->lpDependencies;
                }
                DWORD descSize;
                QueryServiceConfig2W(svc, SERVICE_CONFIG_DESCRIPTION, NULL, 0, &descSize);
                std::vector<BYTE> descBuffer(descSize);
                if (QueryServiceConfig2W(svc, SERVICE_CONFIG_DESCRIPTION, descBuffer.data(), descSize, &descSize)) {
                    auto desc = (SERVICE_DESCRIPTIONW*)descBuffer.data();
                    if (desc->lpDescription) info.description = desc->lpDescription;
                }
                CloseServiceHandle(svc);
            }
            result.push_back(info);
        }
    }
    CloseServiceHandle(scm);
    return result;
}

bool ControlServiceEx(const std::wstring& name, DWORD control) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return false;
    SC_HANDLE svc = OpenServiceW(scm, name.c_str(), SERVICE_STOP | SERVICE_START | SERVICE_PAUSE_CONTINUE | SERVICE_QUERY_STATUS);
    if (!svc) { CloseServiceHandle(scm); return false; }
    SERVICE_STATUS status;
    bool result = ControlService(svc, control, &status) != 0;
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return result;
}

bool SetServiceStartType(const std::wstring& name, DWORD startType) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return false;
    SC_HANDLE svc = OpenServiceW(scm, name.c_str(), SERVICE_CHANGE_CONFIG);
    if (!svc) { CloseServiceHandle(scm); return false; }
    bool result = ChangeServiceConfigW(svc, SERVICE_NO_CHANGE, startType,
        SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != 0;
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return result;
}

bool CreateService(const std::wstring& name, const std::wstring& displayName, 
                   const std::wstring& path, DWORD startType) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!scm) return false;
    SC_HANDLE svc = CreateServiceW(scm, name.c_str(), displayName.c_str(),
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, startType,
        SERVICE_ERROR_NORMAL, path.c_str(), NULL, NULL, NULL, NULL, NULL);
    if (!svc) { CloseServiceHandle(scm); return false; }
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return true;
}

bool DeleteService(const std::wstring& name) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return false;
    SC_HANDLE svc = OpenServiceW(scm, name.c_str(), DELETE);
    if (!svc) { CloseServiceHandle(scm); return false; }
    bool result = DeleteService(svc) != 0;
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return result;
}

// ========== 3. АВТОЗАГРУЗКА (все функции v2.0) ==========
std::vector<StartupItem> EnumerateStartup() {
    std::vector<StartupItem> result;
    HKEY keys[] = {HKEY_LOCAL_MACHINE, HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, HKEY_CURRENT_USER,
                   HKEY_LOCAL_MACHINE, HKEY_LOCAL_MACHINE, HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER};
    const wchar_t* subkeys[] = {
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices",
        L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\BootExecute",
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Userinit",
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Shell"
    };
    const wchar_t* locations[] = {
        L"HKLM\\Run", L"HKLM\\RunOnce", L"HKCU\\Run", L"HKCU\\RunOnce",
        L"HKLM\\RunServices", L"BootExecute", L"Userinit", L"Shell"
    };
    for (int i = 0; i < 8; i++) {
        HKEY hKey;
        if (RegOpenKeyExW(keys[i], subkeys[i], 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD index = 0;
            wchar_t valueName[256], data[1024];
            DWORD valueNameSize = 256, dataSize = 1024;
            while (RegEnumValueW(hKey, index++, valueName, &valueNameSize, NULL, NULL,
                (LPBYTE)data, &dataSize) == ERROR_SUCCESS) {
                StartupItem item;
                item.name = valueName; item.path = data; item.location = locations[i];
                item.enabled = true; item.isMalicious = false; item.riskScore = 0.0;
                item.hash = GetFileMD5(data);
                if (wcsstr(data, L"temp") || wcsstr(data, L"tmp") || wcsstr(data, L"download")) item.riskScore += 0.3;
                if (wcsstr(data, L".vbs") || wcsstr(data, L".js") || wcsstr(data, L".ps1") || wcsstr(data, L".scr")) item.riskScore += 0.4;
                if (GetFileAttributesW(data) == INVALID_FILE_ATTRIBUTES) { item.riskScore += 0.5; item.isMalicious = true; }
                if (!IsFileSigned(data)) item.riskScore += 0.2;
                if (item.riskScore > 0.5) item.isMalicious = true;
                result.push_back(item);
                valueNameSize = 256; dataSize = 1024;
            }
            RegCloseKey(hKey);
        }
    }
    wchar_t startupPaths[2][MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_COMMON_STARTUP, NULL, 0, startupPaths[0]);
    SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, startupPaths[1]);
    for (int i = 0; i < 2; i++) {
        if (wcslen(startupPaths[i]) == 0) continue;
        std::wstring searchPath = std::wstring(startupPaths[i]) + L"\\*";
        WIN32_FIND_DATAW fd;
        HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    StartupItem item;
                    item.name = fd.cFileName;
                    item.path = std::wstring(startupPaths[i]) + L"\\" + fd.cFileName;
                    item.location = (i == 0) ? L"Common Startup" : L"User Startup";
                    item.enabled = true; item.isMalicious = false; item.riskScore = 0.0;
                    item.hash = GetFileMD5(item.path);
                    std::wstring ext = PathFindExtensionW(item.path.c_str());
                    if (ext == L".vbs" || ext == L".js" || ext == L".ps1" || ext == L".scr" || ext == L".com") {
                        item.riskScore += 0.5; item.isMalicious = true;
                    }
                    if (!IsFileSigned(item.path)) item.riskScore += 0.2;
                    result.push_back(item);
                }
            } while (FindNextFileW(hFind, &fd));
            FindClose(hFind);
        }
    }
    CoInitialize(NULL);
    ITaskScheduler* pScheduler = NULL;
    if (CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pScheduler) == S_OK) {
        IEnumWorkItems* pEnum = NULL;
        if (pScheduler->Enum(&pEnum) == S_OK) {
            LPWSTR* pNames = NULL;
            DWORD count = 0;
            while (pEnum->Next(10, &pNames, &count) == S_OK && count > 0) {
                for (DWORD j = 0; j < count; j++) {
                    ITask* pTask = NULL;
                    if (pScheduler->Activate(pNames[j], IID_ITask, (IUnknown**)&pTask) == S_OK) {
                        StartupItem item;
                        item.name = pNames[j]; item.location = L"Task Scheduler";
                        item.enabled = true; item.isMalicious = false; item.riskScore = 0.0;
                        LPWSTR appName = NULL;
                        if (pTask->GetApplicationName(&appName) == S_OK && appName) {
                            item.path = appName; item.hash = GetFileMD5(appName);
                            CoTaskMemFree(appName);
                        }
                        result.push_back(item);
                        pTask->Release();
                    }
                    CoTaskMemFree(pNames[j]);
                }
                CoTaskMemFree(pNames);
            }
            pEnum->Release();
        }
        pScheduler->Release();
    }
    CoUninitialize();
    return result;
}

bool RemoveStartupItem(const std::wstring& name, const std::wstring& location) {
    if (location.find(L"HKLM") != std::wstring::npos) {
        HKEY hKey;
        const wchar_t* subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
        if (location.find(L"RunOnce") != std::wstring::npos) subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
        else if (location.find(L"RunServices") != std::wstring::npos) subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices";
        else if (location.find(L"BootExecute") != std::wstring::npos) subkey = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\BootExecute";
        else if (location.find(L"Userinit") != std::wstring::npos || location.find(L"Shell") != std::wstring::npos)
            subkey = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon";
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subkey, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            bool result = RegDeleteValueW(hKey, name.c_str()) == ERROR_SUCCESS;
            RegCloseKey(hKey);
            return result;
        }
    } else if (location.find(L"HKCU") != std::wstring::npos) {
        HKEY hKey;
        const wchar_t* subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
        if (location.find(L"RunOnce") != std::wstring::npos) subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
        if (RegOpenKeyExW(HKEY_CURRENT_USER, subkey, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            bool result = RegDeleteValueW(hKey, name.c_str()) == ERROR_SUCCESS;
            RegCloseKey(hKey);
            return result;
        }
    } else if (location.find(L"Startup") != std::wstring::npos) {
        wchar_t startupPath[MAX_PATH];
        if (SHGetFolderPathW(NULL, location.find(L"Common") != std::wstring::npos ? CSIDL_COMMON_STARTUP : CSIDL_STARTUP,
            NULL, 0, startupPath) == S_OK) {
            std::wstring fullPath = std::wstring(startupPath) + L"\\" + name;
            return DeleteFileW(fullPath.c_str()) != 0;
        }
    } else if (location.find(L"Task Scheduler") != std::wstring::npos) {
        CoInitialize(NULL);
        ITaskScheduler* pScheduler = NULL;
        if (CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pScheduler) == S_OK) {
            HRESULT hr = pScheduler->Delete(name.c_str());
            pScheduler->Release();
            CoUninitialize();
            return hr == S_OK;
        }
        CoUninitialize();
        return false;
    }
    return false;
}

// ========== 4. СЕТЬ (все функции v2.0) ==========
std::vector<NetworkConnection> EnumerateNetworkConnections() {
    std::vector<NetworkConnection> result;
    MIB_TCPTABLE_OWNER_PID* tcpTable = NULL;
    DWORD size = 0;
    GetExtendedTcpTable(NULL, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    tcpTable = (MIB_TCPTABLE_OWNER_PID*)malloc(size);
    if (GetExtendedTcpTable(tcpTable, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
        for (DWORD i = 0; i < tcpTable->dwNumEntries; i++) {
            NetworkConnection conn;
            conn.pid = tcpTable->table[i].dwOwningPid;
            conn.localPort = ntohs((u_short)tcpTable->table[i].dwLocalPort);
            conn.remotePort = ntohs((u_short)tcpTable->table[i].dwRemotePort);
            conn.state = tcpTable->table[i].dwState;
            conn.protocol = L"TCP";
            conn.isSuspicious = false;
            IN_ADDR localAddr, remoteAddr;
            localAddr.S_un.S_addr = tcpTable->table[i].dwLocalAddr;
            remoteAddr.S_un.S_addr = tcpTable->table[i].dwRemoteAddr;
            wchar_t addrBuf[INET_ADDRSTRLEN];
            InetNtopW(AF_INET, &localAddr, addrBuf, INET_ADDRSTRLEN);
            conn.localAddr = addrBuf;
            InetNtopW(AF_INET, &remoteAddr, addrBuf, INET_ADDRSTRLEN);
            conn.remoteAddr = addrBuf;
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, conn.pid);
            if (hProc) {
                WCHAR procName[MAX_PATH];
                DWORD size2 = MAX_PATH;
                if (QueryFullProcessImageNameW(hProc, 0, procName, &size2))
                    conn.processName = PathFindFileNameW(procName);
                CloseHandle(hProc);
            }
            if (conn.remotePort == 4444 || conn.remotePort == 5555 || conn.remotePort == 6666 || conn.remotePort == 1337 || conn.remotePort == 31337)
                conn.isSuspicious = true;
            result.push_back(conn);
        }
    }
    free(tcpTable);
    MIB_UDPTABLE_OWNER_PID* udpTable = NULL;
    size = 0;
    GetExtendedUdpTable(NULL, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
    udpTable = (MIB_UDPTABLE_OWNER_PID*)malloc(size);
    if (GetExtendedUdpTable(udpTable, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0) == NO_ERROR) {
        for (DWORD i = 0; i < udpTable->dwNumEntries; i++) {
            NetworkConnection conn;
            conn.pid = udpTable->table[i].dwOwningPid;
            conn.localPort = ntohs((u_short)udpTable->table[i].dwLocalPort);
            conn.remotePort = 0;
            conn.state = 0;
            conn.protocol = L"UDP";
            conn.isSuspicious = false;
            IN_ADDR localAddr;
            localAddr.S_un.S_addr = udpTable->table[i].dwLocalAddr;
            wchar_t addrBuf[INET_ADDRSTRLEN];
            InetNtopW(AF_INET, &localAddr, addrBuf, INET_ADDRSTRLEN);
            conn.localAddr = addrBuf;
            conn.remoteAddr = L"*";
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, conn.pid);
            if (hProc) {
                WCHAR procName[MAX_PATH];
                DWORD size2 = MAX_PATH;
                if (QueryFullProcessImageNameW(hProc, 0, procName, &size2))
                    conn.processName = PathFindFileNameW(procName);
                CloseHandle(hProc);
            }
            result.push_back(conn);
        }
    }
    free(udpTable);
    return result;
}

bool ResetNetworkStack() {
    system("netsh winsock reset > nul");
    system("netsh int ip reset > nul");
    system("ipconfig /flushdns > nul");
    system("arp -d * > nul");
    return true;
}

bool BlockPort(DWORD port) {
    char cmd[256];
    sprintf_s(cmd, sizeof(cmd), "netsh advfirewall firewall add rule name=\"Block Port %d\" protocol=TCP dir=in localport=%d action=block", port, port);
    system(cmd);
    sprintf_s(cmd, sizeof(cmd), "netsh advfirewall firewall add rule name=\"Block Port %d\" protocol=TCP dir=out localport=%d action=block", port, port);
    system(cmd);
    return true;
}

bool UnblockPort(DWORD port) {
    char cmd[256];
    sprintf_s(cmd, sizeof(cmd), "netsh advfirewall firewall delete rule name=\"Block Port %d\"", port);
    system(cmd);
    return true;
}

// ========== 5. ВОССТАНОВЛЕНИЕ СИСТЕМЫ (все функции v2.0) ==========
bool CreateRestorePoint(const std::wstring& description) {
    HMODULE hAdvApi = LoadLibraryW(L"advapi32.dll");
    if (!hAdvApi) return false;
    typedef BOOL(WINAPI* SRSetRestorePointW_t)(PRESTOREPOINTINFOW, PSTATEMGRSTATUS);
    auto SRSetRestorePointW = (SRSetRestorePointW_t)GetProcAddress(hAdvApi, "SRSetRestorePointW");
    if (!SRSetRestorePointW) { FreeLibrary(hAdvApi); return false; }
    RESTOREPOINTINFOW rpInfo;
    ZeroMemory(&rpInfo, sizeof(rpInfo));
    rpInfo.dwEventType = BEGIN_SYSTEM_CHANGE;
    rpInfo.dwRestorePtType = APPLICATION_INSTALL;
    wcscpy_s(rpInfo.szDescription, 256, description.c_str());
    STATEMGRSTATUS status;
    ZeroMemory(&status, sizeof(status));
    bool result = SRSetRestorePointW(&rpInfo, &status) != 0;
    FreeLibrary(hAdvApi);
    return result;
}

bool RunSFCScan() {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    wchar_t cmd[] = L"sfc /scannow";
    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
        return true;
    }
    return false;
}

bool RunDISM() {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    wchar_t cmd[] = L"DISM /Online /Cleanup-Image /RestoreHealth";
    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
        return true;
    }
    return false;
}

bool RepairBoot() {
    system("bootrec /fixmbr > nul");
    system("bootrec /fixboot > nul");
    system("bootrec /scanos > nul");
    system("bootrec /rebuildbcd > nul");
    return true;
}

bool FixFilePermissions(const std::wstring& path) {
    char cmd[512];
    sprintf_s(cmd, sizeof(cmd), "takeown /f \"%S\" /r /d y > nul", path.c_str());
    system(cmd);
    sprintf_s(cmd, sizeof(cmd), "icacls \"%S\" /grant administrators:F /t > nul", path.c_str());
    system(cmd);
    return true;
}

bool CleanTempFiles() {
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring searchPath = std::wstring(tempPath) + L"*";
    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::wstring fullPath = std::wstring(tempPath) + fd.cFileName;
                DeleteFileW(fullPath.c_str());
            }
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
    }
    return true;
}

bool CleanPrefetch() {
    wchar_t prefetchPath[MAX_PATH];
    GetWindowsDirectoryW(prefetchPath, MAX_PATH);
    wcscat_s(prefetchPath, L"\\Prefetch\\*.pf");
    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(prefetchPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::wstring fullPath = std::wstring(prefetchPath) + fd.cFileName;
            DeleteFileW(fullPath.c_str());
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
    }
    return true;
}

// ========== 6. MBR RECOVERY (НОВАЯ КАТЕГОРИЯ) ==========
bool BackupMBR(const std::wstring& backupPath) {
    HANDLE hDrive = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) return false;
    HANDLE hFile = CreateFileW(backupPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) { CloseHandle(hDrive); return false; }
    BYTE mbr[512];
    DWORD bytesRead;
    bool result = false;
    if (ReadFile(hDrive, mbr, 512, &bytesRead, NULL) && bytesRead == 512) {
        DWORD written;
        result = WriteFile(hFile, mbr, 512, &written, NULL) && written == 512;
        if (result) {
            MBRBackup backup; backup.path = backupPath; GetSystemTimeAsFileTime(&backup.timestamp);
            backup.size = 512; backup.isValid = true; g_mbrBackups.push_back(backup);
            SetStatusText(L"MBR бэкап создан");
        }
    }
    CloseHandle(hFile); CloseHandle(hDrive);
    return result;
}

bool RestoreMBRFromBackup(const std::wstring& backupPath) {
    HANDLE hFile = CreateFileW(backupPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    HANDLE hDrive = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) { CloseHandle(hFile); return false; }
    BYTE mbr[512];
    DWORD bytesRead;
    bool result = false;
    if (ReadFile(hFile, mbr, 512, &bytesRead, NULL) && bytesRead == 512) {
        DWORD written;
        result = WriteFile(hDrive, mbr, 512, &written, NULL) && written == 512;
        if (result) SetStatusText(L"MBR восстановлен из бэкапа");
    }
    CloseHandle(hDrive); CloseHandle(hFile);
    return result;
}

bool RestoreStandardMBR() {
    BYTE standardMBR[512] = {0};
    standardMBR[510] = 0x55; standardMBR[511] = 0xAA;
    standardMBR[446] = 0x80; standardMBR[450] = 0x07;
    HANDLE hDrive = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) return false;
    DWORD written;
    bool result = WriteFile(hDrive, standardMBR, 512, &written, NULL) && written == 512;
    CloseHandle(hDrive);
    if (result) SetStatusText(L"Стандартный MBR восстановлен");
    return result;
}

bool RepairMBR_BootRec() {
    bool result = true;
    result &= (system("bootrec /fixmbr > nul") == 0);
    result &= (system("bootrec /fixboot > nul") == 0);
    result &= (system("bootrec /scanos > nul") == 0);
    result &= (system("bootrec /rebuildbcd > nul") == 0);
    if (result) SetStatusText(L"MBR восстановлен через bootrec");
    return result;
}

bool RepairMBR_BootSect() {
    bool result = (system("bootsect /nt60 C: /mbr > nul") == 0);
    if (result) SetStatusText(L"MBR восстановлен через bootsect");
    return result;
}

bool RepairMBR_BCD() {
    bool result = (system("bcdboot C:\\Windows /s C: /f ALL > nul") == 0);
    if (result) SetStatusText(L"BCD восстановлен");
    return result;
}

bool WriteMBRDirect(BYTE* mbrData, DWORD size) {
    if (size != 512) return false;
    HANDLE hDrive = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) return false;
    DWORD written;
    bool result = WriteFile(hDrive, mbrData, size, &written, NULL) && written == size;
    CloseHandle(hDrive);
    return result;
}

bool CheckMBRIntegrity() {
    HANDLE hDrive = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) return false;
    BYTE mbr[512];
    DWORD bytesRead;
    if (!ReadFile(hDrive, mbr, 512, &bytesRead, NULL) || bytesRead != 512) { CloseHandle(hDrive); return false; }
    CloseHandle(hDrive);
    if (mbr[510] != 0x55 || mbr[511] != 0xAA) return false;
    for (int i = 0; i < 4; i++) {
        if (mbr[446 + i * 16 + 4] == 0x80) return true;
    }
    return false;
}

bool CreateBootableUSB(const std::wstring& driveLetter) {
    HANDLE hDrive = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) return false;
    std::wstring usbPath = L"\\\\.\\" + driveLetter;
    HANDLE hUSB = CreateFileW(usbPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hUSB == INVALID_HANDLE_VALUE) { CloseHandle(hDrive); return false; }
    BYTE mbr[512];
    DWORD bytesRead, written;
    bool result = false;
    if (ReadFile(hDrive, mbr, 512, &bytesRead, NULL) && bytesRead == 512) {
        result = WriteFile(hUSB, mbr, 512, &written, NULL) && written == 512;
        if (result) SetStatusText(L"Загрузочная USB создана");
    }
    CloseHandle(hUSB); CloseHandle(hDrive);
    return result;
}

std::vector<MBRBackup> GetMBRBackups() { return g_mbrBackups; }

// ========== 7. САМОЗАЩИТА (НОВАЯ) ==========
DWORD WINAPI SelfProtectThread(LPVOID lpParam) {
    while (g_bSelfProtect) {
        Sleep(1000);
        SetProcessCritical(GetCurrentProcessId(), true);
        wchar_t currentPath[MAX_PATH];
        GetModuleFileNameW(NULL, currentPath, MAX_PATH);
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            RegSetValueExW(hKey, L"ARES_Defender", 0, REG_SZ, (BYTE*)currentPath, (wcslen(currentPath) + 1) * 2);
            RegCloseKey(hKey);
        }
    }
    return 0;
}

bool EnableSelfProtection() {
    if (g_bSelfProtect) return true;
    g_bSelfProtect = true;
    g_hSelfProtectThread = CreateThread(NULL, 0, SelfProtectThread, NULL, 0, NULL);
    if (g_hSelfProtectThread) { SetStatusText(L"Самозащита активирована"); return true; }
    return false;
}

bool DisableSelfProtection() {
    g_bSelfProtect = false;
    if (g_hSelfProtectThread) { WaitForSingleObject(g_hSelfProtectThread, 3000); CloseHandle(g_hSelfProtectThread); g_hSelfProtectThread = NULL; }
    SetStatusText(L"Самозащита отключена");
    return true;
}

bool AddToStartup() {
    wchar_t currentPath[MAX_PATH];
    GetModuleFileNameW(NULL, currentPath, MAX_PATH);
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExW(hKey, L"ARES_Defender", 0, REG_SZ, (BYTE*)currentPath, (wcslen(currentPath) + 1) * 2);
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool RemoveFromStartup() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"ARES_Defender");
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

// ========== 8. МОНИТОРИНГ (НОВАЯ) ==========
DWORD WINAPI SystemMonitorThread(LPVOID lpParam) {
    while (g_bMonitoring) {
        Sleep(2000);
        auto processes = EnumerateProcesses();
        for (const auto& p : processes) {
            std::wstring name = p.name;
            std::transform(name.begin(), name.end(), name.begin(), ::towlower);
            if (name.find(L"crypt") != std::wstring::npos || name.find(L"encrypt") != std::wstring::npos ||
                name.find(L"ransom") != std::wstring::npos || name.find(L"wanna") != std::wstring::npos ||
                name.find(L"petya") != std::wstring::npos || name.find(L"lock") != std::wstring::npos ||
                name.find(L"bad") != std::wstring::npos || name.find(L"virus") != std::wstring::npos) {
                SetStatusText(L"ВНИМАНИЕ: Обнаружен подозрительный процесс!");
                SuspendProcess(p.pid);
            }
        }
        auto connections = EnumerateNetworkConnections();
        for (const auto& c : connections) {
            if (c.isSuspicious) {
                SetStatusText(L"ВНИМАНИЕ: Подозрительное соединение!");
                BlockPort(c.remotePort);
            }
        }
        MEMORYSTATUSEX memStatus = { sizeof(MEMORYSTATUSEX) };
        GlobalMemoryStatusEx(&memStatus);
        if (memStatus.dwMemoryLoad > 90) SetStatusText(L"Предупреждение: Высокая загрузка памяти!");
    }
    return 0;
}

bool StartSystemMonitoring() {
    if (g_bMonitoring) return false;
    g_bMonitoring = true;
    g_hMonitorThread = CreateThread(NULL, 0, SystemMonitorThread, NULL, 0, NULL);
    if (g_hMonitorThread) { SetStatusText(L"Мониторинг запущен"); return true; }
    return false;
}

bool StopSystemMonitoring() {
    if (!g_bMonitoring) return false;
    g_bMonitoring = false;
    WaitForSingleObject(g_hMonitorThread, 5000);
    CloseHandle(g_hMonitorThread);
    SetStatusText(L"Мониторинг остановлен");
    return true;
}

// ========== 9. РАЗБЛОКИРОВКА (все функции v2.0) ==========
bool UnlockRegistry() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"DisableRegistryTools"); RegCloseKey(hKey);
    }
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"DisableRegistryTools"); RegCloseKey(hKey);
    }
    return true;
}

bool UnlockCMD() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Policies\\Microsoft\\Windows\\System", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"DisableCMD"); RegCloseKey(hKey);
    }
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Policies\\Microsoft\\Windows\\System", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"DisableCMD"); RegCloseKey(hKey);
    }
    return true;
}

bool UnlockTaskManager() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"DisableTaskMgr"); RegCloseKey(hKey);
    }
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"DisableTaskMgr"); RegCloseKey(hKey);
    }
    return true;
}

bool UnlockControlPanel() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"NoControlPanel"); RegDeleteValueW(hKey, L"DisallowCpl"); RegCloseKey(hKey);
    }
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"NoControlPanel"); RegDeleteValueW(hKey, L"DisallowCpl"); RegCloseKey(hKey);
    }
    return true;
}

bool UnlockGroupPolicy() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Policies\\Microsoft\\Windows\\System", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"DisableGPO"); RegCloseKey(hKey);
    }
    return true;
}

// ========== 10. ЗАЩИТА (все функции v2.0) ==========
bool EnableDefender() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\Microsoft\\Windows Defender", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        DWORD value = 0;
        RegSetValueExW(hKey, L"DisableAntiSpyware", 0, REG_DWORD, (BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
    }
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\Microsoft\\Windows Defender\\Real-Time Protection", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        DWORD value = 0;
        RegSetValueExW(hKey, L"DisableRealtimeMonitoring", 0, REG_DWORD, (BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
    }
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm) {
        SC_HANDLE svc = OpenServiceW(scm, L"WinDefend", SERVICE_START);
        if (svc) { StartServiceW(svc, 0, NULL); CloseServiceHandle(svc); }
        CloseServiceHandle(scm);
    }
    system("\"C:\\Program Files\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 2 > nul");
    return true;
}

bool EnableUAC() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        DWORD value = 5;
        RegSetValueExW(hKey, L"ConsentPromptBehaviorAdmin", 0, REG_DWORD, (BYTE*)&value, sizeof(value));
        value = 1;
        RegSetValueExW(hKey, L"EnableLUA", 0, REG_DWORD, (BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
    }
    return true;
}

bool EnableFirewall() {
    system("netsh advfirewall set allprofiles state on > nul");
    system("netsh advfirewall set currentprofile settings inboundusernotification enable > nul");
    system("netsh advfirewall set currentprofile settings outboundusernotification enable > nul");
    return true;
}

bool RunFullScan() {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    wchar_t cmd[] = L"\"C:\\Program Files\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 2";
    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
        return true;
    }
    return false;
}

bool RunQuickScan() {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    wchar_t cmd[] = L"\"C:\\Program Files\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 1";
    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
        return true;
    }
    return false;
}

bool UpdateDefender() {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    wchar_t cmd[] = L"\"C:\\Program Files\\Windows Defender\\MpCmdRun.exe\" -SignatureUpdate";
    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
        return true;
    }
    return false;
}

// ========== 11. ОБНАРУЖЕНИЕ РУТКИТОВ (v2.0) ==========
std::vector<RootkitIndicator> DetectRootkits() {
    std::vector<RootkitIndicator> result;
    std::set<DWORD> pids;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(PROCESSENTRY32W);
        if (Process32FirstW(snapshot, &pe)) {
            do { pids.insert(pe.th32ProcessID); } while (Process32NextW(snapshot, &pe));
        }
        CloseHandle(snapshot);
    }
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    typedef NTSTATUS(NTAPI* NtQuerySystemInformation_t)(DWORD, PVOID, ULONG, PULONG);
    auto NtQuerySystemInformation = (NtQuerySystemInformation_t)GetProcAddress(ntdll, "NtQuerySystemInformation");
    if (NtQuerySystemInformation) {
        struct SYSTEM_PROCESS_INFO { DWORD NextEntryOffset; DWORD NumberOfThreads; BYTE Reserved1[48]; UNICODE_STRING ImageName; DWORD BasePriority; HANDLE UniqueProcessId; DWORD Reserved2; };
        ULONG size = 0;
        NtQuerySystemInformation(5, NULL, 0, &size);
        std::vector<BYTE> buffer(size);
        if (NtQuerySystemInformation(5, buffer.data(), size, &size) == 0) {
            SYSTEM_PROCESS_INFO* info = (SYSTEM_PROCESS_INFO*)buffer.data();
            while (true) {
                DWORD pid = (DWORD)(ULONG_PTR)info->UniqueProcessId;
                if (pids.find(pid) == pids.end() && pid != 0 && pid != 4) {
                    RootkitIndicator indicator;
                    indicator.description = L"Обнаружен скрытый процесс";
                    indicator.severity = 5; indicator.detected = true;
                    wchar_t details[256]; wsprintfW(details, L"PID: %d", pid);
                    indicator.details = details;
                    result.push_back(indicator);
                }
                if (info->NextEntryOffset == 0) break;
                info = (SYSTEM_PROCESS_INFO*)((BYTE*)info + info->NextEntryOffset);
            }
        }
    }
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (scm) {
        DWORD bytesNeeded, servicesReturned;
        EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesReturned, NULL, NULL);
        std::vector<BYTE> buffer(bytesNeeded);
        if (EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL, buffer.data(), bytesNeeded, &bytesNeeded, &servicesReturned, NULL, NULL)) {
            auto services = (ENUM_SERVICE_STATUS_PROCESSW*)buffer.data();
            for (DWORD i = 0; i < servicesReturned; i++) {
                if (wcsstr(services[i].lpServiceName, L"rootkit") || wcsstr(services[i].lpServiceName, L"backdoor") ||
                    wcsstr(services[i].lpServiceName, L"hack") || wcsstr(services[i].lpServiceName, L"malware") ||
                    wcsstr(services[i].lpServiceName, L"trojan")) {
                    RootkitIndicator indicator;
                    indicator.description = L"Подозрительная служба";
                    indicator.severity = 4; indicator.detected = true;
                    indicator.details = services[i].lpServiceName;
                    result.push_back(indicator);
                }
            }
        }
        CloseServiceHandle(scm);
    }
    return result;
}

// ========== 12. КАРАНТИН (v2.0) ==========
bool QuarantineFile(const std::wstring& path, const std::wstring& reason) {
    wchar_t quarantineDir[MAX_PATH];
    GetWindowsDirectoryW(quarantineDir, MAX_PATH);
    wcscat_s(quarantineDir, L"\\Quarantine");
    CreateDirectoryW(quarantineDir, NULL);
    std::wstring fileName = PathFindFileNameW(path.c_str());
    std::wstring quarantinePath = std::wstring(quarantineDir) + L"\\" + fileName + L".quar";
    if (!CopyFileW(path.c_str(), quarantinePath.c_str(), FALSE)) return false;
    HANDLE hFile = CreateFileW(quarantinePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    BYTE key = 0xAA;
    BYTE buffer[4096];
    DWORD bytesRead, bytesWritten;
    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        for (DWORD i = 0; i < bytesRead; i++) buffer[i] ^= key;
        SetFilePointer(hFile, -bytesRead, NULL, FILE_CURRENT);
        WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL);
    }
    CloseHandle(hFile);
    QuarantineItem item;
    item.originalPath = path; item.quarantinePath = quarantinePath;
    GetSystemTimeAsFileTime(&item.timestamp); item.md5 = GetFileMD5(path);
    item.reason = reason;
    
    HANDLE hSizeFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hSizeFile != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER size;
        GetFileSizeEx(hSizeFile, &size);
        item.size = (DWORD)size.QuadPart;
        CloseHandle(hSizeFile);
    } else {
        item.size = 0;
    }
    
    g_quarantine.push_back(item);
    DeleteFileW(path.c_str());
    return true;
}

bool RestoreQuarantine(const std::wstring& quarantinePath) {
    HANDLE hFile = CreateFileW(quarantinePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    BYTE key = 0xAA;
    BYTE buffer[4096];
    DWORD bytesRead, bytesWritten;
    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        for (DWORD i = 0; i < bytesRead; i++) buffer[i] ^= key;
        SetFilePointer(hFile, -bytesRead, NULL, FILE_CURRENT);
        WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL);
    }
    CloseHandle(hFile);
    for (auto& item : g_quarantine) {
        if (item.quarantinePath == quarantinePath) {
            CopyFileW(quarantinePath.c_str(), item.originalPath.c_str(), FALSE);
            DeleteFileW(quarantinePath.c_str());
            return true;
        }
    }
    return false;
}

// ========== 13. СКАНИРОВАНИЕ ФАЙЛОВ (v2.0) ==========
std::vector<FileInfo> ScanDirectory(const std::wstring& directory) {
    std::vector<FileInfo> result;
    std::wstring searchPath = directory + L"\\*";
    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return result;
    do {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
        FileInfo info;
        info.name = fd.cFileName; info.path = directory + L"\\" + fd.cFileName;
        info.attributes = fd.dwFileAttributes;
        info.size = ((ULONGLONG)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
        info.created = fd.ftCreationTime; info.modified = fd.ftLastWriteTime; info.accessed = fd.ftLastAccessTime;
        info.isSystem = (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0;
        info.isHidden = (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
        info.isReadOnly = (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
        info.isPacked = false; info.isSigned = false; info.entropy = 0.0;
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            info.md5 = GetFileMD5(info.path); info.sha1 = GetFileSHA1(info.path);
            info.sha256 = GetFileSHA256(info.path); info.entropy = CalculateEntropy(info.path);
            info.isPacked = IsPacked(info.path); info.isSigned = IsFileSigned(info.path);
            info.version = GetFileVersion(info.path); info.company = GetFileCompany(info.path);
        }
        result.push_back(info);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            auto subResult = ScanDirectory(info.path);
            result.insert(result.end(), subResult.begin(), subResult.end());
        }
    } while (FindNextFileW(hFind, &fd));
    FindClose(hFind);
    return result;
}

// ========== 14. ЗАЩИТА ОТ ШИФРОВАЛЬЩИКОВ (v2.0) ==========
bool ProtectFolderFromRansomware(const std::wstring& folder) {
    g_protectedFolders.insert(folder);
    return true;
}

bool UnprotectFolder(const std::wstring& folder) {
    g_protectedFolders.erase(folder);
    return true;
}

// ========== 15. АНАЛИЗ СОБЫТИЙ (v2.0) ==========
std::vector<SecurityEvent> GetSecurityEvents() {
    std::vector<SecurityEvent> result;
    CoInitialize(NULL);
    IWbemServices* pSvc = NULL;
    IWbemLocator* pLoc = NULL;
    if (CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc) == S_OK) {
        if (pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc) == S_OK) {
            IEnumWbemClassObject* pEnumerator = NULL;
            if (pSvc->ExecQuery(_bstr_t(L"WQL"), _bstr_t(L"SELECT * FROM Win32_NTLogEvent WHERE LogFile='Security'"), WBEM_FLAG_FORWARD_ONLY, NULL, &pEnumerator) == S_OK) {
                IWbemClassObject* pclsObj = NULL;
                ULONG uReturn = 0;
                while (pEnumerator) {
                    pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                    if (uReturn == 0) break;
                    SecurityEvent event;
                    VARIANT vtProp;
                    if (pclsObj->Get(L"EventCode", 0, &vtProp, 0, 0) == S_OK) { event.eventId = vtProp.ulVal; VariantClear(&vtProp); }
                    if (pclsObj->Get(L"Message", 0, &vtProp, 0, 0) == S_OK) { event.description = vtProp.bstrVal; VariantClear(&vtProp); }
                    if (pclsObj->Get(L"User", 0, &vtProp, 0, 0) == S_OK) { event.user = vtProp.bstrVal; VariantClear(&vtProp); }
                    if (pclsObj->Get(L"TimeWritten", 0, &vtProp, 0, 0) == S_OK) { SystemTimeToFileTime((SYSTEMTIME*)&vtProp.ullVal, &event.timestamp); VariantClear(&vtProp); }
                    event.source = L"Security"; event.severity = 1;
                    if (event.eventId == 4624 || event.eventId == 4625 || event.eventId == 4672 || event.eventId == 4688 ||
                        event.eventId == 4698 || event.eventId == 4702) event.severity = 5;
                    result.push_back(event);
                    pclsObj->Release();
                }
                pEnumerator->Release();
            }
            pSvc->Release();
        }
        pLoc->Release();
    }
    CoUninitialize();
    return result;
}

// ========== 16. GUI (13 вкладок) ==========
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES };
        InitCommonControlsEx(&icex);
        g_hTabControl = CreateWindowW(WC_TABCONTROLW, NULL, WS_CHILD | WS_VISIBLE | TCS_FIXEDWIDTH, 0, 0, 850, 600, hWnd, NULL, g_hInst, NULL);
        TCITEMW tie = {};
        tie.mask = TCIF_TEXT;
        const wchar_t* tabs[] = {
            L"Процессы", L"Службы", L"Автозагрузка", L"Сеть",
            L"Восстановление", L"MBR Recovery", L"Самозащита",
            L"Мониторинг", L"Разблокировка", L"Защита",
            L"Руткиты", L"Карантин", L"События"
        };
        for (int i = 0; i < 13; i++) {
            tie.pszText = (wchar_t*)tabs[i];
            TabCtrl_InsertItem(g_hTabControl, i, &tie);
        }
        g_hStatusBar = CreateWindowW(STATUSCLASSNAMEW, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hWnd, NULL, g_hInst, NULL);
        SetStatusText(L"ARES-Defender v3.0 FULL готов");
        
        // Создаем ListView для каждой вкладки
        for (int i = 0; i < 13; i++) {
            g_hListViews[i] = CreateWindowW(WC_LISTVIEWW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS,
                5, 30, 835, 520, g_hTabControl, NULL, g_hInst, NULL);
            LVCOLUMNW col = {};
            col.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            col.cx = 150;
            if (i == 0) {
                const wchar_t* cols[] = {L"PID", L"Имя", L"Путь", L"Приоритет", L"Потоки", L"Память"};
                for (int j = 0; j < 6; j++) { col.pszText = (wchar_t*)cols[j]; col.cx = (j == 2) ? 250 : 100; ListView_InsertColumn(g_hListViews[i], j, &col); }
            } else if (i == 1) {
                const wchar_t* cols[] = {L"Имя", L"Отображение", L"Статус", L"Тип запуска", L"PID"};
                for (int j = 0; j < 5; j++) { col.pszText = (wchar_t*)cols[j]; col.cx = 150; ListView_InsertColumn(g_hListViews[i], j, &col); }
            } else if (i == 2) {
                const wchar_t* cols[] = {L"Имя", L"Путь", L"Расположение", L"Риск"};
                for (int j = 0; j < 4; j++) { col.pszText = (wchar_t*)cols[j]; col.cx = (j == 1) ? 300 : 150; ListView_InsertColumn(g_hListViews[i], j, &col); }
            } else if (i == 3) {
                const wchar_t* cols[] = {L"PID", L"Процесс", L"Локальный", L"Удаленный", L"Порт", L"Протокол"};
                for (int j = 0; j < 6; j++) { col.pszText = (wchar_t*)cols[j]; col.cx = 120; ListView_InsertColumn(g_hListViews[i], j, &col); }
            } else if (i == 5) {
                const wchar_t* cols[] = {L"Действие", L"Статус", L"Путь"};
                for (int j = 0; j < 3; j++) { col.pszText = (wchar_t*)cols[j]; col.cx = 250; ListView_InsertColumn(g_hListViews[i], j, &col); }
            } else {
                const wchar_t* cols[] = {L"Функция", L"Статус"};
                for (int j = 0; j < 2; j++) { col.pszText = (wchar_t*)cols[j]; col.cx = 400; ListView_InsertColumn(g_hListViews[i], j, &col); }
            }
        }
        break;
    }
    case WM_SIZE: {
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        if (g_hTabControl) {
            SetWindowPos(g_hTabControl, NULL, 0, 0, rcClient.right, rcClient.bottom - 20, SWP_NOZORDER);
            for (int i = 0; i < 13; i++) {
                if (g_hListViews[i]) SetWindowPos(g_hListViews[i], NULL, 5, 30, rcClient.right - 10, rcClient.bottom - 60, SWP_NOZORDER);
            }
        }
        if (g_hStatusBar) SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
        break;
    }
    case WM_NOTIFY: {
        if (((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
            int sel = TabCtrl_GetCurSel(g_hTabControl);
            for (int i = 0; i < 13; i++) ShowWindow(g_hListViews[i], (i == sel) ? SW_SHOW : SW_HIDE);
            switch (sel) {
            case 0: {
                ListView_DeleteAllItems(g_hListViews[0]);
                auto processes = EnumerateProcesses();
                for (const auto& p : processes) {
                    LVITEMW item = { LVIF_TEXT, ListView_GetItemCount(g_hListViews[0]) };
                    wchar_t buf[32];
                    wsprintfW(buf, L"%d", p.pid); item.pszText = buf; item.iSubItem = 0;
                    ListView_InsertItem(g_hListViews[0], &item);
                    ListView_SetItemText(g_hListViews[0], item.iItem, 1, (wchar_t*)p.name.c_str());
                    ListView_SetItemText(g_hListViews[0], item.iItem, 2, (wchar_t*)p.path.c_str());
                    const wchar_t* priorities[] = {L"Idle", L"Normal", L"High", L"Real-Time"};
                    int pri = (p.priority == IDLE_PRIORITY_CLASS) ? 0 : (p.priority == HIGH_PRIORITY_CLASS) ? 2 : (p.priority == REALTIME_PRIORITY_CLASS) ? 3 : 1;
                    ListView_SetItemText(g_hListViews[0], item.iItem, 3, (wchar_t*)priorities[pri]);
                    wsprintfW(buf, L"%d", p.threadCount); ListView_SetItemText(g_hListViews[0], item.iItem, 4, buf);
                    wsprintfW(buf, L"%.2f MB", p.memoryUsage / (1024.0 * 1024.0));
                    ListView_SetItemText(g_hListViews[0], item.iItem, 5, buf);
                }
                break;
            }
            case 1: {
                ListView_DeleteAllItems(g_hListViews[1]);
                auto services = EnumerateServices();
                for (const auto& s : services) {
                    LVITEMW item = { LVIF_TEXT, ListView_GetItemCount(g_hListViews[1]) };
                    item.pszText = (wchar_t*)s.name.c_str(); item.iSubItem = 0;
                    ListView_InsertItem(g_hListViews[1], &item);
                    ListView_SetItemText(g_hListViews[1], item.iItem, 1, (wchar_t*)s.displayName.c_str());
                    ListView_SetItemText(g_hListViews[1], item.iItem, 2, (wchar_t*)(s.isRunning ? L"Running" : L"Stopped"));
                    const wchar_t* startTypes[] = {L"Boot", L"System", L"Auto", L"Demand", L"Disabled"};
                    int st = (s.startType == SERVICE_BOOT_START) ? 0 : (s.startType == SERVICE_SYSTEM_START) ? 1 : (s.startType == SERVICE_AUTO_START) ? 2 : (s.startType == SERVICE_DEMAND_START) ? 3 : 4;
                    ListView_SetItemText(g_hListViews[1], item.iItem, 3, (wchar_t*)startTypes[st]);
                    wchar_t buf[32]; wsprintfW(buf, L"%d", s.pid);
                    ListView_SetItemText(g_hListViews[1], item.iItem, 4, buf);
                }
                break;
            }
            case 2: {
                ListView_DeleteAllItems(g_hListViews[2]);
                auto startups = EnumerateStartup();
                for (const auto& s : startups) {
                    LVITEMW item = { LVIF_TEXT, ListView_GetItemCount(g_hListViews[2]) };
                    item.pszText = (wchar_t*)s.name.c_str(); item.iSubItem = 0;
                    ListView_InsertItem(g_hListViews[2], &item);
                    ListView_SetItemText(g_hListViews[2], item.iItem, 1, (wchar_t*)s.path.c_str());
                    ListView_SetItemText(g_hListViews[2], item.iItem, 2, (wchar_t*)s.location.c_str());
                    wchar_t risk[32]; wsprintfW(risk, L"%.1f%%", s.riskScore * 100);
                    ListView_SetItemText(g_hListViews[2], item.iItem, 3, risk);
                }
                break;
            }
            case 3: {
                ListView_DeleteAllItems(g_hListViews[3]);
                auto connections = EnumerateNetworkConnections();
                for (const auto& c : connections) {
                    LVITEMW item = { LVIF_TEXT, ListView_GetItemCount(g_hListViews[3]) };
                    wchar_t buf[32]; wsprintfW(buf, L"%d", c.pid); item.pszText = buf; item.iSubItem = 0;
                    ListView_InsertItem(g_hListViews[3], &item);
                    ListView_SetItemText(g_hListViews[3], item.iItem, 1, (wchar_t*)c.processName.c_str());
                    wchar_t addr[64]; wsprintfW(addr, L"%s:%d", c.localAddr.c_str(), c.localPort);
                    ListView_SetItemText(g_hListViews[3], item.iItem, 2, addr);
                    wsprintfW(addr, L"%s:%d", c.remoteAddr.c_str(), c.remotePort);
                    ListView_SetItemText(g_hListViews[3], item.iItem, 3, addr);
                    wsprintfW(buf, L"%d", c.remotePort);
                    ListView_SetItemText(g_hListViews[3], item.iItem, 4, buf);
                    ListView_SetItemText(g_hListViews[3], item.iItem, 5, (wchar_t*)c.protocol.c_str());
                }
                break;
            }
            case 5: {
                ListView_DeleteAllItems(g_hListViews[5]);
                auto backups = GetMBRBackups();
                for (const auto& b : backups) {
                    LVITEMW item = { LVIF_TEXT, ListView_GetItemCount(g_hListViews[5]) };
                    item.pszText = L"Бэкап MBR"; item.iSubItem = 0;
                    ListView_InsertItem(g_hListViews[5], &item);
                    ListView_SetItemText(g_hListViews[5], item.iItem, 1, L"Сохранен");
                    ListView_SetItemText(g_hListViews[5], item.iItem, 2, (wchar_t*)b.path.c_str());
                }
                break;
            }
            case 6: {
                ListView_DeleteAllItems(g_hListViews[6]);
                const wchar_t* funcs[] = {L"Самозащита", L"Автозагрузка"};
                const wchar_t* statuses[] = {g_bSelfProtect ? L"Активна" : L"Отключена", L"Включена"};
                for (int i = 0; i < 2; i++) {
                    LVITEMW item = { LVIF_TEXT, i };
                    item.pszText = (wchar_t*)funcs[i]; item.iSubItem = 0;
                    ListView_InsertItem(g_hListViews[6], &item);
                    ListView_SetItemText(g_hListViews[6], item.iItem, 1, (wchar_t*)statuses[i]);
                }
                break;
            }
            case 7: {
                ListView_DeleteAllItems(g_hListViews[7]);
                const wchar_t* funcs[] = {L"Мониторинг процессов", L"Мониторинг сети", L"Мониторинг ресурсов"};
                const wchar_t* statuses[] = {
                    g_bMonitoring ? L"Активен" : L"Остановлен",
                    g_bMonitoring ? L"Активен" : L"Остановлен",
                    g_bMonitoring ? L"Активен" : L"Остановлен"
                };
                for (int i = 0; i < 3; i++) {
                    LVITEMW item = { LVIF_TEXT, i };
                    item.pszText = (wchar_t*)funcs[i]; item.iSubItem = 0;
                    ListView_InsertItem(g_hListViews[7], &item);
                    ListView_SetItemText(g_hListViews[7], item.iItem, 1, (wchar_t*)statuses[i]);
                }
                break;
            }
            case 10: {
                ListView_DeleteAllItems(g_hListViews[10]);
                auto rootkits = DetectRootkits();
                for (const auto& r : rootkits) {
                    LVITEMW item = { LVIF_TEXT, ListView_GetItemCount(g_hListViews[10]) };
                    item.pszText = (wchar_t*)r.description.c_str(); item.iSubItem = 0;
                    ListView_InsertItem(g_hListViews[10], &item);
                    wchar_t sev[32]; wsprintfW(sev, L"Уровень %d", r.severity);
                    ListView_SetItemText(g_hListViews[10], item.iItem, 1, sev);
                    ListView_SetItemText(g_hListViews[10], item.iItem, 2, (wchar_t*)r.details.c_str());
                }
                break;
            }
            case 11: {
                ListView_DeleteAllItems(g_hListViews[11]);
                for (const auto& q : g_quarantine) {
                    LVITEMW item = { LVIF_TEXT, ListView_GetItemCount(g_hListViews[11]) };
                    item.pszText = (wchar_t*)q.originalPath.c_str(); item.iSubItem = 0;
                    ListView_InsertItem(g_hListViews[11], &item);
                    ListView_SetItemText(g_hListViews[11], item.iItem, 1, (wchar_t*)q.reason.c_str());
                    ListView_SetItemText(g_hListViews[11], item.iItem, 2, (wchar_t*)TimeToString(q.timestamp).c_str());
                }
                break;
            }
            case 12: {
                ListView_DeleteAllItems(g_hListViews[12]);
                auto events = GetSecurityEvents();
                for (const auto& e : events) {
                    LVITEMW item = { LVIF_TEXT, ListView_GetItemCount(g_hListViews[12]) };
                    wchar_t buf[32]; wsprintfW(buf, L"%d", e.eventId);
                    item.pszText = buf; item.iSubItem = 0;
                    ListView_InsertItem(g_hListViews[12], &item);
                    ListView_SetItemText(g_hListViews[12], item.iItem, 1, (wchar_t*)e.source.c_str());
                    ListView_SetItemText(g_hListViews[12], item.iItem, 2, (wchar_t*)e.user.c_str());
                    ListView_SetItemText(g_hListViews[12], item.iItem, 3, (wchar_t*)e.description.c_str());
                }
                break;
            }
            }
        }
        break;
    }
    case WM_DESTROY:
        if (g_bMonitoring) StopSystemMonitoring();
        if (g_bSelfProtect) DisableSelfProtection();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

// ========== 17. ГЛАВНЫЙ ВХОД ==========
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInst = hInstance;
    if (!IsAdmin()) {
        MessageBoxW(NULL, L"Требуются права администратора!", L"ARES-Defender", MB_ICONERROR);
        return 1;
    }
    g_hMutex = CreateMutexW(NULL, TRUE, L"ARES_Defender_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(NULL, L"Программа уже запущена.", L"ARES-Defender", MB_OK);
        return 1;
    }
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"ARES_Defender_Class";
    RegisterClassExW(&wcex);
    g_hMainWnd = CreateWindowExW(WS_EX_APPWINDOW, L"ARES_Defender_Class",
        L"ARES-Defender v3.0 FULL — Все функции + MBR + Self-Protection",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 870, 670,
        NULL, NULL, hInstance, NULL);
    if (!g_hMainWnd) return 1;
    EnableSelfProtection();
    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    ReleaseMutex(g_hMutex);
    CloseHandle(g_hMutex);
    return (int)msg.wParam;
}