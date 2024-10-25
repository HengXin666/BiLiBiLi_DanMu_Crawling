import json
import time
from datetime import datetime

class YearDays:
    def __init__(self, year):
        self.year = year
        self.days = self._initialize_days(year)

    def _initialize_days(self, year):
        if self._is_leap_year(year):
            return ['0'] * 366
        else:
            return ['0'] * 365

    def __len__(self):
        """返回该年份的天数"""
        return len(self.days)

    def _is_leap_year(self, year):
        return year % 4 == 0 and (year % 100 != 0 or year % 400 == 0)

    def getDateByIndex(self, index: int):
        """从索引获取到日期('2024-10-24')"""
        if self._is_leap_year(self.year):
            if index < 0 or index >= 366:
                raise IndexError("索引超出范围")
            day_of_year = index + 1
        else:
            if index < 0 or index >= 365:
                raise IndexError("索引超出范围")
            day_of_year = index + 1

        return f"{self.year}-{self._day_of_year_to_month(day_of_year):02d}-{self._day_of_year_to_day(day_of_year):02d}"

    def __setitem__(self, index, value):
        if self._is_leap_year(self.year):
            if index < 0 or index >= 366:
                raise IndexError("索引超出范围")
        else:
            if index < 0 or index >= 365:
                raise IndexError("索引超出范围")

        if value not in ['0', '1', '6']:
            raise ValueError("值必须为 ['0', '1', '6']")

        self.days[index] = value

    def toString(self):
        return ''.join(self.days)

    def fromString(self, binary_string):
        if len(binary_string) != (366 if self._is_leap_year(self.year) else 365):
            raise ValueError("字符串长度不符合该年份的天数")

        self.days = list(binary_string)

    def getDateVal(self, date_str: str):
        """获取指定日期的值"""
        date = datetime.strptime(date_str, '%Y-%m-%d')
        day_of_year = (date - datetime(date.year, 1, 1)).days
        if self.year != date.year or day_of_year < 0 or day_of_year >= len(self.days):
            raise ValueError("日期超出范围")
        return self.days[day_of_year]

    @classmethod
    def from_year_and_string(cls, year, binary_string):
        instance = cls(year)
        instance.fromString(binary_string)
        return instance

    def _day_of_year_to_month(self, day_of_year):
        if self._is_leap_year(self.year):
            days_in_month = [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
        else:
            days_in_month = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

        for month, days in enumerate(days_in_month, start=1):
            if day_of_year <= days:
                return month
            day_of_year -= days

    def _day_of_year_to_day(self, day_of_year):
        if self._is_leap_year(self.year):
            days_in_month = [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
        else:
            days_in_month = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

        for days in days_in_month:
            if day_of_year <= days:
                return day_of_year
            day_of_year -= days

class YearFamily:
    def __init__(self, start_year, end_year):
        self.year_days_list = [YearDays(year) for year in range(start_year, end_year + 1)]
        self.nowAllIndex = -1

    def _indexToArrIndex(self, index: int) -> tuple[int, int]:
        """从ALLIndex转为 Arr Index, 即 year_days_list[res[0]][res[1]]"""
        for i in range(len(self.year_days_list)):
            if index >= len(self.year_days_list[i]):
                index -= len(self.year_days_list[i])
            else:
                return (i, index)
        return (
            len(self.year_days_list) - 1, 
            len(self.year_days_list[len(self.year_days_list) - 1]) - 1
        )

    def findBoundary(self, callback):
        """
        二分得出有弹幕的开始日期
        
        `callback`判断函数, 传参是日期(`2024-10-24`)
        """
        low = 0
        high = sum(366 if yd._is_leap_year(yd.year) else 365 for yd in self.year_days_list) - 1

        while low < high:
            mid = (low + high) // 2
            date = self.getDateFromAllIndex(mid)
            arrIndex = self._indexToArrIndex(mid)
            # 如果之前有标记, 就按照标记来
            if self.year_days_list[arrIndex[0]].days[arrIndex[1]] != '0':
                if self.year_days_list[arrIndex[0]].days[arrIndex[1]] == '1':
                    high = mid
                else:
                    low = mid + 1
            else:
                # 标记查找过的日期
                if callback(date):
                    self.year_days_list[arrIndex[0]].days[arrIndex[1]] = '1'  # 资源存在
                    high = mid  # 向左查找
                else:
                    self.year_days_list[arrIndex[0]].days[arrIndex[1]] = '6'  # 资源不存在
                    low = mid + 1  # 向右查找

        self.nowAllIndex = low

    def getDateFromAllIndex(self, index) -> str:
        """从ALL索引获取日期"""
        idx = self._indexToArrIndex(index)
        return self.year_days_list[idx[0]].getDateByIndex(idx[1])

    def next(self):
        """获取下一个日期"""
        while True:
            arrIndex = self._indexToArrIndex(self.nowAllIndex)
            if self.year_days_list[arrIndex[0]].days[arrIndex[1]] == '0':
                res = self.getDateFromAllIndex(self.nowAllIndex)
                self.nowAllIndex += 1
                return res
            self.nowAllIndex += 1

    def toJsonDict(self) -> dict:
        """将年份族管理类序列化为 JSON Dict"""
        return {
            'list': [{"year": yd.year, "days": yd.toString()} for yd in self.year_days_list],
            'nowAllIndex': self.nowAllIndex # 开始爬取日期, -1 代表需要从二分开始
        }

    @classmethod
    def fromJson(cls, data: dict):
        """从 JSON 反序列化年份族管理类"""
        instance = cls(data['list'][0]['year'], data['list'][-1]['year'])
        for entry in data['list']:
            year_days = YearDays(entry['year'])
            year_days.fromString(entry['days'])
            instance.year_days_list[entry['year'] - instance.year_days_list[0].year] = year_days
        instance.nowAllIndex = data.get('nowAllIndex', -1)  # 恢复索引状态
        return instance

if __name__ == '__main__':
    # 示例使用
    year_family = YearFamily(2020, 2024)
    year_family.year_days_list[0][0] = '1'  # 2020-01-01
    year_family.year_days_list[1][0] = '1'  # 2021-01-01
    year_family.year_days_list[2][1] = '1'  # 2022-01-02
    year_family.year_days_list[3][364] = '1'  # 2023-12-30
    year_family.year_days_list[4][364] = '1'  # 2024-12-30

    idx = year_family._indexToArrIndex(356)
    print(idx, "->", year_family.year_days_list[idx[0]].getDateByIndex(idx[1]))

    # funX 示例调用
    def example_funX(date_str):
        # 这里实现你的逻辑，返回是否存在资源
        return time.strptime('2022-10-24', "%Y-%m-%d") <= time.strptime(date_str, "%Y-%m-%d")

    year_family.funX = example_funX  # 绑定示例逻辑

    boundary_index = year_family.findBoundary()
    print(year_family.getDateFromAllIndex(boundary_index))

    json_data = year_family.toJson()
    print(json_data)

    loaded_family = YearFamily.fromJson(json_data)
    print(loaded_family.funX('2022-01-01'))  # True
