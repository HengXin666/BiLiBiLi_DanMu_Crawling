import tkinter as tk
import re
from tkinter import ttk, font
from .api import videoApi

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

        # 提示文本
        self.info_label = tk.Label(root, text="[提示]: 双击[列表项]就是选择!", font=custom_font)
        self.info_label.grid(row=2, column=0, columnspan=3, padx=5, pady=5, sticky="nsew")

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
    
    def extractANiMe(self):
        """
        提前输入框, 获取md, ss, ep号

        return dict()

        其中 
            dict()['mdId']: str | None
            dict()['ssId']: str | None
            dict()['epId']: str | None
        """
        
        # 获取输入框的文本
        text = self.link_entry.get()

        mdMatch = re.search(r"/md(\d+)", text)
        ssMatch = re.search(r"/ss(\d+)", text)
        epMatch = re.search(r"/ep(\d+)", text)

        return {
            'mdId': mdMatch.group(1) if mdMatch else None,
            'ssId': ssMatch.group(1) if ssMatch else None,
            'epId': epMatch.group(1) if epMatch else None,
        }


    def getInfo(self):
        def _onDoubleClick(event):
            # 获取当前选中的行
            selected_item = self.tree.selection()
            if selected_item:
                item_values = self.tree.item(selected_item, 'values')
                cid = item_values[1]
                self.callback(int(cid))
        # 清除之前的内容
        for item in self.tree.get_children():
            self.tree.delete(item)

        try:
            # 测试 https://www.bilibili.com/video/BV1Wx411F7Hs/
            # 测试番剧 
            bv = self.extractBv()
            anime = self.extractANiMe()
            if (bv != None): # 解析普通视频
                self.data = videoApi.获取视频信息(bv)
                if self.data[0] != 0:
                    self.info_label['text'] = f"[错误]: BiliBili Api 异常: [{'请求错误' if self.data[0] == -400 else '无视频'}]; 对于 BV{bv}"
                    return
                self.data = self.data[1]
            elif (anime != None): # 解析番剧
                self.data = videoApi.获取番剧信息(mdId=anime['mdId'], ssId=anime['ssId'], epId=anime['epId'])
                if self.data[0] != 0:
                    self.info_label['text'] = f"[错误]: BiliBili Api 异常: [{'请求错误' if self.data[0] == -400 else '无视频'}]; 对于 {anime}"
                    return
                self.data = self.data[1]
            else:
                self.info_label['text'] = "[错误]: 无法获取av/bv号, 链接可能有问题, 请人工输入av/bv号"
                return
        except:
            self.info_label['text'] = "[错误]: 网络出错啦~"
            return
        self.info_label['text'] = "[提示]: 双击[列表项]就是选择!"

        for it in self.data:
            self.tree.insert("", "end", values=(it['page'], it['cid'], it['part']))

        self.tree.bind("<Double-1>", _onDoubleClick)

if __name__ == "__main__":
    root = tk.Tk()
    app = VideoInfoApp(root, lambda cid: (
        print("cid 为 ", cid)
    ))
    root.mainloop()
