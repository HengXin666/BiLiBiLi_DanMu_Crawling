#!/usr/bin/python
# -*- coding: UTF-8 -*-
from __future__ import print_function
from rich import print #一个颜色库
# 会使得进度条出错...
import traceback
import re
import os
# import json
import random #随机数
import time
import configparser
import requests
import bilidm_pb2 #解析弹幕文件的
import sys

#不知道为什么 加这个就不报错? #依旧报错...不知道有什么用...复制粘贴的...
os.environ['NO_PROXY'] = 'stackoverflow.com'

def jdt():
    #推迟调用线程的运行，secs指秒数
    daily_ =daily
    while daily_ >= 1:
        dd = f'请等待{daily_}秒.'
        print(dd,end='')
        time.sleep(1)
        daily_-=1

max_get_dm = 365
ip_dl = 0
www = ['baidu','sogou','tieba.baidu','pan.baidu','example'] #用于测试代理ip的可用性
ip_proxy_zs = ''
ip_get_cs = -1 #已经使用ip的次数
def ip_proxy(x):#从代理ip池中获取代理ip
        global ip_get_cs
        while True:
            def get_proxy():
                return requests.get("http://127.0.0.1:6666/get/").json()
                            #端口是cmd 的那个
            def delete_proxy(proxy):
                requests.get("http://127.0.0.1:6666/delete/?proxy={}".format(proxy))
                            #端口是cmd 的那个                                       
            def getHtml():
                retry_count = 2 #测试次数
                proxy = get_proxy().get("proxy")
                while retry_count > 0:
                    try:
                        html = requests.get(f'http://www.{random.choice(www)}.com', proxies={"http": "http://{}".format(proxy)},timeout=3)
                        # 使用代理访问+超时处理
                        return proxy #返回代理ip
                    except Exception:
                        print('重新尝试:',proxy)
                        retry_count -= 1
                # 删除代理池中代理
                delete_proxy(proxy)
                print('删除失效ip',proxy)
                return
            #---------------上面是判断ip是否可用
            global ip_proxy_zs
            if x == 'lj':
                print('删除代理:',ip_proxy_zs)
                delete_proxy(ip_proxy_zs)
                ip_get_cs = get_ip_max_cs
            if ip_get_cs >= get_ip_max_cs or ip_get_cs == -1:
                ip_proxy_ = getHtml()
                if ip_proxy_ != None:
                    show_info('成功选择了可用代理ip: '+ip_proxy_)
                    ip_get_cs = 1
                    ip_proxy_zs = ip_proxy_
                    return ip_proxy_
                print('没有ip代理可供使用,正在重新尝试...')
            else:
                ip_get_cs += 1
                print(f'延续ip代理{ip_proxy_zs}')
                return ip_proxy_zs #暂时

def app_title():
    print(f'''
    ================================================================
    [red]欢迎使用本程序! 本程序仅供学习与交流,由于不当操作产生的问题,责任自负![/red]
    ================================================================

    * --> [1].获取 B站某视频的cid

    * --> [2].设置 ip代理 (1为开启) 当前为: {ip_dl}
               |   当前抓取次数为: {max_get_dm}  (为0则停止爬取)

    * --> [3].[green]爬取[/green] B站某视频的所有历史弹幕

    * --> [4].[yellow]接着[/yellow] 爬取 (中断后会产生保存文件)

    * --> [5].[red]关于 (从使用开始时,即代表同意)[/red]

    * --> [0].退出 程序

    ================================================================
    \t\t\t作者: [yellow]Heng_Xin[/yellow]

    \t\t\t版本号: [yellow]V 2.7.1[/yellow]
    =================================================================
    ''')

