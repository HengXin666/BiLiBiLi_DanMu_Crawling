"use client";

import React, { useState } from "react";
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
} from "@heroui/react";
import {
  CalendarDate,
  DateValue,
  getLocalTimeZone,
  today,
} from "@internationalized/date";

export interface TaskConfig {
  configId: string;
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

function toDateValue(timestamp: number): DateValue | null {
  if (timestamp === 0) return null;
  const dt = new Date(timestamp * 1000);

  return new CalendarDate(dt.getFullYear(), dt.getMonth() + 1, dt.getDate());
}

function toTimestamp(date: DateValue | null, isEnd: boolean = false): number {
  if (!date) return 0;
  const ts =
    Date.UTC(
      date.year,
      date.month - 1,
      date.day,
      isEnd ? 23 : 0,
      isEnd ? 59 : 0,
      isEnd ? 59 : 0,
    ) / 1000;

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
  // 使用 configData.configId 作为 key 的依据,当 configData 变化时重置状态
  const configId = configData?.configId ?? "";

  // 计算初始值
  const getInitialValues = () => {
    if (!configData) {
      return {
        title: "",
        useDefaultRange: true,
        startTime: today(getLocalTimeZone()),
        endTime: today(getLocalTimeZone()),
      };
    }

    const [startTs, endTs] = configData.range;
    const useDefaultRange = startTs === 0 && endTs === 0;

    return {
      title: configData.title,
      useDefaultRange,
      startTime: toDateValue(startTs) ?? today(getLocalTimeZone()),
      endTime: toDateValue(endTs) ?? today(getLocalTimeZone()),
    };
  };

  const [title, setTitle] = useState(() => getInitialValues().title);
  const [useDefaultRange, setUseDefaultRange] = useState(
    () => getInitialValues().useDefaultRange,
  );
  const [startTime, setStartTime] = useState<DateValue>(
    () => getInitialValues().startTime,
  );
  const [endTime, setEndTime] = useState<DateValue>(
    () => getInitialValues().endTime,
  );

  const handleSaveClick = async () => {
    if (!configData) return;

    const newRange: [number, number] = useDefaultRange
      ? [0, 0]
      : [toTimestamp(startTime, false), toTimestamp(endTime, true)];

    await onSave({ ...configData, title, range: newRange });
  };

  return (
    <Modal
      key={configId}
      isDismissable={false}
      isOpen={isOpen}
      size="md"
      onClose={onClose}
    >
      <ModalContent>
        <ModalHeader>任务配置管理</ModalHeader>
        <ModalBody>
          {loading || !configData ? (
            <Spinner />
          ) : (
            <div className="space-y-4">
              <div>cid: {configData.cid}</div>
              <Input
                fullWidth
                label="标题"
                value={title}
                onChange={(e) => setTitle(e.target.value)}
              />
              <div className="flex flex-col gap-3">
                <Checkbox
                  isSelected={useDefaultRange}
                  size="md"
                  onChange={(e) => setUseDefaultRange(e.target.checked)}
                >
                  爬取全弹幕
                </Checkbox>

                {/* 垃圾报错 */}
                {!useDefaultRange && (
                  <div className="flex gap-4">
                    <DatePicker
                      isRequired
                      className="flex-grow"
                      granularity="day"
                      label="开始时间"
                      value={startTime}
                      onChange={(e) =>
                        setStartTime(e || today(getLocalTimeZone()))
                      }
                    />
                    <DatePicker
                      isRequired
                      className="flex-grow"
                      granularity="day"
                      label="结束时间"
                      value={endTime}
                      onChange={(e) =>
                        setEndTime(e || today(getLocalTimeZone()))
                      }
                    />
                  </div>
                )}
              </div>
            </div>
          )}
        </ModalBody>
        <ModalFooter>
          <Button color="secondary" onPress={onClose}>
            取消
          </Button>
          <Button
            color="primary"
            disabled={loading || !configData}
            onPress={handleSaveClick}
          >
            保存
          </Button>
        </ModalFooter>
      </ModalContent>
    </Modal>
  );
};
