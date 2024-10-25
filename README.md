# BiliBili历史全弹幕获取
## 说明

本程序可爬取B站视频的历史弹幕, 支持爬取`高级弹幕`/`BAS弹幕`

> ~~BAS弹幕可能还有点问题, 因为样本太少了, 我也无法测试qwq..~~

- 程序带有GUI界面.

- 支持多个`cookies`, 并且随机选择一个进行爬取.

- 支持断点续爬: 可以从上一次爬取的地方接着开始爬取, 并且实时输出弹幕文件(`.xml`)

- 会自动保存上一次的配置, 下一次打开不需要重新输入, 直接`继续爬取`即可!

- 利用`二分`定位第一个有弹幕的天数:

    ![网络错误: 二分定位](dev/PixPin_2024-10-25_13-14-47.png)

    之后会跳过在`二分`时候已经爬取的数据, 避免重复爬取

    (因为之前的`判断这个月存在弹幕的天数`的api, 只能返回`2021-07`之后的数据; 因此改为`二分`确定范围)

> [!TIP]
>
> 为什么不是手动输入爬取范围为`视频发布日期`呢?
>
> 答: 因为我发现, `av314`它的发布日期是`2012-08-19`(B站界面上显示); 但是爬取`2012-07-19`也是有历史弹幕返回的 (可能某些原因视频被重新上传? 或者B站炸了); ~~(总之不能浪费)~~

- 始终爬取到当天: 可能爬取很久, 可能已经到了新的一天了; 因此爬取弹幕也可以设置爬取新的一天的; 而不是定死在了开始爬取的那一天.

## 界面展示

![主界面](dev/Clip_2024-10-25_13-34-13.png)

![分界面](dev/Clip_2024-10-25_13-36-41.png)

### 使用

- 爬取弹幕需要获取B站的`cookies`:

1. 登录网页版B站

2. 点击键盘`F12`, 进入浏览器审查界面

3. 如下操作

![凭证获取](dev/Clip_2024-10-25_14-53-03.png)

4. 复制到程序对应地方, 进行添加

## 构建要求

- python3.8+ (我本地环境是`3.12.7`)

安装下列依赖:

```sh
pip install requests protobuf
```

如果还不行就请参照 [requirements.txt](./requirements.txt) 这个是我导出的所有安装的第三方库, 可能有用不上的.

## 存在问题
- GUI太丑了... ~~我没有学过py的`tk`~~

- 请小心使用, 我只能保证简单的GUI逻辑没有问题(爬取/继续爬取); 一些复杂的GUI联动细节我可能没有注意到, 从而可能会导致BUG!

- 请务必点击`暂停爬取`, 再关闭程序; 否则可能会丢失配置!

- 关闭程序请点击窗口的叉叉; 而 **不是** 强行关闭控制台窗口或者对控制台`ctrl+c`, 可能导致配置丢失!!!

- 爬取性能优化: 因为弹幕池的增量可能每天就只有几百条, 而容量却是3000条; 从而导致爬取的3000条弹幕中, 可能有2000条是多余的, 此时应该可以改变步长, 隔几天才爬取一天, 这样爬取的数量就多了 (没时间搞这个了, 目前能用就行了...)

- 这种爬取模式(断点继爬)下, 难免会存在重复弹幕, 有条件的可以使用其他软件对弹幕进行去重~

## 问题反馈

- 您可以提`Issues`, 我几乎都会看的qwq..

## 许可证

简单地说:

- 不允许商用行为

- 产生不良后果, 与作者无关

- 必要时, 请标明出处

## 致谢

- [哔哩哔哩 - API 收集整理](https://github.com/SocialSisterYi/bilibili-API-collect)

### 支持

感谢各位的支持, 如果喜欢的话可以点一个`Start`吗?

[![Stargazers repo roster for @HengXin666/BiLiBiLi_DanMu_Crawling](https://reporoster.com/stars/HengXin666/BiLiBiLi_DanMu_Crawling)](https://github.com/HengXin666/BiLiBiLi_DanMu_Crawling/stargazers)

[![Forkers repo roster for @HengXin666/BiLiBiLi_DanMu_Crawling](https://reporoster.com/forks/HengXin666/BiLiBiLi_DanMu_Crawling)](https://github.com/HengXin666/BiLiBiLi_DanMu_Crawling/network/members)