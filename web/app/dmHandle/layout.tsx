import { Suspense } from "react";

export default function CrawlLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <section className="flex flex-col items-center justify-center gap-4 py-8 md:py-10">
      <div className="inline-block w-5/6 md:w-2/3 text-center justify-center">
        <Suspense>{children}</Suspense>
      </div>
    </section>
  );
}
