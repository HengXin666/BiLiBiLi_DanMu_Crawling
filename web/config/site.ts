export type SiteConfig = typeof siteConfig;

export const siteConfig = {
  name: "B站全弹幕爬虫 | HX",
  description: "这是一个开源的高效的B站全弹幕爬虫Web端.",
  navItems: [
    {
      label: "主页",
      href: "/",
    },
    {
      label: "爬虫",
      href: "/crawl",
    },
    {
      label: "处理",
      href: "/dmHandle",
    },
    {
      label: "设置",
      href: "/setting",
    },
    {
      label: "关于",
      href: "/about",
    },
  ],
  links: {
    github: "https://github.com/HengXin666/BiLiBiLi_DanMu_Crawling/",
    docs: "https://heroui.com",
    sponsor: "https://hengxin666.github.io/HXLoLi/",
  },
};
