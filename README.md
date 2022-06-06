# 爬取B站历史弹幕
声明: 

感谢原作者:zjkwdy https://github.com/zjkwdy/bilibili-history-danmu-spider 我在此基础上进行了一些?修改

参考> https://github.com/SocialSisterYi/bilibili-API-collect

若使用ip代理需要> https://github.com/jhao104/proxy_pool

# 主要功能:

    * --> [1].获取 B站某视频的cid

    * --> [2].设置 使用ip代理(从代理池中获取ip) (1为开启)
               |   抓取次数 (为0则停止爬取)

    * --> [3].爬取 B站某视频的所有历史弹幕

    * --> [4].接着 爬取(中断后会产生保存文件)
    
 # 改进特点:
 
 =============================================================
 
** 1 **
 * 原本是爬取弹幕的日期，然后根据日期，再爬取弹幕内容.
 * 如果量好还好说,大量爬取的话很容易被封 .
 
 =============================================================
 
 * 因此: 我改进为将爬取到的弹幕天数 先保存到本地
 * 然后再根据弹幕天数文件来进行爬取
 * 这样就不会因为某些错误操作导致又要重来
 * 【省去了一些时间和功夫】
 
 =============================================================
 
** 2 **
 * 然后是输出弹幕,最容易被封ip
 * 原来是: 爬取好所有弹幕再统一输出. 出错(被封了)也继续爬取下一天. 既浪费时间又没有获得什么.

 =============================================================
 
 * 因此: 我改进为边爬取边输出
 * 出现被封也能保存好文件 , 解封后根据弹幕天数文件 , 也能接着爬取 !

 =============================================================

** 3 **
* 支持同过 ip代理池来爬取弹幕 | 也是会被封的 ...
* 需要 https://github.com/jhao104/proxy_pool 的支持.

 =============================================================
 #  具体使用教程:
 
 B站视频:xxx
 
 =============================================================
 # 某些问题:
 =============================================================
 * 为何api会输出重复弹幕?
  
  ~~因为: 没有及时的弹幕更新. ~~(设历史弹幕库存为1000) ~~当库存有新弹幕进来,就会把旧弹幕(弹幕数量>1000),压入库中,可没压入库的，因为已经输出过一次了,就成了重复弹幕...(好像语文表达不出来)~~
  `换言之`，如果视频存在了1年,每天都有可查的历史弹幕,那应该查到 `1000*365 = 36.5万`弹幕,可是实际上弹幕并没有这么多啊..(~~结合上面的理解理解吧~~)
  
 =============================================================
