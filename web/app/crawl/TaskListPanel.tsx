"use client";

import { useEffect, useState } from "react";
import { Card, Button, Progress } from "@nextui-org/react";
import { BACKEND_URL } from "@/config/env";

interface TaskConfig {
  cid: number;
  title: string;
  lastFetchTime: number;
  range: [number, number];
  currentTime: number;
  totalDanmaku: number;
  advancedDanmaku: number;
  status: string; // 用枚举更好，这里简化
}

interface AllTaskData {
  mainTitle: string;
  tasks: TaskConfig[];
}

export function TaskListPanel() {
  const [taskList, setTaskList] = useState<AllTaskData[]>([]);

  const fetchTasks = async () => {
    const res = await fetch(`${BACKEND_URL}/allDm/getAllTaskData`);
    const data = await res.json();
    setTaskList(data.data);
  };

  useEffect(() => {
    fetchTasks();
  }, []);

  const handleStart = async (task: TaskConfig, mainTitle: string) => {
    await fetch(`${BACKEND_URL}/allDm/startTask`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ cid: task.cid, path: `${mainTitle}/${task.title}` }),
    });
    fetchTasks();
  };

  const handleStop = async (task: TaskConfig) => {
    await fetch(`${BACKEND_URL}/allDm/stopTask`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(task.cid), // 假设 cid 作为 taskId
    });
    fetchTasks();
  };

  return (
    <div className="space-y-4">
      {taskList && taskList.map((t) => (
        <Card key={t.mainTitle} className="p-4">
          <h3>{t.mainTitle}</h3>
          {t.tasks.map((task) => (
            <div key={task.cid} className="flex items-center justify-between p-4">
              <div>
                {task.title} - 弹幕数: {task.totalDanmaku}
                <Progress value={(task.currentTime - task.range[0]) / (task.range[1] - task.range[0]) * 100} />
              </div>
              <div>
                <Button size="sm" onPress={() => handleStart(task, t.mainTitle)}>
                  开始
                </Button>
                <Button size="sm" onPress={() => handleStop(task)}>
                  停止
                </Button>
              </div>
            </div>
          ))}
        </Card>
      ))}
    </div>
  );
}
