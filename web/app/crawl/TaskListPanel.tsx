"use client";

import React, { useState, useEffect } from "react";
import {
  Accordion,
  AccordionItem,
  Card,
  Button,
  Progress,
  Spinner,
} from "@nextui-org/react";
import { DateTime } from "luxon";
import { BACKEND_URL } from "@/config/env";
import { TaskConfigModal, TaskConfig } from "./TaskConfigModal";

interface AllTaskData {
  mainTitle: string;
  tasks: TaskConfig[];
}

export function TaskListPanel({ refreshKey }: { refreshKey: number }) {
  const [taskList, setTaskList] = useState<AllTaskData[]>([]);
  const [taskAddMap, setTaskAddMap] = useState<Record<number, number>>({});
  const [loadingConfig, setLoadingConfig] = useState(false);

  const [taskConfigData, setTaskConfigData] = useState<TaskConfig | null>(null);
  const [isConfigModalOpen, setIsConfigModalOpen] = useState(false);

  // 拉取任务列表
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

  // 启动任务
  const handleStart = async (task: TaskConfig, mainTitle: string) => {
    await fetch(`${BACKEND_URL}/allDm/startTask`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ cid: task.cid, path: `${mainTitle}/${task.title}` }),
    });

    // WebSocket监听弹幕增量
    const ws = new WebSocket(
      `${BACKEND_URL.replace(/^http/, "ws")}/allDm/ws/${task.cid}`
    );
    ws.onmessage = (event) => {
      const addCount = Number(event.data);
      setTaskAddMap((prev) => ({
        ...prev,
        [task.cid]: (prev[task.cid] || 0) + addCount,
      }));
    };

    fetchTasks();
  };

  // 暂停任务
  const handleStop = async (task: TaskConfig) => {
    await fetch(`${BACKEND_URL}/allDm/stopTask`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(task.cid),
    });
    fetchTasks();
  };

  // 打开任务配置管理弹窗
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

  // 保存任务配置
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


  return (
    <div className="space-y-4">
      <Accordion>
        {taskList.map((t) => (
          <AccordionItem key={t.mainTitle} title={t.mainTitle}>
            {t.tasks.map((task) => (
              <Card key={task.cid} className="p-4 mb-2">
                <div className="flex justify-between items-start">
                  <div className="text-left space-y-1">
                    <div>
                      <strong>爬取:</strong> {task.title} ({task.cid})
                    </div>
                    <div>
                      <strong>爬取数据:</strong> {task.totalDanmaku} 条 (+
                      {taskAddMap[task.cid] || 0}) | 神弹幕:{" "}
                      {task.advancedDanmaku}
                    </div>
                    <div>
                      <strong>当前进度:</strong> {formatTimestamp(task.currentTime)} /{" "}
                      {formatTimestamp(task.range[1])}
                    </div>
                    <div>
                      <strong>上次执行:</strong> {formatTimestamp(task.lastFetchTime)}
                    </div>
                    <Progress
                      value={
                        ((task.currentTime - task.range[0]) /
                          (task.range[1] - task.range[0])) *
                        100
                      }
                      className="mt-1"
                    />
                  </div>
                  <div className="flex flex-col items-end space-y-1">
                    <div className={`font-bold ${getStatusColor(task.status)}`}>
                      {task.status}
                    </div>
                    <Button
                      size="sm"
                      color="primary"
                      onPress={() => handleStart(task, t.mainTitle)}
                    >
                      运行
                    </Button>
                    <Button
                      size="sm"
                      color="warning"
                      onPress={() => handleStop(task)}
                    >
                      暂停
                    </Button>
                    <Button
                      size="sm"
                      color="secondary"
                      onPress={() => handleOpenConfig(task.cid)}
                    >
                      管理配置
                    </Button>
                  </div>
                </div>
              </Card>
            ))}
          </AccordionItem>
        ))}
      </Accordion>

      {/* 任务配置管理弹窗 */}
      <TaskConfigModal
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
