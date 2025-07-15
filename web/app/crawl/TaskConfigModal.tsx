"use client";

import React, { useState, useEffect } from "react";
import {
  Modal,
  ModalContent,
  ModalHeader,
  ModalBody,
  ModalFooter,
  Button,
  Spinner,
  Input,
} from "@nextui-org/react";
import { DateValue, getLocalTimeZone, today } from "@internationalized/date";
import { TaskDateRangePicker } from "./TaskDateRangePicker";

export interface TaskConfig {
  cid: number;
  title: string;
  lastFetchTime: number;
  range: [number, number];
  currentTime: number;
  totalDanmaku: number;
  advancedDanmaku: number;
  status: string;
}

interface TaskConfigModalProps {
  isOpen: boolean;
  loading: boolean;
  configData: TaskConfig | null;
  onClose: () => void;
  onSave: (config: TaskConfig) => Promise<void>;
}

function toDateValue (timestamp: number): DateValue | null {
  if (timestamp === 0) return null;
  const dt = new Date(timestamp * 1000);
  return {
    year: dt.getFullYear(),
    month: dt.getMonth() + 1,
    day: dt.getDate(),
  };
}

function toTimestamp (date: DateValue | null, isEnd: boolean = false): number {
  if (!date) return 0;
  const ts = Date.UTC(date.year, date.month - 1, date.day, isEnd ? 23 : 0, isEnd ? 59 : 0, isEnd ? 59 : 0) / 1000;
  return ts - 8 * 3600;
}

export function TaskConfigModal ({
  isOpen,
  loading,
  configData,
  onClose,
  onSave,
}: TaskConfigModalProps) {
  const [title, setTitle] = useState("");
  const [startTime, setStartTime] = useState<DateValue>(today(getLocalTimeZone()));
  const [endTime, setEndTime] = useState<DateValue>(today(getLocalTimeZone()));
  const [useDefaultRange, setUseDefaultRange] = useState(true);

  const handleRangeChange = (
    newStartTime: DateValue,
    newEndTime: DateValue
  ) => {
    setStartTime(newStartTime);
    setEndTime(newEndTime);
  };

  useEffect(() => {
    setTitle(configData?.title || "");
    setStartTime(toDateValue(configData?.range[0] || 0) || today(getLocalTimeZone()));
    setEndTime(toDateValue(configData?.range[1] || 0) || today(getLocalTimeZone()));
    setUseDefaultRange(configData?.range[0] === configData?.range[1] && configData?.range[0] === 0);
  }, [configData]);

  const handleSaveClick = async () => {
    if (!configData) return;
    const newRange: [number, number] = useDefaultRange
      ? [0, 0]
      : [toTimestamp(startTime, false), toTimestamp(endTime, true)];
    const newConfig: TaskConfig = {
      ...configData,
      title,
      range: newRange,
    };
    await onSave(newConfig);
  };

  return (
    <Modal isOpen={isOpen} onClose={onClose} size="md" isDismissable={false}>
      <ModalContent>
        <ModalHeader>任务配置管理</ModalHeader>
        <ModalBody>
          {loading || !configData ? (
            <Spinner />
          ) : (
            <div className="space-y-4">
              <div>cid: {configData.cid}</div>
              <Input label="标题" value={title} onChange={e => setTitle(e.target.value)} fullWidth />
              <TaskDateRangePicker
                _useDefaultRange={useDefaultRange}
                _startTime={startTime}
                _endTime={endTime}
                onChange={handleRangeChange}
              />
            </div>
          )}
        </ModalBody>
        <ModalFooter>
          <Button onPress={onClose} color="secondary">取消</Button>
          <Button color="primary" onPress={handleSaveClick} disabled={loading || !configData}>
            保存
          </Button>
        </ModalFooter>
      </ModalContent>
    </Modal>
  );
}
