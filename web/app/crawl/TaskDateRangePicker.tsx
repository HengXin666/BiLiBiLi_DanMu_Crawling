"use client";

import React, { useState, useEffect } from "react";
import { Checkbox, DatePicker } from "@nextui-org/react";
import { DateValue, getLocalTimeZone, today } from "@internationalized/date";

export interface TaskDateRangePickerProps {
  _useDefaultRange: boolean;
  _startTime: DateValue;
  _endTime: DateValue;
  onChange: (startTime: DateValue, endTime: DateValue) => void;
}

export function TaskDateRangePicker({
  _useDefaultRange: propUseDefaultRange,
  _startTime: propStartTime,
  _endTime: propEndTime,
  onChange,
}: TaskDateRangePickerProps) {
  const [useDefaultRange, setUseDefaultRange] = useState<boolean>(propUseDefaultRange);
  const [startTime, setStartTime] = useState<DateValue>(propStartTime);
  const [endTime, setEndTime] = useState<DateValue>(propEndTime);

  // 外部属性变动时同步内部状态
  useEffect(() => {
    setUseDefaultRange(propUseDefaultRange);
  }, [propUseDefaultRange]);

  useEffect(() => {
    setStartTime(propStartTime);
  }, [propStartTime]);

  useEffect(() => {
    setEndTime(propEndTime);
  }, [propEndTime]);

  // 每次值变时通知外部
  useEffect(() => {
    if (onChange) onChange(startTime, endTime);
  }, [useDefaultRange, startTime, endTime, onChange]);

  return (
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
  );
}
