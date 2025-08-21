import type { UniPool } from "@dan-uni/dan-any";

import { Input } from "@heroui/react";
import { useMemo } from "react";

import { subtitle } from "@/components/primitives";

export default function Analytics({ dmPool }: { dmPool: UniPool }) {
  const most = useMemo(() => {
    return dmPool.most;
  }, [dmPool]);

  return (
    <>
      <h2 className={subtitle()}>统计信息</h2>
      <div className="flex flex-col gap-2">
        <Input
          isReadOnly
          label="出现最多的内容"
          type="text"
          value={most.content}
          variant="bordered"
        />
        <Input
          isReadOnly
          description="<midHash>@bili，对midHash逆推可得用户的mid"
          label="发言最多的用户ID"
          type="email"
          value={most.senderID}
          variant="bordered"
        />
      </div>
    </>
  );
}
