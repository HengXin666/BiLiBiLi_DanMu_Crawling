from datetime import datetime, timezone

from datetime import datetime, timezone, timedelta

class TimeString:
    # 北京时区
    BEIJING_TZ = timezone(timedelta(hours=8))

    @staticmethod
    def getLocalTimeStr() -> str:
        """获取当前北京时间字符串

        Returns:
            str: %Y-%m-%d 格式
        """
        return datetime.now(TimeString.BEIJING_TZ).strftime("%Y-%m-%d")
    
    @staticmethod
    def getLocalTimestamp() -> int:
        """获取当前北京时间时间戳(秒)

        Returns:
            int: Unix 时间戳(北京时间，非 UTC)
        """
        return int(datetime.now(TimeString.BEIJING_TZ).timestamp())

    @staticmethod
    def timestampToStr(ts: int) -> str:
        """
        将 Unix 时间戳转换为北京时间 'YYYY-MM-DD' 格式字符串
        
        参数:
            ts (int): Unix 时间戳(秒)
        
        返回:
            str: 格式化的日期字符串(北京时间)
        """
        return datetime.fromtimestamp(ts, TimeString.BEIJING_TZ).strftime("%Y-%m-%d")

    @staticmethod
    def strToTimestamp(date_str: str) -> int:
        """
        将 'YYYY-MM-DD' 字符串(北京时间)转换为 Unix 时间戳
        
        参数:
            date_str (str): 格式化的日期字符串(北京时间)
        
        返回:
            int: Unix 时间戳(北京时间)
        """
        dt = datetime.strptime(date_str, "%Y-%m-%d")
        dt = dt.replace(tzinfo=TimeString.BEIJING_TZ)
        return int(dt.timestamp())
