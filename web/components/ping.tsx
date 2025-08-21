"use client";

import { Button, Form, Input } from "@heroui/react";
import { useAtom } from "jotai";
import { RESET } from "jotai/utils";
import { useEffect } from "react";

import { toast } from "@/config/toast";
import { BACKEND_URL, BACKEND_URL_OK } from "@/config/env";

export default function Ping({ renderUI = false }: { renderUI?: boolean }) {
  const [backendUrl, setBackendUrl] = useAtom(BACKEND_URL);
  const [backendUrlOk, setBackendUrlOk] = useAtom(BACKEND_URL_OK);

  const isLoading = backendUrlOk === 3;

  const ping = () => {
    if (isLoading) return;
    setBackendUrlOk(3);

    const fail = () => {
      toast.error("连接失败");
      setBackendUrlOk(1);
    };

    fetch(`${backendUrl}/base/ping`)
      .then((res) => res.json())
      .then(async (data) => {
        if (data?.data === "pong") {
          const ver = await fetch(`${backendUrl}/base/version`)
            .then((res) => res.json())
            .then((data) => data.data);

          toast.success("连接成功", "当前后端版本号：" + ver);
          setBackendUrlOk(0);
        } else fail();
      })
      .catch(() => fail());
  };

  useEffect(() => {
    if (backendUrlOk === 2) ping();
  }, []);

  if (renderUI)
    return (
      <Form
        className="flex flex-col"
        validationBehavior="aria"
        onReset={() => {
          setBackendUrl(RESET);
          ping();
        }}
        onSubmit={(e) => {
          e.preventDefault();
          ping();
        }}
      >
        <Input
          isRequired
          color={(() => {
            switch (backendUrlOk) {
              case 1:
                return "danger";
              case 0:
                return "success";
              default:
                return undefined;
            }
          })()}
          label={"后端URL - " + (!backendUrlOk ? "已连接" : "未连接")}
          labelPlacement="outside"
          type="url"
          value={backendUrl}
          variant="bordered"
          onValueChange={setBackendUrl}
        />
        <div className="flex gap-2">
          <Button color="primary" isLoading={isLoading} type="submit">
            保存
          </Button>
          <Button
            color="danger"
            isLoading={isLoading}
            type="reset"
            variant="flat"
          >
            重置
          </Button>
        </div>
      </Form>
    );
  else return <></>;
}
