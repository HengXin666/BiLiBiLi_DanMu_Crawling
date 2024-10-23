import tkinter as tk
import re
from tkinter import ttk, font
from api import videoApi

class VideoInfoApp:
    def __init__(self, root, callback):
        self.root = root
        self.root.title("从AV/BV获取Cid | 【注】: 双击[列表项]就是选择!")
        self.callback = callback

        custom_font = font.Font(family="黑体", size=14)

        # 创建输入框和按钮
        self.link_label = tk.Label(root, text="视频链接:", font=custom_font)
        self.link_label.grid(row=0, column=0, padx=5, pady=5)

        self.link_entry = tk.Entry(root, width=40)
        self.link_entry.grid(row=0, column=1, padx=5, pady=5)

        self.get_info_button = tk.Button(root, text="获取信息", command=self.getInfo, font=custom_font)
        self.get_info_button.grid(row=0, column=2, padx=5, pady=5)

        # 创建表格
        self.tree = ttk.Treeview(root, columns=("Column1", "Column2", "Column3"), show='headings')
        self.tree.heading("Column1", text="分P")
        self.tree.heading("Column2", text="cid")
        self.tree.heading("Column3", text="标题")
        self.tree.grid(row=1, column=0, columnspan=3, padx=5, pady=5, sticky="nsew")

        # 创建滚动条
        self.scrollbar = ttk.Scrollbar(root, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=self.scrollbar.set)
        self.scrollbar.grid(row=1, column=3, sticky='ns')

        # 调整布局
        root.grid_rowconfigure(1, weight=1)
        root.grid_columnconfigure(0, weight=1)
        root.grid_columnconfigure(1, weight=1)
        root.grid_columnconfigure(2, weight=1)

        # 用于存储信息
        self.data = []

    def extractBv(self) -> str | None:
        """
        从输入框, 提取AV或者BV号

        return: BV号
        
        失败则返回 None
        """

        # av to bv 转换算法
        # 来源: https://github.com/SocialSisterYi/bilibili-API-collect/issues/847#issuecomment-1807020675
        XOR_CODE = 23442827791579
        # MASK_CODE = 2251799813685247
        MAX_AID = 1 << 51
        ALPHABET = "FcwAPNKTMug3GV5Lj7EJnHpWsx4tb8haYeviqBz6rkCy12mUSDQX9RdoZf"
        ENCODE_MAP = 8, 7, 0, 5, 1, 3, 2, 4, 6
        # DECODE_MAP = tuple(reversed(ENCODE_MAP))

        BASE = len(ALPHABET)
        PREFIX = "BV1"
        # PREFIX_LEN = len(PREFIX)
        CODE_LEN = len(ENCODE_MAP)

        def av2bv(aid: int) -> str:
            bvid = [""] * 9
            tmp = (MAX_AID | aid) ^ XOR_CODE
            for i in range(CODE_LEN):
                bvid[ENCODE_MAP[i]] = ALPHABET[tmp % BASE]
                tmp //= BASE
            return PREFIX + "".join(bvid)

        # 获取输入框的文本
        text = self.link_entry.get()

        # 使用正则表达式提取 BV 号, 忽略大小写
        bv_match = re.search(r'BV([a-zA-Z0-9]+)/?|/BV([0-9a-zA-Z]+)/?', text, re.IGNORECASE)

        # 提取找到的 BV 号
        if bv_match:
            bv_number = bv_match.group(1) or bv_match.group(2)
            return f"BV{bv_number}"
        
        # 使用正则表达式提取 AV 号, 忽略大小写
        av_match = re.search(r'AV([0-9]+)/?|/AV([0-9]+)/?', text, re.IGNORECASE)

        # AV 号
        if av_match:
            av_number = av_match.group(1) or av_match.group(2)
            return av2bv(int(av_number))

        return None


    def getInfo(self):
        # 清除之前的内容
        for item in self.tree.get_children():
            self.tree.delete(item)

        try:
            bv = self.extractBv()
            if (bv != None):
                self.data = videoApi.获取视频信息(bv)
                if self.data[0] != 0:
                    print("出错啦:", self.data[0])
                self.data = self.data[1]
            else:
                print("无法获取av/bv号")
        except:
            print("网络出错啦~")
            return

        for it in self.data:
            self.tree.insert("", "end", values=(it['page'], it['cid'], it['part']))
            self.tree.bind("<Double-1>", lambda event: self.callback(event))

if __name__ == "__main__":
    root = tk.Tk()
    app = VideoInfoApp(root, lambda cid: (
        print("cid 为 ", cid)
    ))
    root.mainloop()
