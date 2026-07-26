// raznos_xp_ultimate.cpp - УЛЬТРА-ЖЁСТКАЯ ВЕРСИЯ ДЛЯ WINDOWS XP
#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0501
#define WINVER 0x0501

#include <Windows.h>
#include <windowsx.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Msimg32.lib")

#define M_PI 3.14159265358979323846264338327950288

// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ====================

void DrawPixel(HDC hdc, int x, int y, COLORREF color) {
    SetPixel(hdc, x, y, color);
}

// ==================== НОВЫЕ ЭФФЕКТЫ ====================

// 1. SCREEN_SQUEEZE - экран схлопывается в точку
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
        // Добавляем инверсию
        for (int i = 0; i < 50; i++) {
            int x = rand() % w, y = rand() % h;
            BitBlt(hdc, x, y, 20 + rand() % 30, 20 + rand() % 30, memDC, x, y, NOTSRCCOPY);
        }
        phase += 0.02f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 2. PIXEL_MELT - пиксели плавятся
DWORD WINAPI pixel_melt(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    int meltY[1024] = {0};
    for (int i = 0; i < 1024; i++) meltY[i] = rand() % h;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int x = 0; x < w; x += 4) {
            int idx = x % 1024;
            meltY[idx] += 2 + rand() % 5;
            if (meltY[idx] > h) meltY[idx] = 0;
            BitBlt(hdc, x, meltY[idx], 4, h - meltY[idx], memDC, x, 0, SRCCOPY);
            BitBlt(hdc, x, 0, 4, meltY[idx], memDC, x, h - meltY[idx], SRCCOPY);
        }
        // Размазывание
        for (int y = 0; y < h; y += 2) {
            int shift = (int)(10 * sin(y * 0.05f + GetTickCount() * 0.005f));
            BitBlt(hdc, shift, y, w - abs(shift), 2, memDC, 0, y, SRCCOPY);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(8);
    }
    return 0;
}

// 3. RADIAL_BLUR - радиальное размытие
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
                float scale = 1.0f + 0.3f * sin(dist * 0.02f + angle);
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
        angle += 0.02f;
        DeleteDC(memDC);
        DeleteObject(bmp);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(8);
    }
    return 0;
}

// 4. SCREEN_WIPE - размазывание тряпкой
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
        // Размазывание
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

// 5. FLOATING_UI - плавающий интерфейс
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
        
        // Тряска
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

// 6. CUBIC_DISTORTION - кубическое искажение
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

// 7. WAVE_RIPPLE - волновые искажения
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
        angle += 0.04f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(6);
    }
    return 0;
}

// 8. SCREEN_TWIRL - закручивание экрана
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
        angle += 0.03f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(8);
    }
    return 0;
}

// 9. COLOR_EXPLOSION - взрыв цвета
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
        // Вспышки
        for (int i = 0; i < 20; i++) {
            int x = rand() % w, y = rand() % h;
            int size = 20 + rand() % 60;
            HPEN pen = CreatePen(PS_SOLID, 1, RGB(rand() % 256, rand() % 256, rand() % 256));
            HBRUSH brush = CreateSolidBrush(RGB(rand() % 256, rand() % 256, rand() % 256));
            SelectObject(hdc, pen);
            SelectObject(hdc, brush);
            Ellipse(hdc, x - size/2, y - size/2, x + size/2, y + size/2);
            DeleteObject(pen);
            DeleteObject(brush);
        }
        phase += 3;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 10. PIXEL_SORTING - сортировка пикселей
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

// 11. GHOST_TRAIL - призрачные следы
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
        for (int i = 0; i < 30; i++) {
            int x = rand() % w, y = rand() % h;
            BitBlt(hdc, x + rand() % 10 - 5, y + rand() % 10 - 5, 30, 30, memDC, x, y, SRCINVERT);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 12. SCREEN_FLIP - переворот экрана
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
        angle += 0.01f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 13. ZOOM_BURST - зум с разрывом
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
        for (int i = 0; i < 20; i++) {
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

// 14. GLASS_SHATTER - разбитое стекло
DWORD WINAPI glass_shatter(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    int pieces[100][4];
    for (int i = 0; i < 100; i++) {
        pieces[i][0] = rand() % w;
        pieces[i][1] = rand() % h;
        pieces[i][2] = 10 + rand() % 30;
        pieces[i][3] = 10 + rand() % 30;
    }
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int i = 0; i < 100; i++) {
            pieces[i][0] += rand() % 5 - 2;
            pieces[i][1] += rand() % 5 - 2;
            if (pieces[i][0] < 0) pieces[i][0] = w;
            if (pieces[i][0] > w) pieces[i][0] = 0;
            if (pieces[i][1] < 0) pieces[i][1] = h;
            if (pieces[i][1] > h) pieces[i][1] = 0;
            StretchBlt(hdc, pieces[i][0], pieces[i][1], pieces[i][2], pieces[i][3],
                       memDC, pieces[i][0], pieces[i][1], pieces[i][2], pieces[i][3], NOTSRCCOPY);
        }
        // Линии разбитого стекла
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        SelectObject(hdc, pen);
        for (int i = 0; i < 20; i++) {
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

// 15. NEON_GLOW - неоновое свечение
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
        // Неоновый эффект
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
        hue += 2;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 16. THERMAL_VISION - тепловизор
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
        // Шум тепловизора
        for (int i = 0; i < 500; i++) {
            SetPixel(hdc, rand() % w, rand() % h, RGB(rand() % 256, rand() % 256, rand() % 256));
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(15);
    }
    return 0;
}

// 17. GLITCH_WARP - глитч-искривление
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
        // Глитч-полосы
        for (int i = 0; i < 20; i++) {
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

// 18. SCREEN_CRUSH - сжатие экрана
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
        // Искажение
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

// 19. DIGITAL_RAIN - цифровой дождь
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
                    char c = "0123456789ABCDEF"[rand() % 16];
                    COLORREF color = RGB(0, 255 - j * 15, 0);
                    if (j == 0) color = RGB(255, 255, 255);
                    SetPixel(hdc, i * 8 + rand() % 6, y, color);
                }
            }
        }
        // Размытие
        BitBlt(hdc, 0, 0, w, h, hdc, rand() % 5 - 2, rand() % 5 - 2, SRCCOPY);
        ReleaseDC(0, hdc);
        Sleep(15);
    }
    return 0;
}

