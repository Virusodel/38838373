// raznos_xp_clean.cpp - ЧИСТАЯ ВЕРСИЯ (ТОЛЬКО ЭФФЕКТЫ И ЗВУКИ, БЕЗ ДЕСТРУКТИВА)
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

// ==================== ОРИГИНАЛЬНЫЕ ЗВУКИ ИЗ ſ.cpp ====================

VOID WINAPI sound_crush() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 22050, 22050, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[22050 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t ^ t >> 4) * (t >> 3) & (t >> 6));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_glitch() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 16000, 16000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[16000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t & 0xFF) ^ (t >> 5) * (t & 0x1F));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_scan() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 12000, 12000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[12000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t * 3) ^ (t >> 4) | (t & 0x3F));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_shatter() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[8000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t * 7) ^ (t >> 3) * (t & 0x0F) | (t >> 7));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_pulse() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 11025, 11025, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[11025 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t * 5) ^ (t >> 6) | (t & 0x7F) * (t >> 4));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_blast() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 20000, 20000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[20000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t * 9) ^ (t >> 5) * (t & 0x3F) | (t >> 3));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_demolish() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[8000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t * 11) ^ (t >> 2) | (t & 0x1F) * (t >> 6));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_annihilate() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 22050, 22050, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[22050 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t * 13) ^ (t >> 7) | (t & 0x0F) * (t >> 5) | (t >> 4));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_spin() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 18000, 18000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[18000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t * 7) ^ (t >> 4) | (t & 0x7F) * (t >> 3));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_squeeze() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 14000, 14000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[14000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t * 3) ^ (t >> 6) | (t & 0x3F) * (t >> 5));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_waterfall() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 10000, 10000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[10000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t * 5) ^ (t >> 3) | (t & 0x1F) * (t >> 7));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_sideways() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 13000, 13000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[13000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t * 11) ^ (t >> 5) | (t & 0x0F) * (t >> 4));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_explode() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 24000, 24000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[24000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t * 17) ^ (t >> 7) | (t & 0x7F) * (t >> 6));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI sound_quake() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 9000, 9000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[9000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>((t * 13) ^ (t >> 2) | (t & 0x3F) * (t >> 8));
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

// ==================== ВСЕ ЭФФЕКТЫ (С ПРОВЕРКОЙ HDC) ====================

// 1. SCREEN_SQUEEZE
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
        if (!hdc) { Sleep(100); continue; }
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        float scale = 0.1f + 0.9f * fabs(sin(phase));
        int nw = (int)(w * scale);
        int nh = (int)(h * scale);
        if (nw < 1) nw = 1;
        if (nh < 1) nh = 1;
        StretchBlt(hdc, (w - nw) / 2, (h - nh) / 2, nw, nh, memDC, 0, 0, w, h, SRCCOPY);
        phase += 0.04f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 2. PIXEL_MELT
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
        if (!hdc) { Sleep(100); continue; }
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int x = 0; x < w; x += 2) {
            int idx = x % 512;
            meltY[idx] += 3 + rand() % 5;
            if (meltY[idx] > h) meltY[idx] = 0;
            BitBlt(hdc, x, meltY[idx], 2, h - meltY[idx], memDC, x, 0, SRCCOPY);
            BitBlt(hdc, x, 0, 2, meltY[idx], memDC, x, h - meltY[idx], SRCCOPY);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 3. RADIAL_BLUR
DWORD WINAPI radial_blur(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int cx = w / 2, cy = h / 2;
    float angle = 0;
    while (1) {
        hdc = GetDC(NULL);
        if (!hdc) { Sleep(100); continue; }
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
                int nx = cx + (int)(dx * cosA - dy * sinA);
                int ny = cy + (int)(dx * sinA + dy * cosA);
                if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                    COLORREF color = GetPixel(memDC, nx, ny);
                    SetPixel(hdc, x, y, color);
                }
            }
        }
        angle += 0.04f;
        DeleteDC(memDC);
        DeleteObject(bmp);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 4. SCREEN_WIPE
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
        if (!hdc) { Sleep(100); continue; }
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        offsetX += rand() % 7 - 3;
        offsetY += rand() % 7 - 3;
        if (offsetX > w / 4) offsetX = -w / 4;
        if (offsetX < -w / 4) offsetX = w / 4;
        if (offsetY > h / 4) offsetY = -h / 4;
        if (offsetY < -h / 4) offsetY = h / 4;
        StretchBlt(hdc, offsetX, offsetY, w, h, memDC, 0, 0, w, h, SRCCOPY);
        for (int y = 0; y < h; y += 2) {
            int shift = (int)(15 * sin(y * 0.1f + GetTickCount() * 0.01f));
            BitBlt(hdc, shift, y, w, 1, memDC, 0, y, SRCPAINT);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 5. FLOATING_UI
DWORD WINAPI floating_ui(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    float angle = 0;
    while (1) {
        HDC hdc = GetDC(0);
        if (!hdc) { Sleep(100); continue; }
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
        angle += 0.02f;
        DeleteDC(memDC);
        DeleteObject(bmp);
        ReleaseDC(0, hdc);
        DeleteDC(hdc);
        Sleep(15);
    }
    return 0;
}

// 6. CUBIC_DISTORTION
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
        if (!hdc) { Sleep(100); continue; }
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
        Sleep(10);
    }
    return 0;
}

