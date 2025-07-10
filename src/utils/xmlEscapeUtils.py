class XmlEscapeUtil:
    """
    用于转义 XML 内容中的特殊字符, 使其可以安全嵌入到 XML 中
    """

    # 定义需要转义的 XML 字符及其对应实体
    _escape_map = {
        '&': '&amp;',
        '<': '&lt;',
        '>': '&gt;',
        '"': '&quot;',
        "'": '&apos;',
    }

    @classmethod
    def escape(cls, text: str) -> str:
        """
        转义给定字符串中的 XML 特殊字符。
        
        Args:
            text (str): 原始字符串
        
        Returns:
            str: 已转义的字符串
        """
        # 注意顺序: 必须先转义 &, 否则后续转义会把实体搞坏
        for char, escape in cls._escape_map.items():
            text = text.replace(char, escape)
        return text

if __name__ == '__main__':
    raw = "<tag>O'Reilly & <friends></tag>"
    escaped = XmlEscapeUtil.escape(raw)
    print(escaped == "&lt;tag&gt;O&apos;Reilly &amp; &lt;friends&gt;&lt;/tag&gt;")