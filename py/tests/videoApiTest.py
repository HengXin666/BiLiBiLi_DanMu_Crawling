import Include
from src.api import videoApi as Api

if __name__ == '__main__':
    api = Api.VideoApi()

    print(api.getCidPart("https://www.bilibili.com/bangumi/play/ss91498?from_spmid=666.4.mylist.1"), "\n")
    print(api.getCidPart("https://www.bilibili.com/bangumi/media/md1587"), "\n")
    print(api.getCidPart("https://www.bilibili.com/bangumi/play/ep85880?from_spmid=666.4.feed.11"), "\n")
    print(api.getCidPart("https://www.bilibili.com/video/BV1Js411o76u/?vd_source=67ebe8ad7f6fc7a37e0608d544169bd8"), "\n")
    print(api.getCidPart("https://www.bilibili.com/video/BV19s6fYwECB/?spm_id_from=333.1387.homepage.video_card.click"), "\n")