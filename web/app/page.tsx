"use client";

import { Link } from "@heroui/link";
import { Button, Form, Input, Snippet } from "@heroui/react";
import { Code } from "@heroui/react";
import { button as buttonStyles } from "@heroui/theme";
import { useAtom } from "jotai";
import { RESET } from "jotai/utils";

import { siteConfig } from "@/config/site";
import { title, subtitle } from "@/components/primitives";
import { GithubIcon } from "@/components/icons";
import { BACKEND_URL, BACKEND_URL_OK } from "@/config/env";
import { ClientOnly } from "@/components/client-only";
import { toast } from "@/config/toast";

export default function Home() {
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
      .then((data) => {
        if (data?.data === "pong") {
          toast.success("连接成功");
          setBackendUrlOk(0);
        } else fail();
      })
      .catch(() => fail());
  };

  if (backendUrlOk === 2) ping();

  return (
    <section className="flex flex-col items-center justify-center gap-4 py-8 md:py-10">
      <div className="inline-block max-w-xl text-center justify-center">
        <span className={title()}>B站&nbsp;</span>
        <span className={title({ color: "violet" })}>全弹幕&nbsp;</span>
        <span className={title()}>爬虫</span>
        <br />
        <div className={subtitle({ class: "mt-4" })}>前端控制台</div>
      </div>

      <div className="flex gap-3">
        <Link
          isExternal
          className={buttonStyles({
            color: "primary",
            radius: "full",
            variant: "shadow",
          })}
          href={siteConfig.links.docs}
        >
          文档
        </Link>
        <Link
          isExternal
          className={buttonStyles({ variant: "bordered", radius: "full" })}
          href={siteConfig.links.github}
        >
          <GithubIcon size={20} />
          GitHub
        </Link>
      </div>

      <div className="mt-8">
        <Snippet hideCopyButton hideSymbol variant="bordered">
          <span>
            点击 <Code color="primary">爬虫</Code> 开始进行全弹幕爬取!
          </span>
        </Snippet>
      </div>

      <div className="mt-2">
        <ClientOnly>
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
        </ClientOnly>
      </div>
    </section>
  );
}