def gy():
    print('''
    =================================================================
    \t\t\t[red]关\t于[/red]
    =================================================================

    [red][!] 请勿用于商业用途,若造成不良后果，责任自负![/red]

    [?] 代码写得很烂? 没有使用多线程等 某些时候效率可能很低? 请见谅qwq..

    [?] #你行你上!\t牢饭包吃包住...

    =================================================================

# \t参考> https://github.com/SocialSisterYi/bilibili-API-collect

# \t大量大量修改(相当于只是使用了其api?)

# \t[yellow]【原作者: zjkwdy】 https://github.com/zjkwdy/bilibili-history-danmu-spider[/yellow]

    =====================[red]更[/red]================[red]新[/red]==========================

#     |01#修复: 爬取某些高级弹幕时 BiliLocal无法识别此弹幕的问题
#     |02#调整: 爬取时如果失败 会无限重复尝试爬取(!不然怎么叫全弹幕?,(我不希望弹幕丢失))
#     |03#调整: 原来是用xml模块输出文件的，我改为open输出(某些错误无法识别)
#     |04#调整: 去除了log日志文件的输出
#     |05#调整: 使用代理ip池时 不设置延迟!$$$ 2.0.1
#     |06#调整: 更改代理思路:不报错 = 继续使用! 2.0.2
#     |07#调整: 发现一个ip请求30次频繁后会被封.此后程序无法通过更改ip躲避封禁? 2.0.3
#     |08#调整: 请求日期 5/ip 2.0.4
#     |09#调整: 请求还是得小,得慢,不然会被封SESSDATA?(06崩坏...) 2.1.0 #一次封我6小时...
#     |10#调整: 优化获取日期方案!(如果爬取本年,则不会爬取未来) 2.1.1
#     --|爬取提醒: 《电磁炮真是太可爱了》是历史弹幕到2010.10.2 #?视频时间是2011的(雾=-=)
#     --|         《【炮姐_AMV】我永远都会守护在你的身边！》(Av810872,P1)的弹幕只能看到2017年后的qwq...
#     |11#调整: 现在每次爬取会生成文件缓存 获取的有弹幕的天数 2.1.2
#     |12#调整: 采用反馈输出! 即边爬边输出(反正有频率限制,不如利用?)
#     --|           输出+去除弹幕缓存日期 2.2.0
#     |13#调整: 输出弹幕诺成功,则延用ip 2.2.1 反正等待再换ip 2.2.2
#     |14#调整: 增加部分ui显示.. 2.2.3
#     --| 封爬日期ip 但不影响爬弹幕内容 ? q^q..
#     |15#调整: 修改ui 删去无用代码
#     |16#调整: 修复了爬取完弹幕不删除缓存弹幕文件天数的BUG(主要是没写:D)
#     |17#调整: 优化无代理爬取 & 2s延迟 可爬365天 不封?(一般情况) 2.5.1
#     |18#调整: 修复了一些bug , 删除无用代码 2.6.3
#     |19#调整: 添加了一些颜色美化 2.6.6 [yellow]并且打包输出.exe文件[/yellow]
#     |20#调整: 颜色美化会使得进度条出现BUG..
#
#     |[red]存在问题[/red]:
#      [!]使用ip代理爬取弹幕还是 400 ssl证书错误 (无法连接到代理'导致，OSError'隧道连接失败:不允许405方法')... 
#        各种各样的错误出现在同一代码.. 有时候(尝试n次后)又会成功... (应该是被封了qwq)

    =================================================================

    \t\t\t By: [yellow]Heng_Xin[/yellow]

    \t[!] 本项目github: https://github.com/HengXin666/BiLiBiLi_DanMu_Crawling
    \t[!] BiLibili主页: https://space.bilibili.com/478917126

    \t[red]【\t请\t勿\t用\t此\t谋\t利\t】[/red]

    =================================================================
''')
def settin():#设置
    print('是否[yellow]使用代理ip[/yellow](0否/1是)')
    xz_ = xz(0,1)
    return xz_

