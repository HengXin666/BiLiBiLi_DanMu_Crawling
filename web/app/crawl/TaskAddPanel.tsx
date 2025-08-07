"use client";

import { useState } from "react";
import {
  Input,
  Button,
  CheckboxGroup,
  Checkbox,
  Card,
  DatePicker,
} from "@heroui/react";
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

interface TaskAddPanelProps {
  onSuccess: () => void;
  isInModal?: boolean;
}

function isValidTaskName(name: string): boolean {
  return !/[\/\\\*\?\<\>\|":]/.test(name);
}

export function TaskAddPanel({
  onSuccess,
  isInModal = false,
}: TaskAddPanelProps) {
  const [taskName, setTaskName] = useState<string>("");
  const [urlOrCid, setUrlOrCid] = useState<string>("");
  const [parts, setParts] = useState<VideoPart[]>([]);
  const [selectedParts, setSelectedParts] = useState<number[]>([]);

  const [useDefaultRange, setUseDefaultRange] = useState(true);
  const [startTime, setStartTime] = useState<DateValue>(
    today(getLocalTimeZone()),
  );
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

    const toBiliTimestamp = (
      year: number,
      month: number,
      day: number,
      hour: number,
      minute: number,
      second: number,
    ): number => {
      // Date.UTC 返回的是 UTC 时间，B站使用东八区时间
      return (
        Math.floor(
          Date.UTC(year, month - 1, day, hour, minute, second) / 1000,
        ) -
        8 * 3600
      );
    };

    return [
      toBiliTimestamp(startTime.year, startTime.month, startTime.day, 0, 0, 0),
      toBiliTimestamp(endTime.year, endTime.month, endTime.day, 23, 59, 59),
    ];
  };

  const handleInitTaskConfig = async (): Promise<void> => {
    if (!taskName.trim()) {
      toast.error("请输入任务名称");

      return;
    }

    if (!isValidTaskName(taskName)) {
      toast.error('任务名称包含非法字符：/ \\ * ? < > | " :');

      return;
    }

    if (parts.length === 0) {
      toast.error("请先获取分P列表");

      return;
    }

    const range: [number, number] = convertRange();

    // 替换 part.part 的非法字符
    const sanitizePath = (input: string): string => {
      return input.replace(/[\/\\\*\?\<\>\|":]/g, "_");
    };

    try {
      for (const part of parts) {
        if (selectedParts.includes(part.cid)) {
          const safePartName = sanitizePath(part.part);
          const config: VidoPartConfigVo = {
            path: `${taskName}/${safePartName}`,
            data: part,
            range,
          };

          await fetch(`${BACKEND_URL}/allDm/initTaskConfig`, {
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
    <Card
      className={
        isInModal ? "space-y-4 p-4 shadow-none bg-transparent" : "p-4 space-y-4"
      }
    >
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

      <div className="flex flex-col gap-3">
        <Checkbox
          isSelected={useDefaultRange}
          size="md"
          onChange={(e) => setUseDefaultRange(e.target.checked)}
        >
          爬取全弹幕
        </Checkbox>

        {!useDefaultRange && (
          <div className="flex gap-4">
            <DatePicker
              isRequired
              className="flex-grow"
              granularity="day"
              label="开始时间"
              value={startTime}
              onChange={(e) => setStartTime(e || today(getLocalTimeZone()))}
            />
            <DatePicker
              isRequired
              className="flex-grow"
              granularity="day"
              label="结束时间"
              value={endTime}
              onChange={(e) => setEndTime(e || today(getLocalTimeZone()))}
            />
          </div>
        )}
      </div>

      <Button color="primary" onPress={handleInitTaskConfig}>
        确定并初始化任务
      </Button>
    </Card>
  );
}
