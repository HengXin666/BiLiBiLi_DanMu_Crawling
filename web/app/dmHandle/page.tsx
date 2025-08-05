"use client";

import {
  addToast,
  Button,
  ButtonGroup,
  Divider,
  Input,
  NumberInput,
} from "@heroui/react";
import { useSearchParams } from "next/navigation";
import { useState } from "react";
import { UniPool } from "@dan-uni/dan-any";

import { title, subtitle } from "@/components/primitives";

const sanitizePath = (input: string): boolean => {
  return /[ \/\\\*\?\<\>\|":]/g.test(input);
};

export default function DmHandlePage() {
  const searchParams = useSearchParams();
  const [fileExt, setFileExt] = useState<string>("xml");
  const url = searchParams.get("url") || "",
    filename = searchParams.get("fn") || "";
  const [fileName, setFileName] = useState<string>(filename);
  const [dmPool, setDmPool] = useState<UniPool>(UniPool.create());
  const [mergeLifetime, setMergeLifetime] = useState<number>(10);

  const importDm = () => {
    fetch(url)
      .then((res) => res.text())
      .then((dmFile) => {
        setDmPool(UniPool.fromBiliXML(dmFile));
        addToast({
          title: "导入弹幕成功",
          timeout: 3000,
          shouldShowTimeoutProgress: true,
          color: "success",
        });
      })
      .catch(() => addToast({ title: "导入弹幕失败", color: "danger" }));
  };
  const startDownload = (dm: string | Uint8Array, ext: string) => {
    setFileExt(ext);
    const link = document.createElement("a");
    const url = URL.createObjectURL(new Blob([dm]));

    link.href = url;
    link.download = fileName + "." + ext;
    link.click();
    addToast({
      title: "导出弹幕",
      timeout: 3000,
      shouldShowTimeoutProgress: true,
      color: "success",
    });
    URL.revokeObjectURL(url);
  };

  // useEffect(() => {}, []);

  return (
    <div className="flex flex-col gap-4">
      <h1 className={title()}>弹幕处理</h1>
      <>
        <h2 className={subtitle()}>导入信息</h2>
        <div className="flex flex-col gap-2">
          <Input
            isReadOnly
            defaultValue={url}
            label="弹幕来源(Blob)"
            labelPlacement="outside"
            type="url"
            variant="bordered"
          />
        </div>
      </>
      <Divider />
      {(dmPool.dans.length > 0 && !dmPool.info.fromConverted) || (
        <Button color="primary" onPress={importDm}>
          读取弹幕
        </Button>
      )}
      {dmPool.dans.length > 0 && !dmPool.info.fromConverted && (
        <>
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
                label="是否经过多次格式转换"
                type="text"
                value={
                  dmPool.info.fromConverted
                    ? "是(可能转换出错误的结果)"
                    : "否(正常使用)"
                }
                variant="bordered"
              />
            </div>
          </>
          <Divider />
          <>
            <h2 className={subtitle()}>统计信息</h2>
            <div className="flex flex-col gap-2">
              <Input
                label="最多出现的内容"
                type="text"
                value={dmPool.most.content}
                variant="bordered"
              />
              <Input
                description="<midHash>@bili，对midHash逆推可得用户的mid"
                label="发言最多的用户ID"
                type="email"
                value={dmPool.most.senderID}
                variant="bordered"
              />
            </div>
          </>
          <Divider />
          <>
            <h2 className={subtitle()}>处理选项</h2>
            <div className="flex flex-wrap gap-4 items-center">
              <div>
                <Button color="danger" onPress={importDm}>
                  还原全部处理
                </Button>
              </div>
              <Divider />
              <div className="flex gap-2">
                <NumberInput
                  description="合并一定时间段内的重复弹幕，防止同屏出现过多[0表示不查重]"
                  endContent={
                    <div className="pointer-events-none flex items-center">
                      <span className="text-default-400 text-small">秒</span>
                    </div>
                  }
                  label="去重合并弹幕阈值(查重时间区段)"
                  type="number"
                  value={mergeLifetime}
                  onValueChange={(e) => setMergeLifetime(e)}
                />
                <Button
                  color="primary"
                  onPress={() => {
                    setDmPool(dmPool.merge(mergeLifetime));
                    addToast({
                      title: "去重成功",
                      timeout: 1000,
                      shouldShowTimeoutProgress: true,
                      color: "success",
                    });
                  }}
                >
                  去重(基础)
                </Button>
              </div>
            </div>
          </>
          <Divider />
          <>
            <h2 className={subtitle()}>导出设置</h2>
            <Input
              isRequired
              defaultValue={fileName}
              endContent={
                <div className="pointer-events-none flex items-center">
                  <span className="text-default-400 text-small">
                    .{fileExt}
                  </span>
                </div>
              }
              isInvalid={sanitizePath(fileName)}
              label="文件名称"
              labelPlacement="outside"
              type="text"
              variant="bordered"
              onValueChange={setFileName}
            />
            <div className="flex gap-2">
              <Button
                color="primary"
                onPress={() => {
                  startDownload(dmPool.toBiliXML(), "xml");
                }}
              >
                Bili(XML)
              </Button>
              <Button
                color="primary"
                onPress={() => {
                  const canvas = document.createElement("canvas");

                  canvas.width = 50;
                  canvas.height = 50;
                  const ctx = canvas.getContext("2d");

                  startDownload(
                    dmPool.toASS(ctx, {
                      filename: `${fileName}.xml`,
                      title: fileName,
                      raw: { compressType: "gzip", baseType: "base18384" },
                    }),
                    "ass",
                  );
                }}
              >
                ASS
              </Button>
              <ButtonGroup>
                <Button
                  color="primary"
                  onPress={() => {
                    startDownload(JSON.stringify(dmPool.dans), "json");
                  }}
                >
                  DanUni(JSON)
                </Button>
                <Button
                  color="secondary"
                  onPress={() => {
                    startDownload(dmPool.toPb(), "protobuf");
                  }}
                >
                  DanUni(ProtoBuf)
                </Button>
              </ButtonGroup>
              <ButtonGroup>
                <Button
                  color="primary"
                  onPress={() => {
                    startDownload(JSON.stringify(dmPool.toDplayer()), "json");
                  }}
                >
                  Dplayer
                </Button>
                <Button
                  color="secondary"
                  onPress={() => {
                    startDownload(JSON.stringify(dmPool.toArtplayer()), "json");
                  }}
                >
                  Artplayer
                </Button>
                <Button
                  color="default"
                  onPress={() => {
                    startDownload(JSON.stringify(dmPool.toDDplay()), "json");
                  }}
                >
                  弹弹Play
                </Button>
              </ButtonGroup>
            </div>
          </>
        </>
      )}
    </div>
  );
}
