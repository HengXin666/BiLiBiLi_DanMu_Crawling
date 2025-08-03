import json
import requests

import Include
from src.fileUtils.jsonConfig import GlobalConfig

if __name__ == '__main__':
    cookie = GlobalConfig().get().cookies[0]
    cid = 31441946404
    url = f"https://api.bilibili.com/x/v2/dm/search?oid={cid}&type=1&keyword=&order=ctime&sort=desc&pn=1&ps=50&cp_filter=false"
    headers = {
        "Accept": "*/*",
        "Accept-Language": "zh-CN,zh;q=0.9,en;q=0.8",
        "Origin": "https://member.bilibili.com",
        "User-Agent": "Mozilla/5.0 (X11; Linux x86_64; rv:141.0) Gecko/20100101 Firefox/141.0",
    }
    response = requests.get(url=url, headers=headers, cookies={
        "SESSDATA": cookie
    })
    data = response.json() 
    print(data)