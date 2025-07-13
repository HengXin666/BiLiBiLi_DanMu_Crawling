from datetime import datetime, timezone

class TimeString:
    @staticmethod
    def getLocalTimeStr() -> str:
        """获取当前本地时间字符串

        Returns:
            str: %Y-%m-%d 格式
        """
        return datetime.now().strftime("%Y-%m-%d")

    @staticmethod
    def timestampToStr(ts: int) -> str:
        """
        将 Unix 时间戳转换为 'YYYY-MM-DD' 格式字符串
        
        参数:
            ts (int): Unix 时间戳（秒）
        
        返回:
            str: 格式化的日期字符串
        """
        return datetime.fromtimestamp(ts, timezone.utc).strftime("%Y-%m-%d")

    @staticmethod
    def strToTimestamp(date_str: str) -> int:
        """
        将 'YYYY-MM-DD' 格式字符串转换为 Unix 时间戳
        
        参数:
            date_str (str): 格式化的日期字符串
        
        返回:
            int: Unix 时间戳（秒）
        """
        dt = datetime.strptime(date_str, "%Y-%m-%d")
        return int(dt.replace(tzinfo=timezone.utc).timestamp())