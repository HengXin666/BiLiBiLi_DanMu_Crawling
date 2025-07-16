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
import { ExportXmlModal } from "./ExportXmlModal";

interface AllTaskData {
  mainTitle: string;
  tasks: TaskConfig[];
}

let taskIdCidMap: Map<number, string> = new Map<number, string>();
interface LogEntry {
  time: string;
  message: string;
}

export function TaskListPanel ({ refreshKey }: { refreshKey: number }) {
  const [taskList, setTaskList] = useState<AllTaskData[]>([]);
  const [logMap, setLogMap] = useState<Record<number, LogEntry[]>>({});
  const [logOpenMap, setLogOpenMap] = useState<Record<number, boolean>>({});
  const [loadingConfig, setLoadingConfig] = useState(false);

  const [taskConfigData, setTaskConfigData] = useState<TaskConfig | null>(null);
  const [isConfigModalOpen, setIsConfigModalOpen] = useState(false);

  const wsMap = useRef<Record<number, WebSocket>>({});

  const [isExportModalOpen, setIsExportModalOpen] = useState(false);
  const [exportCid, setExportCid] = useState<number>(0);
  const [exportFileName, setExportFileName] = useState<string>("");

  const handleOpenExport = (cid: number, title: string) => {
    setExportCid(cid);
    setExportFileName(title);
    setIsExportModalOpen(true);
  };

  const fetchTasks = async () => {
    const res = await fetch(`${BACKEND_URL}/allDm/getAllTaskData`);
    const json = await res.json();
    setTaskList(json.data);
  };

  useEffect(() => {
    fetchTasks();
  }, [refreshKey]);

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

  const appendLog = (cid: number, message: string) => {
    setLogMap((prev) => {
      const time = DateTime.local().toFormat("HH:mm:ss");
      const newLog: LogEntry = { time, message };
      const prevLogs = prev[cid] || [];
      return {
        ...prev,
        [cid]: [...prevLogs.slice(-49), newLog], // 保留最多50条
      };
    });
  };

  const handleStart = async (task: TaskConfig, mainTitle: string) => {
    const res = await fetch(`${BACKEND_URL}/allDm/startTask`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ cid: task.cid, path: `${mainTitle}/${task.title}` }),
    });

    const taskId = (await res.json()).data.taskId;
    taskIdCidMap.set(task.cid, taskId);

    const ws = new WebSocket(
      `${BACKEND_URL.replace(/^http/, "ws")}/allDm/ws/${taskId}`
    );

    ws.onmessage = (event) => {
      appendLog(task.cid, event.data);
    };

    wsMap.current[task.cid] = ws;

    fetchTasks();
  };

  const handleStop = async (task: TaskConfig) => {
    await fetch(`${BACKEND_URL}/allDm/stopTask`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(taskIdCidMap.get(task.cid)),
    });

    taskIdCidMap.delete(task.cid);

    if (wsMap.current[task.cid]) {
      wsMap.current[task.cid].close();
      delete wsMap.current[task.cid];
    }

    fetchTasks();
  };

  const handleOpenConfig = async (cid: number) => {
    setLoadingConfig(true);
    setIsConfigModalOpen(true);
    try {
      const res = await fetch(`${BACKEND_URL}/allDm/getTaskConfig/${cid}`);
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

  const toggleLogOpen = (cid: number) => {
    setLogOpenMap((prev) => ({
      ...prev,
      [cid]: !prev[cid],
    }));
  };

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
                      onClick={() => toggleLogOpen(task.cid)}>
                      {logOpenMap[task.cid]
                        ? (
                          <ScrollShadow hideScrollBar className="max-h-40">
                            {logMap[task.cid]?.map((log, idx) => (
                              <div key={idx}>
                                [{log.time}] {log.message}
                              </div>
                            )) || <div>暂无日志</div>}
                          </ScrollShadow>
                        ) : (
                          <div>
                            {logMap[task.cid]?.length
                              ? `[${logMap[task.cid][logMap[task.cid].length - 1].time}] ${logMap[task.cid][logMap[task.cid].length - 1].message}`
                              : "暂无日志 (点击展开)"}
                          </div>
                        )}
                    </div>
                  </div>

                  <div className="flex flex-col items-end space-y-1 w-1/4">
                    <div className={`font-bold ${getStatusColor(task.status)}`}>
                      {task.status}
                    </div>
                    <Button size="sm" color="primary" onPress={() => handleStart(task, t.mainTitle)}>
                      运行
                    </Button>
                    <Button size="sm" color="warning" onPress={() => handleStop(task)}>
                      暂停
                    </Button>
                    <Button size="sm" color="secondary" onPress={() => handleOpenConfig(task.cid)}>
                      配置
                    </Button>
                    <Button size="sm" color="default" onPress={() => handleOpenExport(task.cid, task.title)}>
                      导出弹幕
                    </Button>
                  </div>
                </div>
              </Card>
            ))}
          </AccordionItem>
        ))}
      </Accordion>

      <ExportXmlModal
        isOpen={isExportModalOpen}
        onClose={() => setIsExportModalOpen(false)}
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