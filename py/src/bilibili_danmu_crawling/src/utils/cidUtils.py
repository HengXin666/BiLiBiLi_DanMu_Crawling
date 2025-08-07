import re

class CidUtils:
    @staticmethod
    def extractCid(text: str) -> str | None:
        res = re.search(r"cid=(\d+)", text, re.IGNORECASE)
        if res:
            return res.group(1)
        return None
    
    @staticmethod
    def extractBv(text: str) -> str | None:
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
    
    @staticmethod
    def extractANiMe(text: str) -> dict[str, str | None]:
        """
        提前输入框, 获取md, ss, ep号

        return dict()

        其中 
            dict()['mdId']: str | None
            dict()['ssId']: str | None
            dict()['epId']: str | None
        """
        
        mdMatch = re.search(r"/md(\d+)", text)
        ssMatch = re.search(r"/ss(\d+)", text)
        epMatch = re.search(r"/ep(\d+)", text)

        return {
            'mdId': mdMatch.group(1) if mdMatch else None,
            'ssId': ssMatch.group(1) if ssMatch else None,
            'epId': epMatch.group(1) if epMatch else None,
        }