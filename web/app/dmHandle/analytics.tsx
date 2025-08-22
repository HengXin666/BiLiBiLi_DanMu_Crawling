"use client";

import type { UniPool } from "@dan-uni/dan-any";

import { Button, Input, Spinner } from "@heroui/react";
import { memo, Suspense, use, useMemo, useState } from "react";

import { subtitle } from "@/components/primitives";

import sync2async from "./sync2async";

const Main = memo(function Main({
  dmPoolMost,
}: {
  dmPoolMost: Promise<{
    content: { val?: string | number; count: number };
    senderID: { val?: string | number; count: number };
  }>;
}) {
  const most = use(dmPoolMost);

  return (
    <>
      <Input
        isReadOnly
        label="出现最多的内容"
        type="text"
        value={most.content.val as string}
        variant="bordered"
      />
      <Input
        isReadOnly
        description="<midHash>@bili，对midHash逆推可得用户的mid"
        label="发言最多的用户ID"
        type="email"
        value={most.senderID.val as string}
        variant="bordered"
      />
    </>
  );
});

export default memo(function Analytics({
  dmPool,
  limit = 6000,
}: {
  dmPool: UniPool;
  limit?: number;
}) {
  const [show, setShow] = useState(dmPool.dans.length <= limit);
  const dmp = useMemo(() => {
    setShow(dmPool.dans.length <= limit);

    return dmPool;
  }, [dmPool]);

  function getMost(dmPool: UniPool) {
    return {
      content: dmPool.getMost("content"),
      senderID: dmPool.getMost("senderID"),
    };
  }

  return (
    <>
      <h2 className={subtitle()}>统计信息</h2>
      <div className="flex flex-col gap-2">
        {show ? (
          <Suspense fallback={<Spinner variant="wave" />}>
            <Main
              dmPoolMost={
                sync2async<typeof getMost>(getMost, dmp)
                // new Promise((resolve) => {
                //   setTimeout(() => resolve(getMost(dmp)), 0);
                // })
              }
            />
          </Suspense>
        ) : (
          <div className="text-center justify-center">
            <p>该弹幕库条目过多，统计操作可能会造成不可预期的卡顿、无响应。</p>
            <Button
              className="mt-2"
              color="warning"
              onPress={() => setShow(true)}
            >
              我已知悉
            </Button>
          </div>
        )}
      </div>
    </>
  );
});
