import tkinter as tk
from tkinter import messagebox, font, simpledialog
import datetime

# from api import *

def open_child_window(callback):
    child_window = tk.Toplevel()
    child_window.title("从AV/BV获取Cid")

    # 在子窗口中添加一个按钮
    btn_child = tk.Button(child_window, text="Child Button", command=lambda: callback(
        "开始爬取...", "green"
    ))
    btn_child.pack()

class VideoScraperUI:
    def __init__(self, master):
        self.master = master
        self.master.title("弹幕爬取工具 V1.0.0 By Heng_Xin")

        # 自定义字体
        self.custom_font = font.Font(family="黑体", size=14)
        
        # 视频CID输入
        self.cid_label = tk.Label(master, text="视频CID: ", font=self.custom_font)
        self.cid_label.grid(row=0, column=0, padx=10, pady=10, sticky='w')

        # 获取CID
        # 创建按钮，点击按钮时打开子窗口，并传递回调函数
        self.get_cid_button = tk.Button(master, text="获取CID", command=lambda: open_child_window(self.add_log), font=self.custom_font)
        self.get_cid_button.grid(row=0, column=1, padx=10, pady=10, sticky='ew')

        # 起始日期
        self.start_date_label = tk.Label(master, text="起始日期 (YYYY-MM-DD):", font=self.custom_font)
        self.start_date_label.grid(row=1, column=0, padx=10, pady=10, sticky='w')
        self.start_date_entry = tk.Entry(master, font=self.custom_font)
        self.start_date_entry.grid(row=1, column=1, padx=10, pady=10, sticky='ew')

        # 结束日期
        self.end_date_label = tk.Label(master, text="结束日期 (YYYY-MM-DD):", font=self.custom_font)
        self.end_date_label.grid(row=2, column=0, padx=10, pady=10, sticky='w')
        self.end_date_entry = tk.Entry(master, font=self.custom_font)
        self.end_date_entry.grid(row=2, column=1, padx=10, pady=10, sticky='ew')

        # 复选框
        self.binary_scrape_var = tk.IntVar()
        self.binary_scrape_check = tk.Checkbutton(master, text="二分爬取全弹幕", variable=self.binary_scrape_var, font=self.custom_font)
        self.binary_scrape_check.grid(row=3, column=0, padx=10, pady=5, sticky='w')

        self.continue_scrape_var = tk.IntVar()
        self.continue_scrape_check = tk.Checkbutton(master, text="始终爬取到当天", variable=self.continue_scrape_var, font=self.custom_font)
        self.continue_scrape_check.grid(row=3, column=1, padx=10, pady=5, sticky='w')

        # 控制按钮
        self.scrape_button = tk.Button(master, text="开始爬取", command=self.toggle_scrape, font=self.custom_font)
        self.scrape_button.grid(row=4, column=0, padx=10, pady=10, sticky='ew')

        self.continue_button = tk.Button(master, text="继续爬取", command=self.continue_scrape, font=self.custom_font)
        self.continue_button.grid(row=4, column=1, padx=10, pady=10, sticky='ew')

        # 日志框
        self.log_frame = tk.Frame(master)
        self.log_frame.grid(row=0, column=2, rowspan=6, padx=10, pady=10)
        
        self.log_text = tk.Text(self.log_frame, width=40, height=20, font=self.custom_font)
        self.log_text.pack(expand=True, fill='both')

        # 状态栏
        self.status_label = tk.Label(master, text="[状态栏]", font=self.custom_font)
        self.status_label.grid(row=5, column=2, padx=10, pady=5)

        # 工具栏
        self.menu_bar = tk.Menu(master)
        self.settings_menu = tk.Menu(self.menu_bar, tearoff=0)
        self.settings_menu.add_command(label="切换主题", command=self.toggle_theme)
        self.settings_menu.add_command(label="自定义字体大小", command=self.set_font_size)
        self.menu_bar.add_cascade(label="设置", menu=self.settings_menu)
        self.menu_bar.add_command(label="关于", command=self.show_about)

        master.config(menu=self.menu_bar)

        # 初始化为浅色模式
        self.is_dark_mode = False
        self.update_theme()

        self.is_scraping = False

        # 允许自适应布局
        master.grid_rowconfigure(0, weight=1)
        master.grid_columnconfigure(1, weight=1)

    def toggle_scrape(self):
        if not self.is_scraping:
            self.is_scraping = True
            self.scrape_button.config(text="终止爬取")
            self.add_log("开始爬取...", "green")
        else:
            if messagebox.askyesno("确认", "确定要终止爬取吗？"):
                self.is_scraping = False
                self.scrape_button.config(text="开始爬取")
                self.add_log("终止爬取", "red")

    def continue_scrape(self):
        self.add_log("继续爬取功能尚未实现", "yellow")

    def add_log(self, message: str, color: str):
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_message = f"[{timestamp}]: {message}\n"
        self.log_text.insert(tk.END, log_message, color)

        # 改变文本颜色
        self.log_text.tag_configure(color, foreground=color)
        self.log_text.tag_add(color, "end-1c linestart", "end")

    def show_about(self):
        messagebox.showinfo("关于", "视频爬取工具 v1.0")

    def toggle_theme(self):
        self.is_dark_mode = not self.is_dark_mode
        self.update_theme()

    def update_theme(self):
        bg_color = "#171717" if self.is_dark_mode else "white"
        fg_color = "white" if self.is_dark_mode else "#171717"
        self.master.configure(bg=bg_color)
        for child in self.master.winfo_children():
            if isinstance(child, (tk.Label, tk.Button)):
                child.config(bg=bg_color, fg=fg_color, font=self.custom_font)
        # 更新复选框的选中状态颜色
        # self.binary_scrape_check.config(bg=bg_color, fg=fg_color)
        # self.continue_scrape_check.config(bg=bg_color, fg=fg_color)
        self.log_text.configure(bg="#282828")

    def set_font_size(self):
        size = simpledialog.askinteger("字体大小", "输入字体大小:", minvalue=8, maxvalue=30)
        if size:
            self.custom_font.configure(size=size)
            self.update_theme()

def start() -> None:
    root = tk.Tk()
    app = VideoScraperUI(root)
    root.mainloop()

if __name__ == "__main__":
    start()
