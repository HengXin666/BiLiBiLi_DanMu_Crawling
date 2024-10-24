import tkinter as tk
from tkinter import messagebox, font, simpledialog
import datetime

from .getCidWindow import VideoInfoApp
from .api.reqDataSingleton import ReqDataSingleton
from .credentialManager import CredentialManager
from . import tkcalendar


class VideoScraperUI:
    def __init__(self, master):
        self.master = master
        self.master.title("弹幕爬取工具 V1.0.0 By Heng_Xin")

        self.isGetAllDanmMaKu = tk.BooleanVar(value=ReqDataSingleton().isGetAllDanmMaKu)
        self.isGetToNowTime = tk.BooleanVar(value=ReqDataSingleton().isGetToNowTime)

        # 自定义字体
        self.custom_font = font.Font(family="黑体", size=14)
        
        # 视频CID输入
        self.cid_label = tk.Label(master, text=f"视频CID: {ReqDataSingleton().cid}", font=self.custom_font)
        self.cid_label.grid(row=0, column=0, padx=10, pady=10, sticky='w')

        # 获取CID
        # 创建按钮，点击按钮时打开子窗口，并传递回调函数
        self.get_cid_button = tk.Button(master, text="获取CID", command=self.getCidWindow, font=self.custom_font)
        self.get_cid_button.grid(row=0, column=1, padx=10, pady=10, sticky='ew')

        # 日期范围
        tk.Label(master, text="爬取范围:").grid(row=1, column=0, padx=10, pady=10, sticky='ew')
        self.dateObj = tkcalendar.Datepicker(master, (1, 1))  # 初始化类为对象
        self.dateObj.start_date.set(ReqDataSingleton().startDate)
        self.dateObj.end_date.set(ReqDataSingleton().endDate)

        # 保存到文件
        tk.Label(master, text="保存文件名:").grid(row=2, column=0, padx=10, pady=10, sticky='e')
        self.outFileBtn = tk.Button(master, text=ReqDataSingleton().outFile, command=self.setOutFile)
        self.outFileBtn.grid(row=2, column=1, padx=10, pady=10, sticky='ew')

        # 复选框
        self.binary_scrape_check = tk.Checkbutton(master, text="二分爬取全弹幕", variable=self.isGetAllDanmMaKu, font=self.custom_font, command=self.updateIsGetAllDanmMaKu)
        self.binary_scrape_check.grid(row=3, column=0, padx=10, pady=5, sticky='w')

        self.continue_scrape_check = tk.Checkbutton(master, text="始终爬取到当天", variable=self.isGetToNowTime, font=self.custom_font, command=self.updateIsGetToNowTime)
        self.continue_scrape_check.grid(row=3, column=1, padx=10, pady=5, sticky='w')

        # 控制按钮
        self.scrape_button = tk.Button(master, text="开始爬取", command=self.toggle_scrape, font=self.custom_font)
        self.scrape_button.grid(row=4, column=0, padx=10, pady=10, sticky='ew')

        self.continue_button = tk.Button(master, text="继续爬取", command=self.continue_scrape, font=self.custom_font)
        self.continue_button.grid(row=4, column=1, padx=10, pady=10, sticky='ew')

        # 日志框
        self.log_frame = tk.Frame(master)
        self.log_frame.grid(row=1, column=2, rowspan=6, padx=10, pady=10)
        
        self.log_text = tk.Text(self.log_frame, width=40, height=20, font=self.custom_font)
        self.log_text.pack(expand=True, fill='both')

        # 状态栏
        self.status_label = tk.Label(master, text="[状态栏]", font=self.custom_font)
        self.status_label.grid(row=0, column=2, padx=10, pady=5)

        # 摘要栏
        self.info_label = tk.Label(master, text="摘要:", font=self.custom_font)
        self.info_label.grid(row=5, column=0, padx=10, pady=5)

        # 工具栏
        self.menu_bar = tk.Menu(master)
        self.settings_menu = tk.Menu(self.menu_bar, tearoff=0)
        self.settings_menu.add_command(label="切换主题 (有Bug)", command=self.toggle_theme)
        self.settings_menu.add_command(label="自定义字体大小", command=self.set_font_size)
        self.menu_bar.add_cascade(label="设置", menu=self.settings_menu)
        self.menu_bar.add_command(label="配置凭证", command=self.setCookies)
        self.menu_bar.add_command(label="关于", command=self.show_about)

        master.config(menu=self.menu_bar)

        # 初始化为浅色模式
        self.is_dark_mode = False
        self.update_theme()

        self.is_scraping = False

        # 允许自适应布局
        master.grid_rowconfigure(0, weight=1)
        master.grid_columnconfigure(1, weight=1)

    def setCookies(self):
        child_window = tk.Toplevel()
        CredentialManager(child_window)
        child_window.mainloop()

    def setOutFile(self):
        # 弹出输入对话框
        while True:
            user_input = simpledialog.askstring("输入: [文件名.xml]", "请输入文件名(.xml):")
            if user_input is None:
                return
            if len(user_input) >= 4:
                ReqDataSingleton().outFile = self.outFileBtn['text'] = user_input
                return

    def updateIsGetAllDanmMaKu(self):
        if self.isGetAllDanmMaKu.get():
            ReqDataSingleton().isGetAllDanmMaKu = True
        else:
            ReqDataSingleton().isGetAllDanmMaKu = False

    def updateIsGetToNowTime(self):
        if self.isGetToNowTime.get():
            ReqDataSingleton().isGetToNowTime = True
        else:
            ReqDataSingleton().isGetToNowTime = False

    def setCid(self, cid: int):
        ReqDataSingleton().cid = cid
        self.cid_label['text'] = f"视频CID: {ReqDataSingleton().cid}"

    def getCidWindow(self):
        child_window = tk.Toplevel()
        VideoInfoApp(child_window, self.setCid)
        child_window.mainloop()

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
    
    def save(self):
        ReqDataSingleton().startDate = self.dateObj.start_date.get()
        ReqDataSingleton().endDate = self.dateObj.end_date.get()
        ReqDataSingleton().save() # 保存

def start() -> None:
    root = tk.Tk()
    app = VideoScraperUI(root)
    root.mainloop()
    app.save()

if __name__ == "__main__":
    start()