// 7. WAVE_RIPPLE
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
        if (!hdc) { Sleep(100); continue; }
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < h; y += 1) {
            int shift = (int)(30 * sin(y * 0.05f + angle));
            BitBlt(hdc, shift, y, w - abs(shift), 1, memDC, 0, y, SRCCOPY);
        }
        angle += 0.08f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 8. SCREEN_TWIRL
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
        if (!hdc) { Sleep(100); continue; }
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
        Sleep(10);
    }
    return 0;
}

// 9. COLOR_EXPLOSION
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
        if (!hdc) { Sleep(100); continue; }
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
        phase += 5;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 10. PIXEL_SORTING
DWORD WINAPI pixel_sorting(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    while (1) {
        hdc = GetDC(NULL);
        if (!hdc) { Sleep(100); continue; }
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < h; y += 4) {
            int startX = rand() % (w / 2);
            int endX = startX + 50 + rand() % 100;
            for (int x = startX; x < endX && x < w; x++) {
                COLORREF color = GetPixel(memDC, x, y);
                SetPixel(hdc, x + rand() % 20 - 10, y + rand() % 20 - 10, color);
            }
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 11. GHOST_TRAIL
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
        if (!hdc) { Sleep(100); continue; }
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        AlphaBlend(hdc, rand() % 30 - 15, rand() % 30 - 15, w, h, memDC, 0, 0, w, h, blend);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(15);
    }
    return 0;
}

// 12. SCREEN_FLIP
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
        if (!hdc) { Sleep(100); continue; }
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
        Sleep(10);
    }
    return 0;
}

// 13. ZOOM_BURST
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
        if (!hdc) { Sleep(100); continue; }
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        zoom += speed;
        if (zoom > 3.0f || zoom < 0.3f) speed = -speed;
        int nw = (int)(w / zoom);
        int nh = (int)(h / zoom);
        if (nw < 1) nw = 1;
        if (nh < 1) nh = 1;
        StretchBlt(hdc, (w - nw)/2, (h - nh)/2, nw, nh, memDC, 0, 0, w, h, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 14. GLASS_SHATTER
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
        if (!hdc) { Sleep(100); continue; }
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
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(12);
    }
    return 0;
}

// 15. NEON_GLOW
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
        if (!hdc) { Sleep(100); continue; }
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < h; y += 3) {
            for (int x = 0; x < w; x += 3) {
                COLORREF color = GetPixel(memDC, x, y);
                int r = GetRValue(color) + 50 * sin(x * 0.01f + hue * 0.02f);
                int g = GetGValue(color) + 50 * cos(y * 0.01f + hue * 0.03f);
                int b = GetBValue(color) + 50 * sin((x + y) * 0.01f + hue * 0.04f);
                if (r > 255) r = 255; if (g > 255) g = 255; if (b > 255) b = 255;
                if (r < 0) r = 0; if (g < 0) g = 0; if (b < 0) b = 0;
                SetPixel(hdc, x, y, RGB(r, g, b));
            }
        }
        hue += 3;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(12);
    }
    return 0;
}

