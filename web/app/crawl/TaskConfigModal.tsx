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
  Checkbox,
  DatePicker,
} from "@nextui-org/react";
import { CalendarDate, DateValue, getLocalTimeZone, today } from "@internationalized/date";

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
  return new CalendarDate(dt.getFullYear(), dt.getMonth() + 1, dt.getDate());
}

function toTimestamp (date: DateValue | null, isEnd: boolean = false): number {
  if (!date) return 0;
  const ts = Date.UTC(date.year, date.month - 1, date.day, isEnd ? 23 : 0, isEnd ? 59 : 0, isEnd ? 59 : 0) / 1000;
  return ts - 8 * 3600;
}

export interface TaskConfig {
  cid: number;
  title: string;
  range: [number, number];
}

export const TaskConfigModal: React.FC<TaskConfigModalProps> = ({
  isOpen,
  loading,
  configData,
  onClose,
  onSave,
}) => {
  const [title, setTitle] = useState("");
  const [useDefaultRange, setUseDefaultRange] = useState(true);
  const [startTime, setStartTime] = useState<DateValue>(today(getLocalTimeZone()));
  const [endTime, setEndTime] = useState<DateValue>(today(getLocalTimeZone()));

  useEffect(() => {
    if (!configData) return;
    const [startTs, endTs] = configData.range;
    setTitle(configData.title);
    setUseDefaultRange(startTs === 0 && endTs === 0);
    setStartTime(toDateValue(startTs) ?? today(getLocalTimeZone()));
    setEndTime(toDateValue(endTs) ?? today(getLocalTimeZone()));
  }, [configData]);

  const handleSaveClick = async () => {
    if (!configData) return;
    const newRange: [number, number] = useDefaultRange
      ? [0, 0]
      : [toTimestamp(startTime, false), toTimestamp(endTime, true)];
    await onSave({ ...configData, title, range: newRange });
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
              <div className="flex flex-col gap-3">
                <Checkbox
                  isSelected={useDefaultRange}
                  onChange={(e) => setUseDefaultRange(e.target.checked)}
                  size="md"
                >
                  爬取全弹幕
                </Checkbox>

                {/* 垃圾报错 */}
                {!useDefaultRange && (
                  <div className="flex gap-4">
                    <DatePicker
                      label="开始时间"
                      value={startTime}
                      onChange={setStartTime}
                      granularity="day"
                      isRequired
                      className="flex-grow"
                    />
                    <DatePicker
                      label="结束时间"
                      value={endTime}
                      onChange={setEndTime}
                      granularity="day"
                      isRequired
                      className="flex-grow"
                    />
                  </div>
                )}
              </div>
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
};
