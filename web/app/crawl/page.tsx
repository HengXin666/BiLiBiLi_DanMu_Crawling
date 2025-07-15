"use client";

import { title } from "@/components/primitives";
import { useEffect, useState } from "react";
import { TaskAddPanel } from "./TaskAddPanel";
import { TaskListPanel } from "./TaskListPanel";

/**
 * 需求:
 * 1. 添加爬虫任务:
 *  输入: 
 *    1. 任务名称(可以为中文, 但是不能是路径禁止的字符, 如/*.?)
 *    2. url 或者直接输入 cid
 * 2. 如果输入 url 则获取一个爬取列表 (通过 /allDm/getVideoPartList)
 *    @allDmReqController.post("/getVideoPartList", response_class=ResponseModel[VideoPartVo])
 *    def getVideoPartList(url: str):
 *    列表为:
 *      class VideoPartVo(BaseModel):
 *        cidList: List[VideoPart]
 *    其中:
 *    @dataclass
 *    class VideoPart:
 *        """视频分P信息"""
 *        cid: int   # 视频 cid
 *        page: int  # 分P编号
 *        part: str  # 标题
 *    以列表 page | part | cid 展示
 * 3. 获取列表或者cid可以选择 那些是真正需要爬取的, 即每一项前面可以勾选 (默认是全选)
 * 4. 确定后, 依次发送:
 *    class VidoPartConfig(BaseModel):
        path: str (是 ${任务名称}/${part})
        data: VideoPart
    给 /allDm/initTaskConfig
 *
 * 然后它们就成为爬取界面的一项 (以用户定义的标题 展示)
 *    然后它的子项就是列表, 每一项是:
 *  @dataclass
    class TaskConfig:
        """
        弹幕爬取任务配置数据类
        
        Attributes:
            cid (int): 视频唯一标识符
            title (str): 爬取的视频标题
            lastFetchTime (int): 上次爬取时间, Unix 时间戳(秒)
            range (Tuple[int, int]): 爬取时间范围(左边界, 右边界), Unix 时间戳(秒)
            currentTime (int): 当前爬取的时间点, Unix 时间戳(秒)
            totalDanmaku (int): 爬取到的弹幕总数
            advancedDanmaku (int): 爬取到的高级弹幕(如代码弹幕、Bas弹幕)数
            status (FetchStatus): 当前爬取状态
        """
        cid: int
        title: str
        lastFetchTime: int
        range: Tuple[int, int]
        currentTime: int
        totalDanmaku: int
        advancedDanmaku: int
        status: FetchStatus
  
 * 其中, 通过 /allDm/getAllTaskData 获取, 返回的是List[AllTaskData], 其中
        class AllTaskData:
          # 标题
          mainTitle: str
          # 子任务
          tasks: List[TaskConfig]
  *
  * 并且展示这些项,
  * 并且根据状态, 我们还可以 开始/暂停/结束 任务
  * 
  * 开始任务:
  *  @allDmReqController.post("/startTask", response_model=ResponseModel[dict])
      async def startTask(startTask: StartTaskVo):

    其中
    class StartTaskVo(BaseModel):
      cid: int
      path: str

    结束任务:
    @allDmReqController.post("/stopTask", response_model=ResponseModel[None])
    async def stopTask(taskId: str):
        try:
            allDmReqManager._taskData[taskId].isRun = False
            return ResponseModel.success()
        except:
            return ResponseModel.error(msg="taskId does not exist")

    监听任务(ws):
    @allDmReqController.websocket("/ws/{taskId}")
    async def taskState(ws: WebSocket, taskId: str):
        await ws.accept()
        allDmReqManager._clients[taskId].add(ws)
        try:
            while True:
                await asyncio.sleep(60 * 60) # 保持连接, 心跳可选
        except WebSocketDisconnect:
            allDmReqManager._clients[taskId].remove(ws)
 */

export default function CrawlPage() {
  const [refreshKey, setRefreshKey] = useState(0);

  // useEffect(() => {
  //   const ws = new WebSocket(`ws://localhost:28299/allDm/ws/${taskId}`);
  //   ws.onmessage = (e) => {
  //     // 更新状态，实时显示
  //   };
  //   return () => ws.close();
  // }, [taskId]);

  return (
    <div className="space-y-4">
      <h1 className={title()}>弹幕爬虫 | 任务管理</h1>
      <TaskAddPanel onSuccess={() => setRefreshKey((k) => k + 1)} />
      <TaskListPanel key={refreshKey} />
    </div>
  );
}