// 16. THERMAL_VISION
DWORD WINAPI thermal_vision(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    while (1) {
        hdc = GetDC(NULL);
        if (!hdc) { Sleep(100); continue; }
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                COLORREF color = GetPixel(memDC, x, y);
                int gray = (GetRValue(color) + GetGValue(color) + GetBValue(color)) / 3;
                int intensity = (gray * (x + y) / (w + h)) % 256;
                if (intensity > 200) SetPixel(hdc, x, y, RGB(255, 0, 0));
                else if (intensity > 120) SetPixel(hdc, x, y, RGB(255, 255, 0));
                else if (intensity > 60) SetPixel(hdc, x, y, RGB(0, 255, 0));
                else SetPixel(hdc, x, y, RGB(0, 0, 255));
            }
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(15);
    }
    return 0;
}

// 17. GLITCH_WARP
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
        if (!hdc) { Sleep(100); continue; }
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        glitchX += rand() % 11 - 5;
        glitchY += rand() % 11 - 5;
        if (glitchX > w / 3) glitchX = -w / 3;
        if (glitchX < -w / 3) glitchX = w / 3;
        if (glitchY > h / 3) glitchY = -h / 3;
        if (glitchY < -h / 3) glitchY = h / 3;
        StretchBlt(hdc, glitchX, glitchY, w, h, memDC, 0, 0, w, h, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 18. SCREEN_CRUSH
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
        if (!hdc) { Sleep(100); continue; }
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        crush += 0.01f * (rand() % 3 - 1);
        if (crush > 2.0f) crush = 0.3f;
        if (crush < 0.2f) crush = 2.0f;
        int nw = (int)(w / crush);
        int nh = (int)(h * crush);
        if (nw < 1) nw = 1;
        if (nh < 1) nh = 1;
        StretchBlt(hdc, (w - nw)/2, (h - nh)/2, nw, nh, memDC, 0, 0, w, h, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 19. DIGITAL_RAIN
DWORD WINAPI digital_rain(LPVOID lpvd) {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int columns = w / 8;
    int drops[256];
    for (int i = 0; i < 256; i++) drops[i] = rand() % h;
    while (1) {
        HDC hdc = GetDC(0);
        if (!hdc) { Sleep(100); continue; }
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
        ReleaseDC(0, hdc);
        Sleep(20);
    }
    return 0;
}

// 20. SPIN_CRUSH
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
        if (!hdc) { Sleep(100); continue; }
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
        angle += 0.5f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 21. FAST_WAVE
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
        if (!hdc) { Sleep(100); continue; }
        BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < h; y += 1) {
            int shift = (int)(60 * sin(y * 0.08f + angle * 2.0f));
            BitBlt(hdc, shift, y, w - abs(shift), 1, memDC, 0, y, SRCCOPY);
        }
        angle += 0.1f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
    return 0;
}

// 22. RADIAL_EXPLOSION
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
        if (!hdc) { Sleep(100); continue; }
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
        Sleep(10);
    }
    return 0;
}

