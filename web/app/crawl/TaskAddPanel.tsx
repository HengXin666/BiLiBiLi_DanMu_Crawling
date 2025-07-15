"use client";

import { useState } from "react";
import { Input, Button, CheckboxGroup, Checkbox, Card, DatePicker } from "@nextui-org/react";
import { toast } from "sonner";
import { DateValue, getLocalTimeZone, today } from "@internationalized/date";
import { BACKEND_URL } from "@/config/env";

interface VideoPart {
  cid: number;
  page: number;
  part: string;
}

interface VidoPartConfigVo {
  path: string;
  data: VideoPart;
  range: [number, number];
}

function isValidTaskName(name: string): boolean {
  return !/[\/\\\*\?\<\>\|":]/.test(name);
}

export function TaskAddPanel({ onSuccess }: { onSuccess: () => void }) {
  const [taskName, setTaskName] = useState<string>("");
  const [urlOrCid, setUrlOrCid] = useState<string>("");
  const [parts, setParts] = useState<VideoPart[]>([]);
  const [selectedParts, setSelectedParts] = useState<number[]>([]);

  const [useDefaultRange, setUseDefaultRange] = useState<boolean>(true);
  const [startTime, setStartTime] = useState<DateValue>(today(getLocalTimeZone()));
  const [endTime, setEndTime] = useState<DateValue>(today(getLocalTimeZone()));

  const handleFetchParts = async (): Promise<void> => {
    if (!urlOrCid.trim()) {
      toast.error("请输入 URL 或 CID");
      return;
    }

    try {
      const res = await fetch(`${BACKEND_URL}/videoInfo/getVideoPartList`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ url: urlOrCid }),
      });

      const data = await res.json();

      if (data.code === 0) {
        setParts(data.data.cidList);
        setSelectedParts(data.data.cidList.map((v: VideoPart) => v.cid));
      } else {
        toast.error(`获取分P失败: ${data.msg}`);
      }
    } catch (e) {
      toast.error(`接口请求异常: ${String(e)}`);
    }
  };

  const convertRange = (): [number, number] => {
    if (useDefaultRange) {
      return [0, 0];
    }

    return [
      Date.UTC(startTime.year, startTime.month - 1, startTime.day, 0, 0, 0) / 1000,
      Date.UTC(endTime.year, endTime.month - 1, endTime.day, 23, 59, 59) / 1000,
    ];
  };

  const handleSetTaskConfig = async (): Promise<void> => {
    if (!taskName.trim()) {
      toast.error("请输入任务名称");
      return;
    }

    if (!isValidTaskName(taskName)) {
      toast.error("任务名称包含非法字符：/ \\ * ? < > | \" :");
      return;
    }

    if (parts.length === 0) {
      toast.error("请先获取分P列表");
      return;
    }

    const range: [number, number] = convertRange();

    try {
      for (const part of parts) {
        if (selectedParts.includes(part.cid)) {
          const config: VidoPartConfigVo = {
            path: `${taskName}/${part.part}`,
            data: part,
            range,
          };

          await fetch(`${BACKEND_URL}/allDm/setTaskConfig`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(config),
          });
        }
      }

      toast.success("任务初始化成功");
      onSuccess();
    } catch (e) {
      toast.error(`任务初始化失败: ${String(e)}`);
    }
  };

  return (
    <Card className="p-4 space-y-4">
      <Input
        label="任务名称"
        placeholder="请输入任务名称"
        value={taskName}
        onChange={(e) => setTaskName(e.target.value)}
      />

      <Input
        label="URL 或 CID"
        placeholder="如果输入cid, 应输入 cid=2233 格式 (无空格)"
        value={urlOrCid}
        onChange={(e) => setUrlOrCid(e.target.value)}
      />

      <Button color="secondary" onPress={handleFetchParts}>
        获取分P
      </Button>

      {parts.length > 0 && (
        <CheckboxGroup
          label="选择需要爬取的分P"
          value={selectedParts.map(String)}
          onValueChange={(val) => setSelectedParts(val.map(Number))}
        >
          {parts.map((p) => (
            <Checkbox key={p.cid} value={String(p.cid)}>
              {p.page} | {p.part} | {p.cid}
            </Checkbox>
          ))}
        </CheckboxGroup>
      )}

      <Checkbox isSelected={useDefaultRange} onValueChange={setUseDefaultRange}>
        爬取全弹幕
      </Checkbox>

      {!useDefaultRange && (
        <div className="flex gap-4">
          <DatePicker
            label="开始时间"
            value={startTime}
            onChange={setStartTime}
            granularity="day"
            isRequired
          />
          <DatePicker
            label="结束时间"
            value={endTime}
            onChange={setEndTime}
            granularity="day"
            isRequired
          />
        </div>
      )}

      <Button color="primary" onPress={handleSetTaskConfig}>
        确定并初始化任务
      </Button>
    </Card>
  );
}
