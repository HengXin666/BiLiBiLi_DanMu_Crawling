# -*- coding: utf-8 -*-
import calendar
import tkinter as tk
import tkinter.font as tkFont
from tkinter import ttk

datetime = calendar.datetime.datetime
timedelta = calendar.datetime.timedelta

"""
CV By https://blog.csdn.net/chaodaibing/article/details/107444031
"""

class Calendar:
    def __init__(s, point=None):
        s.master = tk.Toplevel()
        s.master.withdraw()
        s.master.attributes("-topmost", True)
        fwday = calendar.SUNDAY
        year = datetime.now().year
        month = datetime.now().month
        locale = None
        sel_bg = "#ecffc4"
        sel_fg = "#05640e"
        s._date = datetime(year, month, 1)  # 每月第一日
        s._selection = None  # 设置为未选中日期
        s.G_Frame = ttk.Frame(s.master)
        s._cal = s.__get_calendar(locale, fwday)
        s.__setup_styles()  # 创建自定义样式
        s.__place_widgets()  # pack/grid 小部件
        s.__config_calendar()  # 调整日历列和安装标记
        # 配置画布和正确的绑定，以选择日期。
        s.__setup_selection(sel_bg, sel_fg)
        # 存储项ID，用于稍后插入。
        s._items = [s._calendar.insert("", "end", values="") for _ in range(6)]
        # 在当前空日历中插入日期
        s._update()
        s.G_Frame.pack(expand=1, fill="both")
        s.master.overrideredirect(1)
        s.master.update_idletasks()
        width, height = s.master.winfo_reqwidth(), s.master.winfo_reqheight()
        s.height = height
        if point:
            x, y = point[0], point[1]
        else:
            x, y = (s.master.winfo_screenwidth() - width) / 2, (
                s.master.winfo_screenheight() - height
            ) / 2
        s.master.geometry("%dx%d+%d+%d" % (width, height, x, y))  # 窗口位置居中
        s.master.after(300, s._main_judge)
        s.master.deiconify()
        s.master.focus_set()
        s.master.wait_window()  # 这里应该使用wait_window挂起窗口，如果使用mainloop,可能会导致主程序很多错误

    def __get_calendar(s, locale, fwday):
        if locale is None:
            return calendar.TextCalendar(fwday)
        else:
            return calendar.LocaleTextCalendar(fwday, locale)

    def __setitem__(s, item, value):
        if item in ("year", "month"):
            raise AttributeError("attribute '%s' is not writeable" % item)
        elif item == "selectbackground":
            s._canvas["background"] = value
        elif item == "selectforeground":
            s._canvas.itemconfigure(s._canvas.text, item=value)
        else:
            s.G_Frame.__setitem__(s, item, value)

    def __getitem__(s, item):
        if item in ("year", "month"):
            return getattr(s._date, item)
        elif item == "selectbackground":
            return s._canvas["background"]
        elif item == "selectforeground":
            return s._canvas.itemcget(s._canvas.text, "fill")
        else:
            r = ttk.tclobjs_to_py({item: ttk.Frame.__getitem__(s, item)})
            return r[item]

    def __setup_styles(s):
        # 自定义TTK风格
        style = ttk.Style(s.master)
        arrow_layout = lambda dir: (
            [("Button.focus", {"children": [("Button.%sarrow" % dir, None)]})]
        )
        style.layout("L.TButton", arrow_layout("left"))
        style.layout("R.TButton", arrow_layout("right"))

    def __place_widgets(s):
        # 标头框架及其小部件
        Input_judgment_num = s.master.register(
            s.Input_judgment
        )  # 需要将函数包装一下，必要的
        hframe = ttk.Frame(s.G_Frame)
        gframe = ttk.Frame(s.G_Frame)
        bframe = ttk.Frame(s.G_Frame)
        hframe.pack(in_=s.G_Frame, side="top", pady=5, anchor="center")
        gframe.pack(in_=s.G_Frame, fill=tk.X, pady=5)
        bframe.pack(in_=s.G_Frame, side="bottom", pady=5)
        lbtn = ttk.Button(hframe, style="L.TButton", command=s._prev_month)
        lbtn.grid(in_=hframe, column=0, row=0, padx=12)
        rbtn = ttk.Button(hframe, style="R.TButton", command=s._next_month)
        rbtn.grid(in_=hframe, column=5, row=0, padx=12)
        s.CB_year = ttk.Combobox(
            hframe,
            width=5,
            values=[
                str(year)
                for year in range(datetime.now().year, datetime.now().year - 11, -1)
            ],
            validate="key",
            validatecommand=(Input_judgment_num, "%P"),
        )
        s.CB_year.current(0)
        s.CB_year.grid(in_=hframe, column=1, row=0)
        s.CB_year.bind("<KeyPress>", lambda event: s._update(event, True))
        s.CB_year.bind("<<ComboboxSelected>>", s._update)
        tk.Label(hframe, text="年", justify="left").grid(
            in_=hframe, column=2, row=0, padx=(0, 5)
        )
        s.CB_month = ttk.Combobox(
            hframe,
            width=3,
            values=["%02d" % month for month in range(1, 13)],
            state="readonly",
        )
        s.CB_month.current(datetime.now().month - 1)
        s.CB_month.grid(in_=hframe, column=3, row=0)
        s.CB_month.bind("<<ComboboxSelected>>", s._update)
        tk.Label(hframe, text="月", justify="left").grid(in_=hframe, column=4, row=0)
        # 日历部件
        s._calendar = ttk.Treeview(gframe, show="", selectmode="none", height=7)
        s._calendar.pack(expand=1, fill="both", side="bottom", padx=5)
        ttk.Button(bframe, text="确 定", width=6, command=lambda: s._exit(True)).grid(
            row=0, column=0, sticky="ns", padx=20
        )
        ttk.Button(bframe, text="取 消", width=6, command=s._exit).grid(
            row=0, column=1, sticky="ne", padx=20
        )
        tk.Frame(s.G_Frame, bg="#565656").place(
            x=0, y=0, relx=0, rely=0, relwidth=1, relheigh=2 / 200
        )
        tk.Frame(s.G_Frame, bg="#565656").place(
            x=0, y=0, relx=0, rely=198 / 200, relwidth=1, relheigh=2 / 200
        )
        tk.Frame(s.G_Frame, bg="#565656").place(
            x=0, y=0, relx=0, rely=0, relwidth=2 / 200, relheigh=1
        )
        tk.Frame(s.G_Frame, bg="#565656").place(
            x=0, y=0, relx=198 / 200, rely=0, relwidth=2 / 200, relheigh=1
        )

    def __config_calendar(s):
        # cols = s._cal.formatweekheader(3).split()
        cols = ["日", "一", "二", "三", "四", "五", "六"]
        s._calendar["columns"] = cols
        s._calendar.tag_configure("header", background="grey90")
        s._calendar.insert("", "end", values=cols, tag="header")
        # 调整其列宽
        font = tkFont.Font()
        maxwidth = max(font.measure(col) for col in cols)
        for col in cols:
            s._calendar.column(col, width=maxwidth, minwidth=maxwidth, anchor="center")

    def __setup_selection(s, sel_bg, sel_fg):
        def __canvas_forget(evt):
            canvas.place_forget()
            s._selection = None

        s._font = tkFont.Font()
        s._canvas = canvas = tk.Canvas(
            s._calendar, background=sel_bg, borderwidth=0, highlightthickness=0
        )
        canvas.text = canvas.create_text(0, 0, fill=sel_fg, anchor="w")
        canvas.bind("<Button-1>", __canvas_forget)
        s._calendar.bind("<Configure>", __canvas_forget)
        s._calendar.bind("<Button-1>", s._pressed)

    def _build_calendar(s):
        year, month = s._date.year, s._date.month
        header = s._cal.formatmonthname(year, month, 0)
        # 更新日历显示的日期
        cal = s._cal.monthdayscalendar(year, month)
        for indx, item in enumerate(s._items):
            week = cal[indx] if indx < len(cal) else []
            fmt_week = [("%02d" % day) if day else "" for day in week]
            s._calendar.item(item, values=fmt_week)

    def _show_select(s, text, bbox):
        x, y, width, height = bbox
        textw = s._font.measure(text)
        canvas = s._canvas
        canvas.configure(width=width, height=height)
        canvas.coords(canvas.text, (width - textw) / 2, height / 2 - 1)
        canvas.itemconfigure(canvas.text, text=text)
        canvas.place(in_=s._calendar, x=x, y=y)

    def _pressed(s, evt=None, item=None, column=None, widget=None):
        """在日历的某个地方点击"""
        if not item:
            x, y, widget = evt.x, evt.y, evt.widget
            item = widget.identify_row(y)
            column = widget.identify_column(x)
        if not column or not item in s._items:
            # 在工作日行中单击或仅在列外单击。
            return
        item_values = widget.item(item)["values"]
        if not len(item_values):  # 这个月的行是空的。
            return
        text = item_values[int(column[1]) - 1]
        if not text:
            return
        bbox = widget.bbox(item, column)
        if not bbox:  # 日历尚不可见
            s.master.after(
                20, lambda: s._pressed(item=item, column=column, widget=widget)
            )
            return
        text = "%02d" % text
        s._selection = (text, item, column)
        s._show_select(text, bbox)

    def _prev_month(s):
        """更新日历以显示前一个月"""
        s._canvas.place_forget()
        s._selection = None
        s._date = s._date - timedelta(days=1)
        s._date = datetime(s._date.year, s._date.month, 1)
        s.CB_year.set(s._date.year)
        s.CB_month.set(s._date.month)
        s._update()

    def _next_month(s):
        """更新日历以显示下一个月"""
        s._canvas.place_forget()
        s._selection = None

        year, month = s._date.year, s._date.month
        s._date = s._date + timedelta(days=calendar.monthrange(year, month)[1] + 1)
        s._date = datetime(s._date.year, s._date.month, 1)
        s.CB_year.set(s._date.year)
        s.CB_month.set(s._date.month)
        s._update()

    def _update(s, event=None, key=None):
        """刷新界面"""
        if key and event.keysym != "Return":
            return
        year = int(s.CB_year.get())
        month = int(s.CB_month.get())
        if year == 0 or year > 9999:
            return
        s._canvas.place_forget()
        s._date = datetime(year, month, 1)
        s._build_calendar()  # 重建日历
        if year == datetime.now().year and month == datetime.now().month:
            day = datetime.now().day
            for _item, day_list in enumerate(s._cal.monthdayscalendar(year, month)):
                if day in day_list:
                    item = "I00" + str(_item + 2)
                    column = "#" + str(day_list.index(day) + 1)
                    s.master.after(
                        100,
                        lambda: s._pressed(
                            item=item, column=column, widget=s._calendar
                        ),
                    )

    def _exit(s, confirm=False):
        if not confirm:
            s._selection = None
        s.master.destroy()

    def _main_judge(s):
        """判断窗口是否在最顶层"""
        try:
            if s.master.focus_displayof() == None or "toplevel" not in str(
                s.master.focus_displayof()
            ):
                s._exit()
            else:
                s.master.after(10, s._main_judge)
        except:
            s.master.after(10, s._main_judge)

    def selection(s):
        """返回表示当前选定日期的日期时间"""
        if not s._selection:
            return None
        year, month = s._date.year, s._date.month
        return str(datetime(year, month, int(s._selection[0])))[:10]

    def Input_judgment(s, content):
        """输入判断"""
        if content.isdigit() or content == "":
            return True
        else:
            return False


