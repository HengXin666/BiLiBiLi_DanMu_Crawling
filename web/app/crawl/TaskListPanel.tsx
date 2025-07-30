"use client";

import React, { useState, useEffect, useRef } from "react";
import {
  Accordion,
  AccordionItem,
  Card,
  Button,
  ScrollShadow,
} from "@nextui-org/react";
import { BACKEND_URL } from "@/config/env";
import { TaskConfigModal, TaskConfig } from "./TaskConfigModal";
import { DateTime } from "luxon";
import { toast } from "sonner";
import { ExportXmlModal } from "./ExportXmlModal";
import { Hammer, Trash2 } from "lucide-react";

interface AllTaskData {
  mainTitle: string;
  tasks: TaskConfig[];
}

let taskIdCidMap: Map<string, string> = new Map<string, string>();
interface LogEntry {
  time: string;
  message: string;
}

type ServerMessage =
  | { type: "log"; data: string }
  | { type: "taskConfig"; data: TaskConfig };

export function TaskListPanel ({ refreshKey }: { refreshKey: number }) {
  const [taskList, setTaskList] = useState<AllTaskData[]>([]);
  const configIdIndexMap = useRef<Map<string, { groupIndex: number; taskIndex: number }>>(new Map());

  // 删除
  const [manageMode, setManageMode] = useState(false);
  const [checkedTaskSet, setCheckedTaskSet] = useState<Set<string>>(new Set());

  // 日志: configId -> msgList
  const [logMap, setLogMap] = useState<Record<string, LogEntry[]>>({});
  const [logOpenMap, setLogOpenMap] = useState<Record<string, boolean>>({});
  const [loadingConfig, setLoadingConfig] = useState(false);

  const [taskConfigData, setTaskConfigData] = useState<TaskConfig | null>(null);
  const [isConfigModalOpen, setIsConfigModalOpen] = useState(false);

  const wsMap = useRef<Record<number, WebSocket>>({});

  const [isExportModalOpen, setIsExportModalOpen] = useState(false);
  const [exportCid, setExportCid] = useState<number>(0);
  const [exportConfigId, setExportConfigId] = useState<string>("");
  const [exportFileName, setExportFileName] = useState<string>("");

  const toggleTaskSelect = (configId: string) => {
    setCheckedTaskSet(prev => {
      const newSet = new Set(prev);
      if (newSet.has(configId)) {
        newSet.delete(configId);
      } else {
        newSet.add(configId);
      }
      return newSet;
    });
  };

  const buildConfigIdIndexMap = (list: AllTaskData[]) => {
    const map = new Map<string, { groupIndex: number; taskIndex: number }>();

    list.forEach((group, groupIndex) => {
      group.tasks.forEach((task, taskIndex) => {
        map.set(task.configId, { groupIndex, taskIndex });
      });
    });

    configIdIndexMap.current = map;
  };

  const replaceTaskByConfigId = (newTask: TaskConfig) => {
    const ref = configIdIndexMap.current.get(newTask.configId);
    if (!ref) return;

    setTaskList((prev) => {
      const next = [...prev];
      const group = next[ref.groupIndex];
      const newTasks = [...group.tasks];

      newTasks[ref.taskIndex] = newTask;

      next[ref.groupIndex] = { ...group, tasks: newTasks };

      return next;
    });
  };

  const fetchTasks = async () => {
    const res = await fetch(`${BACKEND_URL}/allDm/getAllTaskData`);
    const json = await res.json();
    setTaskList(json.data);
    buildConfigIdIndexMap(json.data);
  };

  const handleOpenExport = (configId: string, cid: number, title: string) => {
    setExportConfigId(configId)
    setExportCid(cid);
    setExportFileName(title);
    setIsExportModalOpen(true);
  };

  const formatTimestamp = (ts: number) => {
    if (ts === 0) return "未开始";
    return DateTime.fromSeconds(ts).toFormat("yyyy-MM-dd HH:mm:ss");
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case "爬取历史弹幕":
      case "正在爬取实时弹幕":
        return "text-green-500";
      case "暂停爬取历史弹幕":
      case "暂停爬取实时弹幕":
        return "text-yellow-500";
      case "爬取历史弹幕完成":
        return "text-blue-500";
      case "爬取历史弹幕被封禁中":
      case "实时弹幕被封禁中":
        return "text-red-500";
      default:
        return "text-default-500";
    }
  };

  const appendLog = (configId: string, message: string) => {
    setLogMap((prev) => {
      const time = DateTime.local().toFormat("HH:mm:ss");
      const newLog: LogEntry = { time, message };
      const prevLogs = prev[configId] || [];
      return {
        ...prev,
        [configId]: [...prevLogs.slice(-49), newLog], // 保留最多50条
      };
    });
  };

  const handleGetAllRuningTask = async () => {
    const res = await fetch(`${BACKEND_URL}/allDm/getAllRuningTask`, {
      method: "POST"
    });

    const cidToTaskIdMap = (await res.json()).data;
    for (const [configId, _taskId] of Object.entries(cidToTaskIdMap)) {
      if (typeof _taskId !== "string") {
        // 什么垃圾后端?
        console.warn(`taskId 类型错误: ${configId} ->`, _taskId);
        continue; // 跳过无效数据
      }
      const taskId: string = _taskId;
      const cid: number = +(configId.split('-').pop() || 0);
      const ws = new WebSocket(
        `${BACKEND_URL.replace(/^http/, "ws")}/allDm/ws/${taskId}`
      );

      ws.onmessage = (event: MessageEvent<string>) => {
        try {
          const msg: ServerMessage = JSON.parse(event.data);
          
          if (msg.type === "log") {
            appendLog(configId, msg.data);
          } else if (msg.type === "taskConfig") {
            console.log("设置了啊");
            
            replaceTaskByConfigId(msg.data);
          } else {
            console.warn(`未知消息类型: ${msg}`);
          }
        } catch (e) {
          console.error("消息格式错误", e, event.data);
        }
      };

      ws.onerror = (e) => {
        console.error(`WebSocket error for cid ${configId}`, e);
      };

      ws.onclose = () => {
        console.warn(`WebSocket closed for cid ${configId}`);

        if (wsMap.current[cid]) {
          wsMap.current[cid].close();
          delete wsMap.current[cid];
        }
        if (taskIdCidMap.has(configId)) {
          taskIdCidMap.delete(configId);
        }
      };

      toast.info(`连接到: configId = ${configId}`)

      wsMap.current[cid] = ws;
      taskIdCidMap.set(configId, taskId);
    }
  };

  const handleStart = async (task: TaskConfig, mainTitle: string) => {
    if (taskIdCidMap.has(task.configId)) {
      toast.error("任务已经开始了, 不能再次启动任务; 可以刷新网页再尝试");
      return;
    }
    // 新建任务
    const res = await fetch(`${BACKEND_URL}/allDm/startTask`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      // StartTaskVo
      body: JSON.stringify({ configId: task.configId, cid: task.cid, path: `${mainTitle}/${task.title}` }),
    });

    // 记录 taskId
    const taskId = (await res.json()).data.taskId;
    taskIdCidMap.set(task.configId, taskId);

    // 连接到实时日志
    const ws = new WebSocket(
      `${BACKEND_URL.replace(/^http/, "ws")}/allDm/ws/${taskId}`
    );

    toast.info(`连接到: cid = ${task.cid}`)

    ws.onmessage = (event: MessageEvent<string>) => {
      try {
        const msg: ServerMessage = JSON.parse(event.data);
        
        if (msg.type === "log") {
          appendLog(task.configId, msg.data);
        } else if (msg.type === "taskConfig") {
          replaceTaskByConfigId(msg.data);
        } else {
          console.warn(`未知消息类型: ${msg}`);
        }

      } catch (e) {
        console.error("消息格式错误", e, event.data);
      }
    };

    ws.onerror = (e) => {
      console.error(`WebSocket error for cid ${task.cid}`, e);
    };

    ws.onclose = () => {
      console.warn(`WebSocket closed for cid ${task.cid}`);
      if (wsMap.current[task.cid]) {
        wsMap.current[task.cid].close();
        delete wsMap.current[task.cid];
      }
      if (taskIdCidMap.has(task.configId)) {
        taskIdCidMap.delete(task.configId);
      }
    };

    wsMap.current[task.cid] = ws;
  };

  const handleStop = async (task: TaskConfig) => {
    if (taskIdCidMap.get(task.configId) === undefined) {
      toast.error("任务并没有开始, 请刷新网页再尝试")
      return;
    }
    await fetch(`${BACKEND_URL}/allDm/stopTask/${taskIdCidMap.get(task.configId)}`, {
      method: "POST",
    });

    taskIdCidMap.delete(task.configId);

    // 等待服务器关闭 ws, 而不是本地, 否则会结束不到服务器 呜呼
    // if (wsMap.current[task.cid]) {
    //   wsMap.current[task.cid].close();
    //   delete wsMap.current[task.cid];
    // }

    toast.success("已暂停任务");

    fetchTasks();
  };

  const handleOpenConfig = async (configId: string, cid: number) => {
    setLoadingConfig(true);
    setIsConfigModalOpen(true);
    try {
      const res = await fetch(`${BACKEND_URL}/allDm/getTaskConfig/${configId}?cid=${cid}`);
      const json = await res.json();
      setTaskConfigData(json.data);
    } finally {
      setLoadingConfig(false);
    }
  };

  const handleSaveTaskConfig = async (newConfig: TaskConfig) => {
    await fetch(`${BACKEND_URL}/allDm/setTaskConfig`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(newConfig),
    });
    setIsConfigModalOpen(false);
    setTaskConfigData(null);
    fetchTasks();
  };

  const toggleLogOpen = (configId: string) => {
    setLogOpenMap((prev) => ({
      ...prev,
      [configId]: !prev[configId],
    }));
  };

  useEffect(() => {
    fetchTasks();
    handleGetAllRuningTask();
  }, [refreshKey]);

  return (
    <div className="space-y-4">
      <Accordion>
        {taskList.map((t) => (
          <AccordionItem key={t.mainTitle} title={t.mainTitle}>
            {t.tasks.map((task) => (
              <Card key={task.cid} className="p-4 mb-2">
                <div className="flex justify-between items-start">
                  <div className="text-left space-y-1 w-3/4">
                    <div>
                      <strong>爬取:</strong> {task.title} ({task.cid})
                    </div>
                    <div>
                      <strong>数据:</strong> {task.totalDanmaku} 条 | 神弹幕: {task.advancedDanmaku}
                    </div>
                    <div>
                      <strong>进度:</strong> {formatTimestamp(task.currentTime)} / {formatTimestamp(task.range[1])}
                    </div>
                    <div>
                      <strong>上次执行:</strong> {formatTimestamp(task.lastFetchTime)}
                    </div>
                    <div className="mt-2 border rounded p-2 bg-black text-green-400 text-xs font-mono max-h-40 overflow-y-auto cursor-pointer"
                      onClick={() => toggleLogOpen(task.configId)}>
                      {logOpenMap[task.configId]
                        ? (
                          <ScrollShadow hideScrollBar className="max-h-40">
                            {logMap[task.configId]?.map((log, idx) => (
                              <div key={idx}>
                                [{log.time}] {log.message}
                              </div>
                            )) || <div>暂无日志</div>}
                          </ScrollShadow>
                        ) : (
                          <div>
                            {logMap[task.configId]?.length
                              ? `[${logMap[task.configId][logMap[task.configId].length - 1].time}] ${logMap[task.configId][logMap[task.configId].length - 1].message}`
                              : "暂无日志 (点击展开)"}
                          </div>
                        )}
                    </div>
                  </div>

                  <div className="flex flex-col items-end space-y-1 w-1/4">
                    <div className={`font-bold ${getStatusColor(task.status)}`}>
                      {task.status} {" "}
                      {manageMode && (
                        <input
                          type="checkbox"
                          checked={checkedTaskSet.has(task.configId)}
                          onChange={() => toggleTaskSelect(task.configId)}
                        />
                      )}
                    </div>
                    <Button size="sm" color="primary" onPress={() => handleStart(task, t.mainTitle)}>
                      运行
                    </Button>
                    <Button size="sm" color="warning" onPress={() => handleStop(task)}>
                      暂停
                    </Button>
                    <Button size="sm" color="secondary" onPress={() => handleOpenConfig(task.configId, task.cid)}>
                      配置
                    </Button>
                    <Button size="sm" color="default" onPress={() => handleOpenExport(task.configId, task.cid, task.title)}>
                      导出弹幕
                    </Button>
                  </div>
                </div>
              </Card>
            ))}
          </AccordionItem>
        ))}
      </Accordion>

      {manageMode ? (
        <div className="fixed bottom-20 right-8 z-50 flex space-x-4">
          <Button
            color="danger"
            className="shadow-xl rounded-full px-6 py-4 text-base"
            startContent={<Trash2 size={20} />}
            size="sm"
            disabled={checkedTaskSet.size === 0}
            onPress={async () => {
              if (!confirm(`确定删除 ${checkedTaskSet.size} 个任务吗？`)) 
                return;

              await fetch(`${BACKEND_URL}/allDm/deleteTasks`, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ configIds: Array.from(checkedTaskSet) }),
              });

              setCheckedTaskSet(new Set());
              setManageMode(false);
              fetchTasks();
            }}
          >
            删除
          </Button>

          <Button
            color="default"
            className="shadow-xl rounded-full px-6 py-4 text-base"
            size="sm"
            onPress={() => {
              setCheckedTaskSet(new Set());
              setManageMode(false);
            }}
          >
            取消
          </Button>
        </div>
      ) : (
        <Button
          color="secondary"
          className="fixed bottom-20 right-8 z-50 shadow-xl rounded-full px-6 py-4 text-base"
          size="sm"
          onPress={() => setManageMode(true)}
          startContent={<Hammer size={20} />}
        >
          管理任务
        </Button>
      )}

      <ExportXmlModal
        isOpen={isExportModalOpen}
        onClose={() => setIsExportModalOpen(false)}
        configId={exportConfigId}
        cid={exportCid}
        defaultFileName={exportFileName}
      />

      <TaskConfigModal
        key={taskConfigData?.cid || 0}
        isOpen={isConfigModalOpen}
        loading={loadingConfig}
        configData={taskConfigData}
        onClose={() => {
          setIsConfigModalOpen(false);
          setTaskConfigData(null);
        }}
        onSave={handleSaveTaskConfig}
      />
    </div>
  );
}