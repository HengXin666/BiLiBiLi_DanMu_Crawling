"use client";

import {
  Card,
  CardHeader,
  CardBody,
  Button,
  ScrollShadow,
  Image,
  Code,
  User,
  Link,
} from "@heroui/react";

import { GithubIcon } from "@/components/icons";
import { siteConfig } from "@/config/site";
import pkg from "@/package.json";

export default function AboutPage() {
  return (
    <div className="max-w-3xl mx-auto space-y-8">
      <h1 className="text-4xl font-bold tracking-tight">关于本项目</h1>

      <ScrollShadow className="h-auto space-y-6">
        <Card className="rounded-2xl shadow-lg">
          <CardHeader>
            <h2 className="text-2xl font-semibold">项目简介</h2>
          </CardHeader>
          <CardBody>
            <p className="text-base text-muted-foreground leading-relaxed">
              本程序可爬取B站视频的历史弹幕, 支持爬取高级弹幕/BAS弹幕.
              <br />
              全新的爬取算法可尽可能在一次请求中爬取更多弹幕.
              <br />
              实测在 5~8 秒的请求间隔下, 在 55 分钟内爬取了 109.8599 万条弹幕
              (爬完了{" "}
              <a href="https://www.bilibili.com/video/BV1Js411o76u/">
                BV1Js411o76u
              </a>
              )
            </p>
          </CardBody>
        </Card>

        <Card className="rounded-2xl shadow-lg">
          <CardHeader>
            <h2 className="text-2xl font-semibold">项目信息</h2>
          </CardHeader>
          <CardBody>
            <ul className="text-base text-muted-foreground space-y-2">
              <li>
                版本:{" "}
                <span className="font-medium">
                  <Code>v{pkg.version}</Code>
                </span>
              </li>
              <li>
                许可证:{" "}
                <span className="font-medium">
                  <Code>MIT</Code>
                </span>
              </li>
              <li>
                作者:{" "}
                <User
                  avatarProps={{
                    src: "https://avatars.githubusercontent.com/u/103022267?v=4",
                  }}
                  description={
                    <Link
                      isExternal
                      href="https://github.com/HengXin666"
                      size="sm"
                    >
                      @HengXin666
                    </Link>
                  }
                  name="Heng_Xin"
                />
              </li>
            </ul>
          </CardBody>
        </Card>

        <Card className="rounded-2xl shadow-lg">
          <CardHeader>
            <h2 className="text-2xl font-semibold">贡献者</h2>
          </CardHeader>
          <CardBody>
            <p className="text-base text-muted-foreground mb-4 leading-relaxed">
              感谢所有为本项目做出贡献的开发者。
            </p>
            <p className="text-base text-muted-foreground mb-4 leading-relaxed">
              特别感谢{" "}
              <a href="https://github.com/ccicnce113424">ccicnce113424</a> 在{" "}
              <a href="https://github.com/HengXin666/BiLiBiLi_DanMu_Crawling/issues/14">
                #14
              </a>{" "}
              中提供的新的爬取思路(该方法受BiliPlus全弹幕下载器启发)。
            </p>
            <a
              href="https://github.com/HengXin666/BiLiBiLi_DanMu_Crawling/graphs/contributors"
              rel="noopener noreferrer"
              target="_blank"
            >
              <Image
                alt="贡献者列表"
                className="rounded-xl w-auto max-w-full hover:opacity-80 transition-opacity"
                src="https://contrib.rocks/image?repo=HengXin666/BiLiBiLi_DanMu_Crawling"
              />
            </a>
          </CardBody>
        </Card>

        <Card className="rounded-2xl shadow-lg">
          <CardHeader>
            <h2 className="text-2xl font-semibold">参与贡献</h2>
          </CardHeader>
          <CardBody>
            <p className="text-base text-muted-foreground mb-4 leading-relaxed">
              我们欢迎所有开发者参与项目的开发与维护, 您可以通过 GitHub 提交 PR
              或 Issue.
            </p>
            <Button
              as="a"
              color="primary"
              href={siteConfig.links.github}
              rel="noopener noreferrer"
              startContent={<GithubIcon size={18} />}
              target="_blank"
              variant="shadow"
            >
              前往 GitHub
            </Button>
          </CardBody>
        </Card>
      </ScrollShadow>
    </div>
  );
}