def get_cid():
    bvid=input('请输入BV号(如BV1e5411R7e7):')
    #浏览器Cookie SESSDATA，一般不用写，但是比如av2(BV1xx411c7mD)没登陆看不了就得写。
    SESSDATA = ''
    url=f'http://api.bilibili.com/x/web-interface/view?bvid={bvid}'
    req_headers={
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.150 Safari/537.36 Edg/88.0.705.63',
        'Cookie': f'SESSDATA={SESSDATA};'
    }
    req=requests.get(url,headers=req_headers)
    for video in req.json()['data']['pages']:
        print('======================================================================')
        print('视频 P'+str(video['page'])+' '+video['part']+' 的cid是 [green]'+str(video['cid']))
    input('请按回车继续...')

# 返回开始到结束年中的所有月数组，例如['2011-01', '2011-02', '2011-03', '2011-04', '2011-05', '2011-06', '2011-07', '2011-08', '2011-09', '2011-10', '2011-11', '2011-12', '2012-01', '2012-02', '2012-03', '2012-04', '2012-05', '2012-06', '2012-07', '2012-08', '2012-09', '2012-10', '2012-11', '2012-12']
def list_months(start, end):#优化方案!(如果爬取本年,则不会爬取未来)
    result = []
    for year in range(start, end+1):
            if str(year) != time.strftime("%Y"):
                for month in range(1, 13):
                    result.append(str(year)+'-'+str(month).rjust(2, '0'))
            else:
                for month in range(1,int(time.strftime("%m"))+1):
                    result.append(str(year)+'-'+str(month).rjust(2, '0'))
    return result


# 获取某年某月有弹幕的天，返回天数组。
def get_danmu_months(cid, month, SESSDATA):
    api_url = f'http://api.bilibili.com/x/v2/dm/history/index?type=1&oid={str(cid)}&month={month}'
    req_headers = {
        'User-Agent': random_user_agent(),
        'Cookie': 'SESSDATA='+random_SESSDATA(SESSDATA)+';'
    }
    # 如果有代理就随机一个用
    if ip_dl == 1:
    # print(proxy,type(proxy))
        req = requests.get(url=api_url, headers=req_headers,proxies={'https':ip_proxy(0)})
    #原作者参数用错了 是proxies= 不是proxy= ,我都怀疑我的代码半天了都! 
    else:
        req = requests.get(url=api_url, headers=req_headers)
    return req.json()


# 获取所有有弹幕的天
def get_danmu_dates(cid, months, SESSDATA):
    global ip_dl, max_get_dm #声明为全局变量
    result = []
    req_cs = 1
    for month in months:
        dates = get_danmu_months(cid, month, SESSDATA)
        req_cs += 1
        if req_cs % 10 == 0:
            jdt()
        cs = 1
        while True:
            if dates['code'] == 0:
                if dates['data'] != None:
                    for date in dates['data']:
                        result.append(date)
                        show_info(f'{date} 有弹幕！')
                else:
                    show_info(f'{month}：啥弹幕也木有嘞！(＞︿＜)')
                if ip_dl == 0:
                    jdt()
                break
            else:
                show_error(f'{dates},当前错误第{cs}次出现,正在尝试!')
                if ip_dl == 1:
                    ip_proxy('lj')
                if cs%10 == 0:#诺失败次数是10的倍数
                    a = input('失败次数过多!请修改 mac地址 或者 ip地址 或者 等待几个小时 再尝试!\n(输入ip设置网络|回车继续尝试!):')
                    if a == 'ip':
                        ip_dl = settin()
                if ip_dl == 0:
                    jdt()
                cs+=1
    return result

#增加重试连接次数
#request的连接数过多而导致Max retries exceeded
#在header中不使用持久连接
requests.adapters.DEFAULT_RETRIES = 5
s = requests.session()
s.keep_alive = False