// ==================== ТОЧКА ВХОДА (ЧИСТАЯ) ====================

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Скрываем консоль
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    
    srand(GetTickCount());
    
    // ====== ВСЕ ЭФФЕКТЫ ======
    // 19 старых + 3 новых = 22 эффекта × 20 сек = 440 сек = 7:20

    HANDLE t0 = CreateThread(0, 0, screen_squeeze, 0, 0, 0); sound_crush(); Sleep(20000); TerminateThread(t0, 0); CloseHandle(t0);
    HANDLE t1 = CreateThread(0, 0, pixel_melt, 0, 0, 0); sound_glitch(); Sleep(20000); TerminateThread(t1, 0); CloseHandle(t1);
    HANDLE t2 = CreateThread(0, 0, radial_blur, 0, 0, 0); sound_scan(); Sleep(20000); TerminateThread(t2, 0); CloseHandle(t2);
    HANDLE t3 = CreateThread(0, 0, screen_wipe, 0, 0, 0); sound_shatter(); Sleep(20000); TerminateThread(t3, 0); CloseHandle(t3);
    HANDLE t4 = CreateThread(0, 0, floating_ui, 0, 0, 0); sound_pulse(); Sleep(20000); TerminateThread(t4, 0); CloseHandle(t4);
    HANDLE t5 = CreateThread(0, 0, cubic_distortion, 0, 0, 0); sound_blast(); Sleep(20000); TerminateThread(t5, 0); CloseHandle(t5);
    HANDLE t6 = CreateThread(0, 0, wave_ripple, 0, 0, 0); sound_demolish(); Sleep(20000); TerminateThread(t6, 0); CloseHandle(t6);
    HANDLE t7 = CreateThread(0, 0, screen_twirl, 0, 0, 0); sound_annihilate(); Sleep(20000); TerminateThread(t7, 0); CloseHandle(t7);
    HANDLE t8 = CreateThread(0, 0, color_explosion, 0, 0, 0); sound_spin(); Sleep(20000); TerminateThread(t8, 0); CloseHandle(t8);
    HANDLE t9 = CreateThread(0, 0, pixel_sorting, 0, 0, 0); sound_squeeze(); Sleep(20000); TerminateThread(t9, 0); CloseHandle(t9);
    HANDLE t10 = CreateThread(0, 0, ghost_trail, 0, 0, 0); sound_waterfall(); Sleep(20000); TerminateThread(t10, 0); CloseHandle(t10);
    HANDLE t11 = CreateThread(0, 0, screen_flip, 0, 0, 0); sound_sideways(); Sleep(20000); TerminateThread(t11, 0); CloseHandle(t11);
    HANDLE t12 = CreateThread(0, 0, zoom_burst, 0, 0, 0); sound_explode(); Sleep(20000); TerminateThread(t12, 0); CloseHandle(t12);
    HANDLE t13 = CreateThread(0, 0, glass_shatter, 0, 0, 0); sound_quake(); Sleep(20000); TerminateThread(t13, 0); CloseHandle(t13);
    HANDLE t14 = CreateThread(0, 0, neon_glow, 0, 0, 0); sound_crush(); Sleep(20000); TerminateThread(t14, 0); CloseHandle(t14);
    HANDLE t15 = CreateThread(0, 0, thermal_vision, 0, 0, 0); sound_glitch(); Sleep(20000); TerminateThread(t15, 0); CloseHandle(t15);
    HANDLE t16 = CreateThread(0, 0, glitch_warp, 0, 0, 0); sound_scan(); Sleep(20000); TerminateThread(t16, 0); CloseHandle(t16);
    HANDLE t17 = CreateThread(0, 0, screen_crush, 0, 0, 0); sound_shatter(); Sleep(20000); TerminateThread(t17, 0); CloseHandle(t17);
    HANDLE t18 = CreateThread(0, 0, digital_rain, 0, 0, 0); sound_pulse(); Sleep(20000); TerminateThread(t18, 0); CloseHandle(t18);

    // НОВЫЕ ЭФФЕКТЫ (20-22)
    HANDLE t19 = CreateThread(0, 0, spin_crush, 0, 0, 0); sound_spin(); Sleep(20000); TerminateThread(t19, 0); CloseHandle(t19);
    HANDLE t20 = CreateThread(0, 0, fast_wave, 0, 0, 0); sound_waterfall(); Sleep(20000); TerminateThread(t20, 0); CloseHandle(t20);
    HANDLE t21 = CreateThread(0, 0, radial_explosion, 0, 0, 0); sound_explode(); Sleep(20000); TerminateThread(t21, 0); CloseHandle(t21);

    // ====== ЧИСТЫЙ ВЫХОД (БЕЗ BSOD) ======
    ExitProcess(0);
}