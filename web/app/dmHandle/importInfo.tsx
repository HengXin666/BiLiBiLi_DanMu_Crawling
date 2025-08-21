import { Input } from "@heroui/react";

import { subtitle } from "@/components/primitives";

export default function ImportInfo({
  url,
  fileType,
}: {
  url?: string;
  fileType?: string;
}) {
  return (
    <>
      <h2 className={subtitle()}>导入信息</h2>
      <div className="flex flex-col gap-2">
        {url && (
          <Input
            isReadOnly
            label="弹幕来源(Blob)"
            labelPlacement="outside"
            type="url"
            value={url}
            variant="bordered"
          />
        )}
        <Input
          isReadOnly
          label="弹幕来源文件类型"
          labelPlacement="outside"
          type="text"
          value={fileType || "未知"}
          variant="bordered"
        />
      </div>
    </>
  );
}
