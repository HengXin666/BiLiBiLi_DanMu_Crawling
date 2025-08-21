// next.config.mjs
const isProd = process.env.NODE_ENV === "production";
const internalHost = process.env.TAURI_DEV_HOST || "localhost";
const repoName = "BiLiBiLi_DanMu_Crawling"; // GitHub 仓库名

/** @type {import('next').NextConfig} */
const nextConfig = {
  output: "export",             // 确保静态导出
  trailingSlash: true,          // 生成 /about/index.html 等, 避免 404
  images: {
    unoptimized: true,          // 静态导出不优化图片
  },
  basePath: isProd ? `/${repoName}` : "",           // GitHub Pages 仓库路径
  assetPrefix: isProd ? `/${repoName}/` : `http://${internalHost}:3000`, // 静态资源前缀
};

export default nextConfig;
