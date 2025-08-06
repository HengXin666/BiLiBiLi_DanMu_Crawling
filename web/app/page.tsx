"use client";

import { Link } from "@heroui/link";
import { Snippet } from "@heroui/react";
import { Code } from "@heroui/react";
import { button as buttonStyles } from "@heroui/theme";

import { siteConfig } from "@/config/site";
import { title, subtitle } from "@/components/primitives";
import { GithubIcon } from "@/components/icons";

export default function Home() {
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
    </section>
  );
}
