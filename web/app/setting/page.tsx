"use client";

import { useEffect, useState } from "react";
import { Input, Button, Card, CardBody, CardHeader } from "@heroui/react";
import { Slider } from "@heroui/react";
import { Trash, Plus } from "lucide-react";
import { useAtomValue } from "jotai";

import { toast } from "@/config/toast";
import { title } from "@/components/primitives";
import { BACKEND_URL } from "@/config/env";

// 可增删的 Cookies 组件
function CookiesConfig({
  cookies,
  setCookies,
}: {
  cookies: string[];
  setCookies: (v: string[]) => void;
}) {
  const handleChange = (index: number, value: string) => {
    const newCookies = [...cookies];

    newCookies[index] = value;
    setCookies(newCookies);
  };

  const handleDelete = (index: number) => {
    const newCookies = cookies.filter((_, i) => i !== index);

    setCookies(newCookies);
  };

  const handleAdd = () => {
    setCookies([...cookies, ""]);
  };

  return (
    <Card className="mt-4">
      <CardHeader>Cookies 配置</CardHeader>
      <CardBody className="flex flex-col gap-3">
        {cookies.map((cookie, index) => (
          <div key={index} className="flex items-center gap-2">
            <Input
              className="flex-1"
              placeholder={`Cookie #${index + 1}`}
              value={cookie}
              onChange={(e) => handleChange(index, e.target.value)}
            />
            <Button
              isIconOnly
              color="danger"
              size="sm"
              onPress={() => handleDelete(index)}
            >
              <Trash size={16} />
            </Button>
          </div>
        ))}
        <Button
          size="sm"
          startContent={<Plus size={16} />}
          variant="flat"
          onPress={handleAdd}
        >
          添加 Cookie
        </Button>
      </CardBody>
    </Card>
  );
}

interface MainConfig {
  cookies: string[];
  timer: [number, number];
}

export default function SettingPage() {
  const [cookies, setCookies] = useState<string[]>([]);
  const [timerRange, setTimerRange] = useState<[number, number]>([5, 10]);
  const backendUrl = useAtomValue(BACKEND_URL);

  // 提交配置
  const handleSave = async () => {
    const cfg: MainConfig = {
      cookies: cookies.filter((s) => s.trim() !== ""),
      timer: timerRange,
    };

    const res = await fetch(`${backendUrl}/mainConfig/setConfig`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(cfg),
    });

    const result = await res.json();

    if (result.code === 0) {
      toast.success("保存成功");
    } else {
      toast.error("保存失败", result.msg);
    }
  };

  // 获取配置函数，封装方便复用
  const fetchConfig = async () => {
    try {
      const res = await fetch(`${backendUrl}/mainConfig/getConfig`);
      const data = await res.json();

      if (data.code === 0) {
        const cfg: MainConfig = data.data as MainConfig;

        setCookies(cfg.cookies);
        setTimerRange(cfg.timer);
      } else {
        toast.error("获取配置失败", data.msg);
      }
    } catch (e) {
      toast.error("获取配置异常", String(e));
    }
  };

  useEffect(() => {
    fetchConfig();
  }, []);

  const handleReRead = async () => {
    try {
      const res = await fetch(`${backendUrl}/mainConfig/reReadConfig`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
      });
      const result = await res.json();

      if (result.code === 0) {
        toast.success("读取成功");
        // 重新拉取最新配置，刷新界面
        await fetchConfig();
      } else {
        toast.error("读取失败", result.msg);
      }
    } catch (e) {
      toast.error("读取异常", String(e));
    }
  };

  return (
    <div className="p-0 max-w-xl mx-auto">
      <h1 className={title()}>爬虫全局配置</h1>

      <CookiesConfig cookies={cookies} setCookies={setCookies} />

      <Card className="mt-4">
        <CardHeader>
          <div className="text-left">
            爬取间隔 (秒)
            <br />
            <span
              style={{
                color: "violet",
                fontSize: 14,
              }}
            >
              如果只有一个凭证, 不建议间隔低于5秒
            </span>
          </div>
        </CardHeader>
        <CardBody>
          <Slider
            label="范围"
            maxValue={20}
            minValue={1}
            step={1}
            value={timerRange}
            onChange={(val) => setTimerRange(val as [number, number])}
          />
          <div className="text-sm text-gray-500 mt-2">
            当前: {timerRange[0]} ~ {timerRange[1]} 秒
          </div>
        </CardBody>
      </Card>

      <Button className="mt-6 w-full" color="primary" onPress={handleSave}>
        保存配置
      </Button>
      <Button className="mt-6 w-full" color="warning" onPress={handleReRead}>
        从<code>文件系统</code>
        <b>重新读取</b>配置文件
      </Button>
    </div>
  );
}
