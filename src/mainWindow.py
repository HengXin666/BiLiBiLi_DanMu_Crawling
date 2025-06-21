import datetime
import queue
import threading
import time
import random
import re
import tkinter as tk
import webbrowser
from tkinter import messagebox, font, simpledialog

from .getCidWindow import VideoInfoApp
from .fileProcessorWindow import FileProcessorApp 
from .api.reqDataSingleton import ReqDataSingleton
from .api.danMaKuApi import getHistoricalDanMaKu, getBasDanMaKu
from .credentialManager import CredentialManager
from .utils.yearDaysUtils import YearFamily
from .utils.danMaKuXmlUtils import DanMaKuXmlUtils
from .info import version
from . import tkcalendar

class VideoScraperUI:
    def __init__(self, master):
        self.master = master
        self.master.title(f"弹幕爬取工具 V{version.HX_BILIBILI_DANMU_CRAWLING_VERSION} By Heng_Xin")

        self.isGetAllDanMaKu = tk.BooleanVar(value=ReqDataSingleton().isGetAllDanMaKu)
        self.isGetToNowTime = tk.BooleanVar(value=ReqDataSingleton().isGetToNowTime)
        self.isGetOptimize = tk.BooleanVar(value=ReqDataSingleton().isGetOptimize)

        # 自定义字体
        self.custom_font = font.Font(family="黑体", size=14)

        # 视频CID输入
        self.cid_label = tk.Label(master, text=f"视频CID: {ReqDataSingleton().cid}", font=self.custom_font)
        self.cid_label.grid(row=0, column=0, padx=10, pady=10, sticky='w')

        # 获取CID
        self.get_cid_button = tk.Button(master, text="获取CID", command=self.getCidWindow, font=self.custom_font)
        self.get_cid_button.grid(row=0, column=1, padx=10, pady=10, sticky='ew')

        # 日期范围
        tk.Label(master, text="爬取范围:").grid(row=1, column=0, padx=10, pady=10, sticky='w')
        self.dateObj = tkcalendar.Datepicker(master, (1, 1))
        self.dateObj.start_date.set(ReqDataSingleton().startDate)
        self.dateObj.end_date.set(ReqDataSingleton().endDate)

        # 保存到文件
        tk.Label(master, text="保存文件名:").grid(row=2, column=0, padx=10, pady=10, sticky='e')
        self.outFileBtn = tk.Button(master, text=ReqDataSingleton().outFile, command=self.setOutFile)
        self.outFileBtn.grid(row=2, column=1, padx=10, pady=10, sticky='ew')

        # 复选框
        self.binary_scrape_check = tk.Checkbutton(master, text="二分爬取全弹幕", variable=self.isGetAllDanMaKu, font=self.custom_font, command=self.updateIsGetAllDanMaKu)
        self.binary_scrape_check.grid(row=3, column=0, padx=10, pady=5, sticky='w')

        self.continue_scrape_check = tk.Checkbutton(master, text="始终爬取到当天", variable=self.isGetToNowTime, font=self.custom_font, command=self.updateIsGetToNowTime)
        self.continue_scrape_check.grid(row=3, column=1, padx=10, pady=5, sticky='w')

        self.optimize_scrape_check = tk.Checkbutton(master, text="优化爬取速度", variable=self.isGetOptimize, font=self.custom_font, command=self.updateIsGetOptimize)
        self.optimize_scrape_check.grid(row=4, column=0, padx=10, pady=5, sticky='w')

        # 控制按钮
        self.scrape_button = tk.Button(master, text="开始爬取", command=self.toggleScrape, font=self.custom_font)
        self.scrape_button.grid(row=5, column=0, padx=10, pady=10, sticky='ew')

        self.continue_button = tk.Button(master, text="继续爬取", command=self.continueScrape, font=self.custom_font)
        self.continue_button.grid(row=5, column=1, padx=10, pady=10, sticky='ew')

        # 日志框
        self.log_frame = tk.Frame(master)
        self.log_frame.grid(row=1, column=2, rowspan=5, padx=10, pady=10, sticky='nsew')

        # 创建滚动条
        self.scrollbar = tk.Scrollbar(self.log_frame)
        self.scrollbar.pack(side='right', fill='y')

        # 创建文本框并关联滚动条
        self.log_text = tk.Text(self.log_frame, width=40, height=20, font=self.custom_font, yscrollcommand=self.scrollbar.set)
        self.log_text.pack(expand=True, fill='both')

        # 配置滚动条
        self.scrollbar.config(command=self.log_text.yview)

        # 状态栏
        self.status_label = tk.Label(master, text="[状态栏]", font=self.custom_font)
        self.status_label.grid(row=0, column=2, padx=10, pady=10, sticky='nsew')

        # 摘要栏
        # 懒得写了...
        # self.info_label = tk.Label(master, text="摘要:", font=self.custom_font)
        # self.info_label.grid(row=5, column=0, padx=10, pady=5, sticky='w')

        # 配置行和列权重, 以便日志框可以扩展
        master.grid_rowconfigure(1, weight=1)
        master.grid_columnconfigure(2, weight=1)

        # 工具栏
        self.menu_bar = tk.Menu(master)
        self.settings_menu = tk.Menu(self.menu_bar, tearoff=0)
        self.settings_menu.add_command(label="切换主题 (有Bug)", command=self.toggleTheme)
        self.settings_menu.add_command(label="自定义字体大小", command=self.setFontSize)
        self.settings_menu.add_command(label="设置爬取间隔", command=self.openTimerSettings)
        self.settings_menu.add_command(label="关闭日志滚动", command=self.setLogGoToEnd)
        self.menu_bar.add_cascade(label="设置", menu=self.settings_menu)
        self.menu_bar.add_command(label="配置凭证", command=self.setCookies)
        self.menu_bar.add_command(label="弹幕文件操作", command=self.fileProcessor)
        self.menu_bar.add_command(label="关于", command=self.showAbout)
        master.config(menu=self.menu_bar)

        # 初始化为浅色模式
        self.is_dark_mode = False
        self.updateTheme()

        # 允许自适应布局
        master.grid_rowconfigure(0, weight=1)
        master.grid_columnconfigure(1, weight=1)

        self.isLogGoToEnd = True

        ########################################################
        self.running = False
        self.queue = queue.Queue() # 线程安全
        self.dmIdCnt = set() # 弹幕id哈希, 防止重复
        self.allDmCnt = 0    # 当前已经爬取的弹幕数量
        self.seniorDmCnt = 0 # 当前高级弹幕数量
        self.thread = None   # 初始化线程
        self.isThreadExit = False  # 是否要求子线程退出
        self.isWifiNotGood = False # 是否处于网络错误状态

        self.updateReq() # 启动一个事件循环

    def fileProcessor(self):
        child_window = tk.Toplevel()
        FileProcessorApp(child_window)
        child_window.mainloop()

    def setLogGoToEnd(self):
        if self.isLogGoToEnd:
            self.settings_menu.entryconfig("关闭日志滚动", label="启用日志滚动")
        else:
            self.settings_menu.entryconfig("启用日志滚动", label="关闭日志滚动")
        self.isLogGoToEnd = not self.isLogGoToEnd

    def setCookies(self):
        child_window = tk.Toplevel()
        CredentialManager(child_window)
        child_window.mainloop()

    def setOutFile(self):
        # 弹出输入对话框
        while not self.running:
            user_input = simpledialog.askstring("输入: 弹幕文件名", "请输入文件名(示例: file -> 输出: file.xml):")
            if len(user_input) <= 0:
                continue
            ReqDataSingleton().outFile = self.outFileBtn['text'] = f"{user_input}.xml"
            return

    def updateIsGetAllDanMaKu(self):
        if self.isGetAllDanMaKu.get():
            ReqDataSingleton().isGetAllDanMaKu = True
        else:
            ReqDataSingleton().isGetAllDanMaKu = False

    def updateIsGetToNowTime(self):
        if self.isGetToNowTime.get():
            ReqDataSingleton().isGetToNowTime = True
        else:
            ReqDataSingleton().isGetToNowTime = False

    def updateIsGetOptimize(self):
        if self.isGetOptimize.get():
            ReqDataSingleton().isGetOptimize = True
        else:
            ReqDataSingleton().isGetOptimize = False

    def setCid(self, cid: int):
        ReqDataSingleton().cid = cid
        self.cid_label['text'] = f"视频CID: {ReqDataSingleton().cid}"

    def getCidWindow(self):
        """
        打开获取视频Cid窗口
        """
        child_window = tk.Toplevel()
        VideoInfoApp(child_window, self.setCid)
        child_window.mainloop()

    def updateButtonTextByRunning(self):
        """
        通过`running`更新按钮文本
        """
        if self.isWifiNotGood:
            self.isWifiNotGood = not self.isWifiNotGood
            if ReqDataSingleton().yearList.nowAllIndex != -1:
                ReqDataSingleton().yearList.unNext()

        if self.running:
            self.scrape_button.config(text="终止爬取")
            self.continue_button.config(text='暂停爬取')
        else:
            self.isThreadExit = True # 终止子线程
            self.scrape_button.config(text="开始爬取")
            self.continue_button.config(text='继续爬取')

    def _joinThread(self):
        """
        停止子线程, 并且回收
        """
        self.thread.join()
        self.thread = None

    def _startThread(self):
        """
        开启子线程
        """
        while self.thread != None:
            self.isThreadExit = True
            self._joinThread()
        self.isThreadExit = False
        self.thread = threading.Thread(target=self.runReq)
        self.thread.start()

    def toggleScrape(self):
        """
        开始爬取
        """
        if not ReqDataSingleton().isGetAllDanMaKu:
            ReqDataSingleton().startDate = self.dateObj.start_date.get()
            ReqDataSingleton().endDate = self.dateObj.end_date.get()
        if not self.running: # 开始爬取
            if len(ReqDataSingleton().cookies) == 0:
                self.addLog("请添加凭证再开始爬取!", "red")
                return
            if ReqDataSingleton().yearList != None and not messagebox.askyesno("注意: 真的要重新开始爬取吗?", "点击[开始爬取]是重新爬取;\n会清空之前的记录, 请做好备份!\n(继续爬取的按钮在旁边)"):
                self.addLog("您取消了重新爬取", "red")
                return
            try:
                # 如果之前有文件, 就删除一下
                DanMaKuXmlUtils.remove(ReqDataSingleton().outFile)
            except:
                pass
            self.running = True
            self.dmIdCnt = set() # 弹幕id哈希, 防止重复
            self.allDmCnt = 0    # 当前已经爬取的弹幕数量
            self.seniorDmCnt = 0 # 当前高级弹幕数量
            self.addLog("开始重新爬取...", "green")
            if ReqDataSingleton().isGetAllDanMaKu:
                ReqDataSingleton().yearList = YearFamily(2009, int(time.strftime("%Y", time.localtime())))
            else:
                ReqDataSingleton().yearList = YearFamily(
                    datetime.datetime.strptime(ReqDataSingleton().startDate, '%Y-%m-%d').year,
                    datetime.datetime.strptime(ReqDataSingleton().endDate, '%Y-%m-%d').year
                )
                ReqDataSingleton().yearList.setNowAllIndexFromDate(ReqDataSingleton().startDate)
            self._startThread()
        else:
            if messagebox.askyesno("确认", "确定要终止爬取吗?"):
                self.running = False
                self.addLog("终止爬取", "red")
                # 为弹幕文件写入 </i>
                DanMaKuXmlUtils.writeDmEndToXml(ReqDataSingleton().outFile)
                self.saveConfig()
        self.updateButtonTextByRunning()
    
    def updateReq(self):
        """
        主线程: 更新数据
        """
        try:
            while True:
                item = self.queue.get_nowait()
                if isinstance(item, str):
                    # 显示消息
                    self.addLog(item, "#F0F0F0")
                    self.status_label['text'] = f"已爬取: {self.allDmCnt} 条弹幕; 其中有 {self.seniorDmCnt} 条高级弹幕"
                elif isinstance(item, list):
                    # 写入到文件
                    DanMaKuXmlUtils.writeDmToXml(ReqDataSingleton().outFile, item)
        except queue.Empty:
            pass
        self.master.after(1000, self.updateReq)

    def getBasDanMaKu(self) -> None:
        """
        爬取Bas弹幕专包, 并且保存
        """
        while True:
            if self.isThreadExit:
                raise ValueError("程序已退出")
            try:
                dmList = getBasDanMaKu(ReqDataSingleton().cid)
                writeXmlDm = []
                nowAdd = 0
                for it in dmList:
                    if it[7] not in self.dmIdCnt:
                        self.dmIdCnt.add(it[7])
                        # 写入弹幕
                        writeXmlDm.append(
                            f'<d p="{it[0]},{it[1]},{it[2]},{it[3]},{it[4]},{it[5]},{it[6]},{it[7]}">{it[8]}</d>\n'
                        )
                        nowAdd += 1
                self.allDmCnt += nowAdd
                self.queue.put(f"爬取 Bas弹幕专包 获取 {len(dmList)} 条弹幕")
                self.queue.put(writeXmlDm)
                return
            except:
                cs += 1
                self.queue.put(f"爬取 Bas弹幕专包 出错: 网络错误, 可能被封禁了!, 正在重试: {cs}/5 次")
                if cs >= 5:
                    self.isThreadExit = True
                    self.queue.put("程序已终止, 请暂停, 以保存状态!")
            finally:
                # 随机暂停
                sleepTime = random.uniform(ReqDataSingleton().timerMin, ReqDataSingleton().timerMax)
                self.queue.put(f"等待 {sleepTime} 秒...")
                for _ in range(int(sleepTime)):
                    time.sleep(1)
                    if self.isThreadExit:
                        raise ValueError("程序已退出")
                time.sleep(sleepTime - int(sleepTime))

    def saveDanMaKu(self, date, dmList) -> int:
        """保存弹幕

        Args:
            date (str): 当前爬取的日期
            dmList (list): 弹幕列表

        Returns:
            int: 新增弹幕数量
        """
        writeXmlDm = []
        seniorDmCnt = 0
        nowAdd = 0
        for it in dmList:
            if int(it[7]) not in self.dmIdCnt:
                self.dmIdCnt.add(int(it[7]))
                # 写入弹幕
                writeXmlDm.append(
                    f'<d p="{it[0]},{it[1]},{it[2]},{it[3]},{it[4]},{it[5]},{it[6]},{it[7]}">{it[8]}</d>\n'
                )
                if int(it[1]) == 7:
                    seniorDmCnt += 1
                nowAdd += 1
        self.allDmCnt += nowAdd
        self.seniorDmCnt += seniorDmCnt
        self.queue.put(f"爬取 {date} 获取 {len(dmList)} 条弹幕; 新增 {nowAdd} 条弹幕, 高级弹幕新增 {seniorDmCnt} 条.")
        self.queue.put(writeXmlDm)
        return nowAdd

    def getDanMaKu(self, date: str) -> bool:
        """爬取代码并且保存, 返回是否有弹幕

        Args:
            date (str): 需要爬取的日期

        Raises:
            ValueError: 程序已退出

        Returns:
            bool: 是否有弹幕
        """
        cs = 0
        while True:
            if self.isThreadExit:
                raise ValueError("程序已退出")
            try:
                dmList = getHistoricalDanMaKu(date, ReqDataSingleton().cid)
                self.saveDanMaKu(date, dmList)
                return len(dmList) > 0
            except:
                cs += 1
                self.queue.put(f"爬取 {date} 出错: 网络错误, 可能被封禁了!, 正在重试: {cs}/5 次")
                self.isWifiNotGood = True
                if cs >= 5:
                    self.isThreadExit = True
                    self.isWifiNotGood = False
                    self.queue.put("程序已终止, 请暂停, 以保存状态!")
                    if ReqDataSingleton().yearList.nowAllIndex != -1:
                        ReqDataSingleton().yearList.unNext()
                continue
            finally:
                # 随机暂停
                sleepTime = random.uniform(ReqDataSingleton().timerMin, ReqDataSingleton().timerMax)
                self.queue.put(f"等待 {sleepTime} 秒...")
                for _ in range(int(sleepTime)):
                    time.sleep(1)
                    if self.isThreadExit:
                        raise ValueError("程序已退出")
                time.sleep(sleepTime - int(sleepTime))

    def getDanMaKuPrime(self, date: str) -> int:
        """获取弹幕, 并且通过算法分析, 返回的是下一个需要爬取的日期相当于date的偏移量

        Args:
            date (str): 需要爬取的弹幕的日期

        Returns:
            int: 下一个需要爬取的日期相当于date的偏移量
        """
        def addCntToNextCnt(cnt: int, poolSize: int) -> int:
            """经验主义的跳步

            Args:
                cnt (int): 新增的弹幕数
                poolSize (int): 弹幕池大小

            Returns:
                int: 应该跳步的步长, 如果为 -1 则需要回溯
            """
            if cnt >= poolSize * 0.5:
                return 1
            elif cnt >= poolSize * 0.3:
                return 2
            elif cnt >= poolSize * 0.2:
                return 3
            elif cnt >= poolSize * 0.1:
                return 5
            # elif cnt >= poolSize * 0.075:
            #     return 6
            elif cnt >= poolSize * 0.05:
                return 7
            elif cnt >= poolSize * 0.025:
                return 8
            elif cnt >= poolSize * 0.001:
                return 9
            else: # 你着视频妹人看啊
                return 10
        cs = 0
        while True:
            if self.isThreadExit:
                raise ValueError("程序已退出")
            try:
                dmList = getHistoricalDanMaKu(date, ReqDataSingleton().cid)
                poolSize = len(dmList)
                addCnt = self.saveDanMaKu(date, dmList)
                if addCnt == poolSize: # 触发回溯
                    return -1
                return addCntToNextCnt(addCnt, poolSize)
            except:
                cs += 1
                self.queue.put(f"爬取 {date} 出错: 网络错误, 可能被封禁了!, 正在重试: {cs}/5 次")
                self.isWifiNotGood = True
                if cs >= 5:
                    self.isThreadExit = True
                    self.isWifiNotGood = False
                    self.queue.put("程序已终止, 请暂停, 以保存状态!")
                    if ReqDataSingleton().yearList.nowAllIndex != -1:
                        ReqDataSingleton().yearList.unNext()
                continue
            finally:
                # 随机暂停
                sleepTime = random.uniform(ReqDataSingleton().timerMin, ReqDataSingleton().timerMax)
                self.queue.put(f"等待 {sleepTime} 秒...")
                for _ in range(int(sleepTime)):
                    time.sleep(1)
                    if self.isThreadExit:
                        raise ValueError("程序已退出")
                time.sleep(sleepTime - int(sleepTime))

    def runReq(self):
        """
        子线程: 爬取数据
        """
        # 看看需要保存弹幕的文件在不在, 如果存在, 则继续爬取, 否则新建文件
        if not DanMaKuXmlUtils.initXmlHead(ReqDataSingleton().outFile, ReqDataSingleton().cid):
            # 文件存在
            lineLen = DanMaKuXmlUtils.isLineCountGreaterThan(ReqDataSingleton().outFile)
            # 读取最后 min(3000, lineLen) 行
            # 并且加入 hash
            res = DanMaKuXmlUtils.readLastNLines(ReqDataSingleton().outFile, lineLen)
            reDmMid = re.compile('<d p="\\d+\\.\\d+,\\d,\\d+,\\d+,\\d+,\\d,[A-Za-z0-9]+,(\\d+)">.*?</d>') # 弹幕id
            self.queue.put("正在将上一次的爬取记录添加到去重哈希表...")
            for it in res:
                try:
                    item = re.findall(reDmMid, it.decode("utf-8"))[0]
                    item = int(item)
                    if item not in self.dmIdCnt:
                        self.dmIdCnt.add(item)
                except:
                    pass
        else:
            # 爬取Bas弹幕专包
            self.queue.put("开始爬取 Bas弹幕专包...")
            self.getBasDanMaKu()

        try:
            if ReqDataSingleton().yearList.nowAllIndex == -1:
                if ReqDataSingleton().isGetAllDanMaKu: # 二分爬取全弹幕
                    self.queue.put("开始二分爬取, 请勿退出!!!")
                    ReqDataSingleton().yearList.findBoundary(self.getDanMaKu)
                    self.saveConfig()
                    if ReqDataSingleton().isGetOptimize:
                        self.dmIdCnt = set()
                    self.queue.put("二分爬取结束, 状态已记录..")
                else:
                    ReqDataSingleton().yearList.nowAllIndex = 0
            
            self.queue.put("开始顺序爬取")
            if ReqDataSingleton().isGetOptimize:
                self.queue.put("[启用]爬取速度优化算法")
                maeJp = 0
                jp = 0
                while self.running:
                    date = ReqDataSingleton().yearList.next() # += 1

                    # 纠正
                    if datetime.datetime.strptime(date, '%Y-%m-%d') >= datetime.datetime.strptime(ReqDataSingleton().endDate, '%Y-%m-%d'): # 爬取完毕
                        self.isThreadExit = True
                        while datetime.datetime.strptime(date, '%Y-%m-%d') >= datetime.datetime.strptime(ReqDataSingleton().endDate, '%Y-%m-%d'):
                            ReqDataSingleton().yearList.unNext()
                            date = ReqDataSingleton().yearList.getDateFromAllIndex(
                                ReqDataSingleton().yearList.nowAllIndex
                            )
                        self.queue.put(f'爬取 cid: {ReqDataSingleton().cid} 完成!')
                        return
                    
                    maeJp = jp
                    jp = self.getDanMaKuPrime(date)
                    if (jp == -1 and maeJp != 0):
                        self.queue.put(f"爬取 {date} 触发回溯机制! 需要回溯 {maeJp} 次!")
                        # 回溯
                        for _ in range(maeJp):
                            ReqDataSingleton().yearList.unNext() # -= 1
                            ReqDataSingleton().yearList.unNext() # -= 1
                            self.getDanMaKuPrime(ReqDataSingleton().yearList.next()) # += 1
                        # 归位
                        for _ in range(maeJp):
                            ReqDataSingleton().yearList.next()
                        self.queue.put(f"回溯结束! 继续爬取~")
                    else:
                        for _ in range(jp):
                            date = ReqDataSingleton().yearList.next() # += 1

                            # 如果跳过了, 则到最近的有效日期
                            if datetime.datetime.strptime(date, '%Y-%m-%d') >= datetime.datetime.strptime(ReqDataSingleton().endDate, '%Y-%m-%d'): # 爬取完毕
                                ReqDataSingleton().yearList.unNext() # -= 1
                                self.queue.put(f"最后一次爬取!")
                                date = ReqDataSingleton().yearList.next()
                                self.getDanMaKuPrime(date)
                                self.queue.put(f'爬取 cid: {ReqDataSingleton().cid} 完成!')
                                self.saveConfig()
                                self.isThreadExit = True
                                return
                    self.saveConfig()
                    if datetime.datetime.strptime(date, '%Y-%m-%d') >= datetime.datetime.strptime(ReqDataSingleton().endDate, '%Y-%m-%d'): # 爬取完毕
                        self.isThreadExit = True
                        self.queue.put(f'爬取 cid: {ReqDataSingleton().cid} 完成!')
                        break
            else:
                while self.running:
                    date = ReqDataSingleton().yearList.next()
                    self.getDanMaKu(date)
                    self.saveConfig()
                    if datetime.datetime.strptime(date, '%Y-%m-%d') >= datetime.datetime.strptime(ReqDataSingleton().endDate, '%Y-%m-%d'): # 爬取完毕
                        self.isThreadExit = True
                        self.queue.put(f'爬取 cid: {ReqDataSingleton().cid} 完成!')
                        break
        except ValueError:
            print("子线程已退出")
            self.isThreadExit = True

    def continueScrape(self):
        """
        继续爬取
        """
        if self.running:
            self.addLog("暂停爬取", "yellow")
            self.saveConfig()
        else:
            self.addLog("继续爬取", "yellow")
            self._startThread()
        self.running = not self.running
        self.updateButtonTextByRunning()

    def addLog(self, message: str, color: str):
        """
        添加一条日志
        """
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_message = f"[{timestamp}]: {message}\n"
        self.log_text.insert(tk.END, log_message, color)

        # 改变文本颜色
        self.log_text.tag_configure(color, foreground=color)
        self.log_text.tag_add(color, "end-1c linestart", "end")

        # 确保文本框滚动到最下面
        if self.isLogGoToEnd:
            self.log_text.see(tk.END)

    def openTimerSettings(self):
        # 创建设置间隔时间的窗口
        settings_window = tk.Toplevel(self.master)
        settings_window.title("设置爬取间隔时间")
        settings_window.geometry("300x300")

        # 最小时间输入
        tk.Label(settings_window, text="最小时间(秒):").pack(pady=5)
        self.min_entry = tk.Entry(settings_window)
        self.min_entry.insert(0, str(ReqDataSingleton().timerMin))  # 默认值
        self.min_entry.pack(pady=5)

        # 最大时间输入
        tk.Label(settings_window, text="最大时间(秒):").pack(pady=5)
        self.max_entry = tk.Entry(settings_window)
        self.max_entry.insert(0, str(ReqDataSingleton().timerMax))  # 默认值
        self.max_entry.pack(pady=5)

        # 确认按钮
        confirm_button = tk.Button(settings_window, text="确认", command=self.updateTimer)
        confirm_button.pack(pady=10)

    def updateTimer(self):
        try:
            # 更新最小和最大时间
            ReqDataSingleton().timerMin = int(self.min_entry.get())
            ReqDataSingleton().timerMax = int(self.max_entry.get())
            self.addLog(f"更新成功: 最小时间 = {ReqDataSingleton().timerMin}, 最大时间 = {ReqDataSingleton().timerMax}", "#CC563F")
        except ValueError:
            self.addLog("设置爬取间隔失败!", "red")

    def showAbout(self):
        """
        关于界面
        """
        # 创建一个新的顶级窗口
        about_window = tk.Toplevel()
        about_window.title("关于")
        about_window.geometry("600x240")

        # 添加信息标签
        tk.Label(about_window, text=f"弹幕爬取工具 V{version.HX_BILIBILI_DANMU_CRAWLING_VERSION}", font=("黑体", 14)).pack(pady=10)

        # 作者
        tk.Label(about_window, text="作者: Heng_Xin", font=("黑体", 14), fg="#990099").pack(pady=10)

        # 添加链接标签
        link = tk.Label(about_window, text="项目地址:\nhttps://github.com/HengXin666/BiLiBiLi_DanMu_Crawling", fg="blue", cursor="hand2", font=("黑体", 14))
        link.pack()
        link.bind("<Button-1>", lambda e: webbrowser.open_new("https://github.com/HengXin666/BiLiBiLi_DanMu_Crawling"))  # 替换为你的链接

        tk.Label(about_window, text=f"当前版本更新时间: {version.HX_BILIBILI_DANMU_CRAWLING_UPDATE_DATE}", font=("黑体", 14)).pack(pady=10)

        # 添加关闭按钮
        close_button = tk.Label(about_window, text="关闭", fg="red", cursor="hand2", font=("黑体", 14))
        close_button.pack(pady=5)
        close_button.bind("<Button-1>", lambda e: about_window.destroy())

    def toggleTheme(self):
        """
        切换主题逻辑
        """
        self.is_dark_mode = not self.is_dark_mode
        self.updateTheme()

    def updateTheme(self):
        """
        进行主题切换
        """
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

    def setFontSize(self):
        """
        设置字体大小
        """
        size = simpledialog.askinteger("字体大小", "输入字体大小:", minvalue=8, maxvalue=30)
        if size:
            self.custom_font.configure(size=size)
            self.updateTheme()
    
    def saveConfig(self):
        """
        保存日志, 以实时更新
        """
        ReqDataSingleton().startDate = self.dateObj.start_date.get()
        if ReqDataSingleton().isGetToNowTime:
                ReqDataSingleton().endDate = time.strftime("%Y-%m-%d", time.localtime())
        else:
            ReqDataSingleton().endDate = self.dateObj.end_date.get()
        ReqDataSingleton().save() # 保存

def start() -> None:
    root = tk.Tk()
    app = VideoScraperUI(root)
    root.mainloop()
    app.saveConfig()
    app.running = False
    app.isThreadExit = True
    print("等待退出...(2秒)")
    time.sleep(2)

if __name__ == "__main__":
    start()
