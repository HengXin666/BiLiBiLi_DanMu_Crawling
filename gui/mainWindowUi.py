import tkinter as tk
from tkinter import messagebox
import datetime

from api import *

class VideoScraperUI:
    def __init__(self, master):
        self.master = master
        self.master.title("视频爬取工具")

        # 视频CID输入
        self.cid_label = tk.Label(master, text="视频CID:")
        self.cid_label.grid(row=0, column=0, padx=10, pady=10)
        self.cid_entry = tk.Entry(master)
        self.cid_entry.grid(row=0, column=1, padx=10, pady=10)

        # 起始日期
        self.start_date_label = tk.Label(master, text="起始日期 (YYYY-MM-DD):")
        self.start_date_label.grid(row=1, column=0, padx=10, pady=10)
        self.start_date_entry = tk.Entry(master)
        self.start_date_entry.grid(row=1, column=1, padx=10, pady=10)

        # 结束日期
        self.end_date_label = tk.Label(master, text="结束日期 (YYYY-MM-DD):")
        self.end_date_label.grid(row=2, column=0, padx=10, pady=10)
        self.end_date_entry = tk.Entry(master)
        self.end_date_entry.grid(row=2, column=1, padx=10, pady=10)

        # 复选框
        self.binary_scrape_var = tk.IntVar()
        self.binary_scrape_check = tk.Checkbutton(master, text="二分爬取全弹幕", variable=self.binary_scrape_var)
        self.binary_scrape_check.grid(row=3, column=0, padx=10, pady=5)

        self.continue_scrape_var = tk.IntVar()
        self.continue_scrape_check = tk.Checkbutton(master, text="始终爬取到当天", variable=self.continue_scrape_var)
        self.continue_scrape_check.grid(row=3, column=1, padx=10, pady=5)

        # 控制按钮
        self.scrape_button = tk.Button(master, text="开始爬取", command=self.toggle_scrape)
        self.scrape_button.grid(row=5, column=0, padx=10, pady=10)

        self.continue_button = tk.Button(master, text="继续爬取", command=self.continue_scrape)
        self.continue_button.grid(row=5, column=1, padx=10, pady=10)

        # 日志框
        self.log_frame = tk.Frame(master)
        self.log_frame.configure(background="red")
        self.log_frame.grid(row=0, column=2, rowspan=6, padx=10, pady=10)

        self.log_text = tk.Text(self.log_frame, width=40, height=20)
        self.log_text.pack()

        # 状态栏
        self.status_label = tk.Label(master, text="[状态栏]")
        self.status_label.grid(row=6, column=2, padx=10, pady=5)

        # 工具栏
        self.menu_bar = tk.Menu(master)
        self.settings_menu = tk.Menu(self.menu_bar, tearoff=0)
        self.settings_menu.add_command(label="切换主题", command=self.toggle_theme)
        self.menu_bar.add_cascade(label="设置", menu=self.settings_menu)
        self.menu_bar.add_command(label="关于", command=self.show_about)

        master.config(menu=self.menu_bar)

        # 初始化为浅色模式
        self.is_dark_mode = False
        self.update_theme()

        self.is_scraping = False

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
        bg_color = "black" if self.is_dark_mode else "white"
        fg_color = "white" if self.is_dark_mode else "black"
        self.master.configure(bg=bg_color)
        for child in self.master.winfo_children():
            if isinstance(child, tk.Label):
                child.config(bg=bg_color, fg=fg_color) 
        self.log_text.configure(bg="#282828" if self.is_dark_mode else "white", fg=fg_color)


if __name__ == "__main__":
    root = tk.Tk()
    app = VideoScraperUI(root)
    root.mainloop()