// ==================== ЗВУКИ (XP-совместимые) ====================

void PlayNoise(int freq, int duration) {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    int samples = 8000 * duration / 1000;
    char* buffer = new char[samples];
    for (int i = 0; i < samples; i++) {
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
    // Скрываем консоль
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    
    srand(GetTickCount());
    
    // ====== ВСЕ ЭФФЕКТЫ ПО 20 СЕКУНД ======
    // 19 жёстких эффектов × 20 сек = 380 сек = 6 минут 20 секунд

    HANDLE t0 = CreateThread(0, 0, screen_squeeze, 0, 0, 0); PlayNoise(200, 1000); Sleep(20000); TerminateThread(t0, 0); CloseHandle(t0);
    HANDLE t1 = CreateThread(0, 0, pixel_melt, 0, 0, 0); PlayNoise(300, 800); Sleep(20000); TerminateThread(t1, 0); CloseHandle(t1);
    HANDLE t2 = CreateThread(0, 0, radial_blur, 0, 0, 0); PlayNoise(150, 1200); Sleep(20000); TerminateThread(t2, 0); CloseHandle(t2);
    HANDLE t3 = CreateThread(0, 0, screen_wipe, 0, 0, 0); PlayNoise(400, 600); Sleep(20000); TerminateThread(t3, 0); CloseHandle(t3);
    HANDLE t4 = CreateThread(0, 0, floating_ui, 0, 0, 0); PlayNoise(250, 900); Sleep(20000); TerminateThread(t4, 0); CloseHandle(t4);
    HANDLE t5 = CreateThread(0, 0, cubic_distortion, 0, 0, 0); PlayNoise(350, 700); Sleep(20000); TerminateThread(t5, 0); CloseHandle(t5);
    HANDLE t6 = CreateThread(0, 0, wave_ripple, 0, 0, 0); PlayNoise(100, 1500); Sleep(20000); TerminateThread(t6, 0); CloseHandle(t6);
    HANDLE t7 = CreateThread(0, 0, screen_twirl, 0, 0, 0); PlayNoise(280, 850); Sleep(20000); TerminateThread(t7, 0); CloseHandle(t7);
    HANDLE t8 = CreateThread(0, 0, color_explosion, 0, 0, 0); PlayNoise(500, 500); Sleep(20000); TerminateThread(t8, 0); CloseHandle(t8);
    HANDLE t9 = CreateThread(0, 0, pixel_sorting, 0, 0, 0); PlayNoise(180, 1100); Sleep(20000); TerminateThread(t9, 0); CloseHandle(t9);
    HANDLE t10 = CreateThread(0, 0, ghost_trail, 0, 0, 0); PlayNoise(220, 950); Sleep(20000); TerminateThread(t10, 0); CloseHandle(t10);
    HANDLE t11 = CreateThread(0, 0, screen_flip, 0, 0, 0); PlayNoise(320, 750); Sleep(20000); TerminateThread(t11, 0); CloseHandle(t11);
    HANDLE t12 = CreateThread(0, 0, zoom_burst, 0, 0, 0); PlayNoise(450, 550); Sleep(20000); TerminateThread(t12, 0); CloseHandle(t12);
    HANDLE t13 = CreateThread(0, 0, glass_shatter, 0, 0, 0); PlayNoise(600, 400); Sleep(20000); TerminateThread(t13, 0); CloseHandle(t13);
    HANDLE t14 = CreateThread(0, 0, neon_glow, 0, 0, 0); PlayNoise(170, 1150); Sleep(20000); TerminateThread(t14, 0); CloseHandle(t14);
    HANDLE t15 = CreateThread(0, 0, thermal_vision, 0, 0, 0); PlayNoise(130, 1300); Sleep(20000); TerminateThread(t15, 0); CloseHandle(t15);
    HANDLE t16 = CreateThread(0, 0, glitch_warp, 0, 0, 0); PlayNoise(380, 650); Sleep(20000); TerminateThread(t16, 0); CloseHandle(t16);
    HANDLE t17 = CreateThread(0, 0, screen_crush, 0, 0, 0); PlayNoise(420, 580); Sleep(20000); TerminateThread(t17, 0); CloseHandle(t17);
    HANDLE t18 = CreateThread(0, 0, digital_rain, 0, 0, 0); PlayNoise(120, 1400); Sleep(20000); TerminateThread(t18, 0); CloseHandle(t18);

    // ====== ВЫХОД ======
    ExitProcess(0);
}