#爬取很多时,诺被屏蔽ip 希望下次接着输出 , どうしよ?
def get_day_danmu(cid, date, SESSDATA):# 获取某天的历史弹幕!
    global ip_dl , get_danmu_fail_cs , get_danmu_success_cs , max_get_dm #声明为全局变量
    show_info(f'[yellow]正在处理: {date} . | [成功]: {get_danmu_success_cs} , [尝试]: {get_danmu_fail_cs} | 还可请求: {max_get_dm} 次.[/yellow]')
    api_url = f'https://api.bilibili.com/x/v2/dm/web/history/seg.so?type=1&oid={cid}&date={date}'
    req_headers = {
        'User-Agent': random_user_agent(),
        'Cookie': 'SESSDATA='+random_SESSDATA(SESSDATA)+';',
        'Referer': 'https://www.bilibili.com/',
        'Origin': 'https://www.bilibili.com'
    }
    cs = 1 #次数
    while True:
        #下载protobuf格式弹幕，有代理就整一个！
        if ip_dl == 1:#启动ip代理
            get_danmu_present_fail_cs = 1 #当前日期失败次数
            while True:
                try:
                    data = requests.get(url=api_url, headers=req_headers,proxies={'https': "http://{}".format(ip_proxy(0))}, timeout=3, verify=False) 
                    #verify是否验证服务器的SSL证书
                    break
                except:
                    traceback.print_exc()#输出错误位置
                    global ip_get_cs
                    get_danmu_fail_cs += 1
                    show_info(f'第 {get_danmu_present_fail_cs} 次尝试重新处理: {date} . | [成功]:{get_danmu_success_cs} , [尝试]:{get_danmu_fail_cs} | 还可请求: {max_get_dm} 次.')
                    get_danmu_present_fail_cs += 1
                    ip_get_cs = -1
                    # ip是没问题的啊！
        else:
            data = requests.get(api_url, headers=req_headers)
            jdt()
        try:
            target = bilidm_pb2.DmSegMobileReply()
            target.ParseFromString(data.content)
            show_info(f'{date}处理完成.')
            get_danmu_success_cs += 1
            return target.elems
        except:
            show_error(f'处理弹幕出错:{date}:{data.json()},当前错误第{cs}次出现,正在尝试!')
            if cs%10 == 0:#诺失败次数是10的倍数
                a = input('失败次数过多!请修改 mac地址 或者 ip地址 或者 等待几个小时 再尝试!\n(输入ip设置网络|0退出|回车继续尝试!):')
                if a == 'ip':
                    ip_dl = settin()
                if a == '0':#退出
                    return 'null'
            cs+=1


# 随机UA标
def random_user_agent():#用于伪装h...
    # 复制的，不知道还有用没
    USER_AGENTS = [
        "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; "
        "SV1; AcooBrowser; .NET CLR 1.1.4322; .NET CLR 2.0.50727)",
        "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0; Acoo Browser; "
        "SLCC1; .NET CLR 2.0.50727; Media Center PC 5.0; .NET CLR 3.0.04506)",
        "Mozilla/4.0 (compatible; MSIE 7.0; AOL 9.5; AOLBuild 4337.35; "
        "Windows NT 5.1; .NET CLR 1.1.4322; .NET CLR 2.0.50727)",
        "Mozilla/5.0 (Windows; U; MSIE 9.0; Windows NT 9.0; en-US)",
        "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Win64; x64; "
        "Trident/5.0; .NET CLR 3.5.30729; .NET CLR 3.0.30729; "
        ".NET CLR 2.0.50727; Media Center PC 6.0)",
        "Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 6.0; "
        "Trident/4.0; WOW64; Trident/4.0; SLCC2; .NET CLR 2.0.50727; "
        ".NET CLR 3.5.30729; .NET CLR 3.0.30729; "
        ".NET CLR 1.0.3705; .NET CLR 1.1.4322)",
        "Mozilla/4.0 (compatible; MSIE 7.0b; "
        "Windows NT 5.2; .NET CLR 1.1.4322; .NET CLR 2.0.50727; "
        "InfoPath.2; .NET CLR 3.0.04506.30)",
        "Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN) "
        "AppleWebKit/523.15 (KHTML, like Gecko, Safari/419.3) "
        "Arora/0.3 (Change: 287 c9dfb30)",
        "Mozilla/5.0 (X11; U; Linux; en-US) AppleWebKit/527+ "
        "(KHTML, like Gecko, Safari/419.3) Arora/0.6",
        "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; "
        "rv:1.8.1.2pre) Gecko/20070215 K-Ninja/2.1.1",
        "Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9) "
        "Gecko/20080705 Firefox/3.0 Kapiko/3.0",
        "Mozilla/5.0 (X11; Linux i686; U;) "
        "Gecko/20070322 Kazehakase/0.4.5",
        "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.0.8) "
        "Gecko Fedora/1.9.0.8-1.fc10 Kazehakase/0.5.6",
        "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/535.11 "
        "(KHTML, like Gecko) Chrome/17.0.963.56 Safari/535.11",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_3) "
        "AppleWebKit/535.20 (KHTML, like Gecko) Chrome/19.0.1036.7 Safari/535.20",
        "Opera/9.80 (Macintosh; Intel Mac OS X 10.6.8; U; fr) "
        "Presto/2.9.168 Version/11.52",
    ]
    result = random.choice(USER_AGENTS)#直接从列表随机选择
    return result


