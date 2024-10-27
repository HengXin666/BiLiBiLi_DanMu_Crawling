import os
import re
import time
import datetime
import tkinter as tk
from tkinter import filedialog, messagebox

def ShowCustomMessage(title, message):
    # 创建自定义对话框
    custom_message_box = tk.Toplevel()
    custom_message_box.title(title)
    custom_message_box.geometry("300x120")

    # 隐藏关闭按钮
    custom_message_box.protocol("WM_DELETE_WINDOW", lambda: None)

    # 设置字体样式
    label = tk.Label(custom_message_box, text=message, font=("黑体", 14))
    label.pack(padx=20, pady=20)

    # 确定按钮
    ok_button = tk.Button(custom_message_box, text="确定", command=custom_message_box.destroy, font=("黑体", 14))
    ok_button.pack(pady=10)

class FileProcessorApp:
    def __init__(self, master):
        self.master = master
        self.master.title("弹幕文件处理程序")

        # 允许自适应布局
        master.grid_rowconfigure(0, weight=1)
        master.grid_columnconfigure(0, weight=1)
        master.grid_columnconfigure(1, weight=1)

        # 选择文件按钮
        self.select_files_button = tk.Button(master, text="选择弹幕文件", command=self.select_files, font=("黑体", 14))
        self.select_files_button.grid(row=0, column=0, padx=10, pady=10)

        # 显示选择的文件
        self.file_listbox = tk.Listbox(master, selectmode=tk.MULTIPLE, width=50, height=10, font=("黑体", 14))
        self.file_listbox.grid(row=1, column=0, columnspan=2, padx=10, pady=10)

        # 选择输出路径按钮
        self.select_output_button = tk.Button(master, text="选择输出路径", command=self.select_output_path, font=("黑体", 14))
        self.select_output_button.grid(row=2, column=0, padx=10, pady=10)

        # 显示输出路径
        self.output_path_label = tk.Label(master, text="输出路径: 未选择", font=("黑体", 14))
        self.output_path_label.grid(row=2, column=1, padx=10, pady=10)

        # 按钮容器
        button_frame = tk.Frame(master)
        button_frame.grid(row=3, column=0, columnspan=2, pady=10)

        # 文件去重按钮
        self.deduplicate_button = tk.Button(button_frame, text="弹幕去重或合并", command=self.deduplicate_files, font=("黑体", 14))
        self.deduplicate_button.grid(row=0, column=0, padx=5)

        # 初始化文件和输出路径
        self.selected_files = []
        self.output_path = ""

    def select_files(self):
        files = filedialog.askopenfilenames(title="选择弹幕文件")
        if files:
            self.selected_files = list(files)
            self.file_listbox.delete(0, tk.END)
            for file in self.selected_files:
                self.file_listbox.insert(tk.END, file)
            self.update_output_path()

    def select_output_path(self):
        path = filedialog.askdirectory(title="选择输出路径")
        if path:
            self.output_path = path
            self.output_path_label.config(text=f"输出路径: {self.output_path}")

    def update_output_path(self):
        if self.selected_files:
            self.output_path = os.path.dirname(self.selected_files[-1])
            self.output_path_label.config(text=f"输出路径: {self.output_path}")

    def deduplicate_files(self):
        if not self.selected_files:
            ShowCustomMessage("警告", "请先选择文件")
            return
        if not self.output_path:
            ShowCustomMessage("警告", "请先选择输出路径")
            return
        
        print("提示: 去重或者合并需要花费一点时间! 该区间窗口将会[未响应]! 请耐心等待!")
        
        cnt = set()
        cntAllNum = 0
        resList = []
        dmCid = None
        reCid = re.compile(r"<chatid>(\d+)</chatid>")  # 抓取cid
        reAll = re.compile(r'(<d p="\d+\.\d+,\d,\d+,\d+,\d+,\d,.+?,\d+">.+?</d>)')
        reDmMid = re.compile('<d p="\\d+\\.\\d+,\\d,\\d+,\\d+,\\d+,\\d,[A-Za-z0-9]+,(\\d+)">.*?</d>') # 弹幕id

        for filePath in self.selected_files:  # 读取每一个文件
            fileDataList = self._read_xml(filePath)
            print("正在处理:", filePath)
            for line in fileDataList:
                # 读取每一行
                if dmCid == None:
                    tmpCidReRet = re.findall(reCid, line)
                    if len(tmpCidReRet) == 1:
                        dmCid = tmpCidReRet[0]
                        print("获取到cid:", dmCid)
                dmList = re.findall(reAll, line)
                cntAllNum += len(dmList)
                for dm in dmList:
                    # 读取每一条弹幕
                    dmMid = re.findall(reDmMid, dm)[0]
                    dmMid = int(dmMid)
                    if dmMid not in cnt:
                        # 去重结果
                        cnt.add(dmMid)
                        resList.append(dm)
            print(filePath, "处理完成!")

        with open(os.path.join(self.output_path, f"HX-{datetime.datetime.now().strftime("%Y-%m-%d_%H%M%S")}.xml"), "w", encoding="utf-8") as f:
            f.writelines(f'<?xml version="1.0" encoding="UTF-8"?><i><chatserver>chat.bilibili.com</chatserver><chatid>{dmCid}</chatid><mission>0</mission><maxlimit>3000</maxlimit><state>0</state><real_name>0</real_name><source>k-v</source>\n')
            for i in resList:
                f.writelines(i + "\n")
            f.writelines("</i>")

        ShowCustomMessage("去重结果", f"去重后弹幕数量: {cntAllNum} -> {len(cnt)}")
        print(f"去重后弹幕数量: {cntAllNum} -> {len(cnt)}")

    def _read_xml(self, filePath: str):
        with open(filePath, "r", encoding="UTF-8") as f:
            return f.readlines() # 获取文件每一行内容,保存为列表


if __name__ == "__main__":
    root = tk.Tk()
    app = FileProcessorApp(root)
    root.mainloop()
