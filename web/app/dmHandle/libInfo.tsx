import type { UniPool } from "@dan-uni/dan-any";

import { Input } from "@heroui/react";

import { subtitle } from "@/components/primitives";

export default function LibInfo({ dmPool }: { dmPool: UniPool }) {
  return (
    <>
      <h2 className={subtitle()}>弹幕库信息</h2>
      <div className="flex flex-col gap-2">
        <Input
          isReadOnly
          label="弹幕数量"
          type="number"
          value={dmPool.dans.length.toString()}
          variant="bordered"
        />
        <Input
          isReadOnly
          description="def_<platform>+<cid>@<platform>"
          label="弹幕资源ID(SOID)"
          type="email"
          value={dmPool.shared.SOID}
          variant="bordered"
        />
        <Input
          isReadOnly
          label="弹幕来源"
          type="text"
          value={dmPool.shared.platform}
          variant="bordered"
        />
        <Input
          isReadOnly
          description="经过多次格式转换可能会带来意料之外的错误结果"
          isInvalid={dmPool.info.fromConverted}
          label="是否经过多次格式转换"
          type="text"
          value={
            dmPool.info.fromConverted
              ? "是(极有可能转换出错误的结果)"
              : "否(正常使用)"
          }
          variant="bordered"
        />
      </div>
    </>
  );
}