# 随机SESSDATA，多个SESSDATA时很有用
def random_SESSDATA(SESSDATA):
    result = random.choice(SESSDATA)#直接从列表随机选择
    return result

def show_info(message):
    print(f'[green][{time.strftime("%H:%M:%S")}]:{message}[/green]')
def show_error(message):
    print(f'[red]【错误】:[{time.strftime("%H:%M:%S")}]:{message}[/red]')

#额外
def search():#初始化工作区
    try:
        config.read('HX配置文件.ini', encoding='utf-8')
    except:
        show_error('找不到文件 [yellow]HX配置文件.ini[/yellow]')
        re_ll_wj = re.compile(r'^(.:\\)')
        dq = os.getcwd()
        print("当前工作目录 : %s" % dq)
        path = re.findall(re_ll_wj,dq)[0]
        name = 'HX配置文件.ini' #填写文件名
        for root, dirs, files in os.walk(path):  # path 为根目录
        #root-路径 dirs-文件夹 files-文件
            if name in dirs or name in files:
                #flag = 1      判断是否找到文件
                root = str(root)
                os.chdir(os.path.join(root))
                print('【成功找到文件!】初始化工作目录成功!\n当前工作目录:',os.getcwd())
                return
        print('【错误】:找不到文件!')
        return

def xz(m,x):#选择函数2.0
    while True:
        try:
            print('请输入',m,'~',x,'之间的数字.')
            xz=int(input('请选择:'))
            if m<=xz<=x:
                return xz
            else:
                print('请输入一个有效的数字!')
        except:
            print('错误数值!请按要求输入.')