class Datepicker:
    def __init__(s, window, axes):  # 窗口对象 坐标
        s.window = window
        s.frame = tk.Frame(s.window, padx=5)
        s.frame.grid(row=axes[0], column=axes[1])
        s.start_date = tk.StringVar()  # 开始日期
        s.end_date = tk.StringVar()  # 结束日期
        s.bt1 = tk.Button(
            s.frame, text="开始", command=lambda: s.getdate("start")
        )  # 开始按钮
        s.bt1.grid(row=0, column=0)
        s.ent1 = tk.Entry(s.frame, textvariable=s.start_date)  # 开始输入框
        s.ent1.grid(row=0, column=1)
        s.bt2 = tk.Button(s.frame, text="结束", command=lambda: s.getdate("end"))
        s.bt2.grid(row=0, column=2)
        s.ent2 = tk.Entry(s.frame, textvariable=s.end_date)
        s.ent2.grid(row=0, column=3)

    def getdate(s, type):  # 获取选择的日期
        for date in [Calendar().selection()]:
            if date:
                if type == "start":  # 如果是开始按钮，就赋值给开始日期
                    s.start_date.set(date)
                elif type == "end":
                    s.end_date.set(date)


# 执行
if __name__ == "__main__":
    window = tk.Tk()
    window.wm_attributes("-topmost", True)  # 窗口置顶
    tk.Label(window, text="日期段一:").grid(row=0, column=0)
    obj = Datepicker(window, (0, 1))  # 初始化类为对象
    startstamp1 = obj.start_date.get()  # 获取开始时期
    endstamp1 = obj.end_date.get()

    tk.Label(window, text="日期段二:").grid(row=1, column=0)
    obj = Datepicker(window, (1, 1))
    startstamp2 = obj.start_date.get()
    endstamp2 = obj.end_date.get()
    window.mainloop()
