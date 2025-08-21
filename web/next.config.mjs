const isProd = process.env.NODE_ENV === "production";

const internalHost = process.env.TAURI_DEV_HOST || "localhost";

/** @type {import('next').NextConfig} */
const nextConfig = {
  // 确保 Next.js 使用 SSG 而不是 SSR
  // https://nextjs.org/docs/pages/building-your-application/deploying/static-exports
  output: "export",
  // 注意：在 SSG 模式下使用 Next.js 的 Image 组件需要此功能。
  // 请参阅 https://nextjs.org/docs/messages/export-image-api 了解不同的解决方法。
  images: {
    unoptimized: true,
  },
  // 配置 assetPrefix，否则服务器无法正确解析您的资产。
  assetPrefix: isProd ? undefined : `http://${internalHost}:3000`,
};

export default nextConfig;