if __name__ == '__main__':
    search()
    # 代码开始
    while True:
        app_title()
        xz_ = xz(0,5)
        if xz_ == 1:
            get_cid()
        elif xz_ == 2:
            ip_dl = settin()
            if ip_dl == 0:
                print(' [!]: 请输入抓取的年数,防封?')
                max_get_dm = xz(1,16)*356
        elif xz_ == 5:#关于
            gy()
            input('\n请按回车继续...')
        elif xz_ == 0:
            print('正在退出程序!')
            exit(code=1)
        else:#3 and 4走
            break
    config = configparser.RawConfigParser()
    try:#获取信息从配置文件中获取
        config.read('HX配置文件.ini', encoding='utf-8')
        SESSDATA = str(config.items('account')[0][1]).split(',')
        cid = int(config.items('spider')[0][1])
        start_year = int(config.items('spider')[1][1])
        end_year = int(config.items('spider')[2][1])
        daily = int(config.items('spider')[3][1])
    except:
        show_error('兄啊你的配置文件有问题啊！ | 请确保配置文件: [ HX配置文件.ini ] 存在(不能改文件名!!!)')
        input('')
        exit(code=1)
    if xz_ == 3:
        get_ip_max_cs = 5 #每过x次请求换一个ip 可谓是最优结果?
        months = list_months(start_year, end_year)
        wj_cs_name = f'缓存{cid}弹幕天数文件'
        show_info('「开始获取历史弹幕日期...时间较长耐心等待」だって')
        print(f'请输入保存获取弹幕天数弹幕的文件名:\n当前文件名为:[ {wj_cs_name} ]\n为空 则使用当前文件名!')
        wj_cs_name = input('请输入文件名:')+'.txt'
        if wj_cs_name =='':
            wj_cs_name = f'缓存{cid}弹幕天数文件'
        all_danmu_dates = get_danmu_dates(cid, months, SESSDATA)
        #保存爬取的日期!
        show_info(f'总共有{len(all_danmu_dates)}天的弹幕')
        not_ok_day_danmu = all_danmu_dates #存放没有处理的天的弹幕的天数
        for i in all_danmu_dates:
            with open(f'{wj_cs_name}','a+',encoding='utf-8') as wj:
                wj.write(str(i)+'\n')
        show_info(f'\t文件保存为【{wj_cs_name}】\n于路径:【 {os.getcwd()} 】中\n可以尝试等待几个小时,然后使用本程序的功能[4]接着爬取!\n([!]到时候请将本文件名输入至本程序)')
        print('\t-------------------------------')
        time.sleep(5)
        show_info('「获取所有历史弹幕日期完成，开始扒取历史弹幕」だって')
    else:
        show_info('「接着重新扒取历史弹幕」だって')
        #获取文件:
        while True:
            wj_cs_name = input('请手动输入文件名(包含.txt后缀)|输入0退出:')
            if wj_cs_name == '0':
                exit(code=1)
            try:
                with open(f'{wj_cs_name}','r',encoding='utf-8') as wj:
                    s = wj.readlines()
                all_danmu_dates = []
                for i in s:
                    i = i.strip('\n')#分割后是列表?
                    all_danmu_dates.append(i)
                print('')
                show_info(f'成功加载有弹幕的天数:【 {len(all_danmu_dates)} 】天')
                break
            except:
                show_error(f'文件:{wj_cs_name},请保证输入的正确性,并且在目录:【 {os.getcwd()} 】中')

    danmu_wj_name = f'{cid}-{start_year}-{end_year}.xml'
    print(f'当前要爬取的视频 cid 为 {cid}')
    print(f'请输入保存弹幕的文件名:\n当前文件名为:[ {danmu_wj_name} ]\n为空 则使用当前文件名!')
    danmu_wj_name = input('请输入文件名:')
    if danmu_wj_name =='':
        danmu_wj_name = f'{cid}-{start_year}-{end_year}.xml'
    
    # 初始化弹幕列表
    danmu_list_all = [] #所有弹幕!
    ok_day_danmu = []#存放处理好的弹幕天数
    danmu_id_list = []# 发现有重复弹幕，于是拿来了这个 #不可每次重置
    
    get_ip_max_cs = 5 #请求次数/ip
    get_danmu_fail_cs = 0 #获取弹幕总失败次数
    get_danmu_success_cs = 0  #获取弹幕总成功次数
    try:
        with open(f'{danmu_wj_name}.xml','x',encoding='UTF-8') as danmu_wj:#文件已存在会报错
                danmu_wj.write(f'<?xml version="1.0" encoding="UTF-8"?><i><chatserver>chat.bilibili.com</chatserver><chatid>{cid}</chatid><mission>0</mission><maxlimit>3000</maxlimit></i>\n')
                danmu_wj.close()
        print(f'「成功创建文件:[ {danmu_wj_name} ]输出xml格式弹幕文件」だって')
    except:
        print(f'[ {danmu_wj_name}.xml ]文件已存在!')

    # 下载所有历史弹幕
    #re表达式
    re_ll_gjdm_t = re.compile(r'^(\[)\d+?,')#判断是否为高级弹幕 头部
    re_ll_gjdm_w = re.compile(r'",\d(])$')#尾部
    bug_gjdm = 0
    #
    ip_get_cs = -1
    for date in all_danmu_dates:
        if max_get_dm >= 0:
            max_get_dm -= 1
        else:
            show_error(' 抓取次数已用尽! | 防止封禁请等待一段时间.')
            print(' [!]: 请输入抓取的天数,防封? | (等待一段时间 / 无视警告.)')
            max_get_dm = xz(1,3650)
        try:
            history_danmu = get_day_danmu(cid, date, SESSDATA)

            with open(f'{wj_cs_name}','r+',encoding='utf-8')as wj:#边输出边删除缓存日期
                a = wj.readlines()
            a.pop(0)
            wj = open(f'{wj_cs_name}','w',encoding='utf-8')
            for i in a:
                wj.writelines(i)
            wj.close()

            if history_danmu == 'null':#退出保存标识
                show_info(f'总共有{len(all_danmu_dates)}天的弹幕')
                not_ok_day_danmu = all_danmu_dates #存放没有处理的天的弹幕的天数
                for day in ok_day_danmu:
                    not_ok_day_danmu.remove(day)
                show_info(f'剩余{len(not_ok_day_danmu)}天的弹幕,未完成！')
                with open(f'{wj_cs_name}','w+',encoding='utf-8') as wj:
                    pass
                for i in not_ok_day_danmu:
                    with open(f'{wj_cs_name}','a+',encoding='utf-8') as wj:
                        wj.write(str(i)+'\n')
                show_info(f'文件保存为【{wj_cs_name}】\n于路径:【 {os.getcwd()} 】中\n可以尝试等待几个小时,然后使用本程序的功能[4]接着爬取!\n([!]到时候请将本文件名输入至本程序)\n')
                input('回车退出!')
                exit(code=1)
            else:
                ok_day_danmu.append(date)#记录已经处理好的天数
            danmu_list_all.append(history_danmu)#将每一天弹幕储存起来,日后跟踪
            
        except:
            traceback.print_exc()#输出错误位置
            show_error('请求过于频繁!')
    #for day_danmu in danmu_list_all: #总的拿出每一天
        try:
                # danmu_id_list = [] #初始化弹幕id列表xxx
                for danmu in history_danmu:#理论是可以防止内存地址重复
                        # 弹幕id入库防止重复 | 确实会重复!
                    if danmu.id not in danmu_id_list:
                        danmu_id_list.append(danmu.id)
                            # 每条弹幕
                        content = danmu.content
                        danmu_time_str = f'{danmu.progress}'
                        danmu_time = int(danmu_time_str)/1000
                        #判断高级弹幕-出现BiliLocal无法识别的高级弹幕会企图修正!
                        dm_t = re.findall(re_ll_gjdm_t,danmu.content)
                        dm_w = re.findall(re_ll_gjdm_w,danmu.content)
                        if '[' in dm_t and']' in dm_w:
                            danmu_nr = re.sub(r'(\\")','',danmu.content)
                            show_info(f'发现第{bug_gjdm}条异常高级弹幕,企图修正!{danmu.content}')
                            bug_gjdm+=1
                        else:
                            danmu_nr = danmu.content
                        danmu_sc_nr = f'<d p="{danmu_time},{danmu.mode},{danmu.fontsize},{danmu.color},{danmu.ctime},{danmu.pool},{danmu.midHash},{danmu.idStr}">{danmu_nr}</d>\n'
                        with open(f'{danmu_wj_name}.xml','a+',encoding='UTF-8') as danmu_wj:
                            danmu_wj.write(danmu_sc_nr)
                            danmu_wj.close()
                        show_info('输出弹幕:'+content)
                    else:
                        show_info(f'api输出了重复弹幕：{danmu.content}')
                show_info(f'目前输出了{len(danmu_id_list)}条弹幕 获取了: {len(history_danmu)} 条弹幕.')
        except:
            show_error('输出文件中出问题！！可能少一些弹幕。')
    show_info(f'正在删除缓存弹幕文件... [ {wj_cs_name} ]')
    os.remove(wj_cs_name)
    show_info(f'保存xml弹幕成功! 保存为 [ {danmu_wj_name}.xml ] ,本次输出了{len(danmu_id_list)}条弹幕，企图修正{bug_gjdm}条高级弹幕。')