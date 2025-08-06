"use client";

import {
  addToast,
  Button,
  ButtonGroup,
  Divider,
  Input,
  NumberInput,
} from "@heroui/react";
import { useRouter, useSearchParams } from "next/navigation";
import { useState } from "react";
import { DM_format, UniPool } from "@dan-uni/dan-any";
import { fileOpen, fileSave } from "browser-fs-access";

import { title, subtitle } from "@/components/primitives";

const sanitizePath = (input: string): boolean => {
  return /[ \/\\\*\?\<\>\|":]/g.test(input);
};

export default function DmHandlePage() {
  const searchParams = useSearchParams();
  const router = useRouter();
  const [fileExt, setFileExt] = useState<string>("xml");
  const url = searchParams.get("url") || "",
    filename = searchParams.get("fn") || "";
  const [fileName, setFileName] = useState<string>(filename);
  const [dmPool, setDmPool] = useState<UniPool>(UniPool.create());
  const [fileType, setFileType] = useState<DM_format>();
  const [mergeLifetime, setMergeLifetime] = useState<number>(10);

  const importDm = async () => {
    const fail = (description?: string) =>
      addToast({ title: "弹幕导入失败", description, color: "danger" });
    const success = () =>
      addToast({
        title: "弹幕导入成功",
        timeout: 1000,
        shouldShowTimeoutProgress: true,
        color: "success",
      });

    if (url)
      await fetch(url)
        .then((res) => res.text())
        .then((dmFile) => {
          setFileType("bili.xml");
          setDmPool(UniPool.fromBiliXML(dmFile));
          success();
        })
        .catch(() => {
          fail();
          router.push("/dmHandle");
        });
    else {
      const file = await fileOpen([
        {
          description: "bili xml 弹幕",
          mimeTypes: ["application/xml"],
          extensions: [".xml"],
        },
        {
          description: "ProtoBuf 弹幕",
          mimeTypes: ["application/octet-stream"],
          extensions: [".bin", ".so"],
        },
        {
          description: "JSON 弹幕",
          mimeTypes: ["application/json"],
          extensions: [".json"],
        },
        {
          description: "ASS 弹幕",
          mimeTypes: ["text/x-ssa"],
          extensions: [".ass"],
        },
      ]);
      const getFileExtension = (filename: string) => {
        const parts = filename.split(".");

        return parts.length > 1 ? parts.pop() : "";
      };

      const fn = file.name;
      let ext = getFileExtension(fn),
        mainFn = fn.replaceAll(`.${ext}`, "");

      if (ext === "bin" && mainFn.endsWith(".pb")) {
        ext = "pb.bin";
        mainFn = mainFn.replaceAll(`.pb`, "");
      }

      ext && setFileExt(ext);
      setFileName(mainFn);

      if (ext === "xml") {
        try {
          setFileType("bili.xml");
          setDmPool(UniPool.fromBiliXML(await file.text()));
        } catch (e) {
          setFileType(undefined);
          fail(`解析XML失败，该XML不是bili格式[${e}]]`);
        }
      } else if (ext === "ass") {
        try {
          const pool = UniPool.fromASS(await file.text());

          if (pool.dans.length === 0) throw new Error("该ASS不含恢复信息");
          else {
            setFileType("common.ass");
            setDmPool(pool);
          }
        } catch (e) {
          setFileType(undefined);
          fail(`解析ASS失败[${e}]`);
        }
      } else if (ext === "json") {
        try {
          // console.log(await file.text());
          const imp = UniPool.import(await file.text());

          setFileType(imp.fmt);
          setDmPool(imp.pool);
        } catch (e) {
          setFileType(undefined);
          fail(`解析JSON失败[${e}]`);
        }
      } else {
        try {
          const imp = UniPool.import(await file.arrayBuffer());

          setFileType(imp.fmt);
          setDmPool(imp.pool);
        } catch (e) {
          setFileType(undefined);
          fail(`解析二进制文件失败[${e}]`);
        }
      }
    }
  };
  const startDownload = (dm: string | Uint8Array, ext: string) => {
    setFileExt(ext);
    fileSave(new Blob([dm]), {
      fileName: fileName + "." + ext,
      extensions: ["." + ext],
      startIn: "downloads",
    });
    addToast({
      title: "弹幕已导出",
      timeout: 3000,
      shouldShowTimeoutProgress: true,
      color: "success",
    });
  };

  // useEffect(() => {}, []);

  return (
    <div className="flex flex-col gap-4">
      <h1 className={title()}>弹幕处理</h1>

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

      <Divider />
      {(dmPool.dans.length > 0 && !dmPool.info.fromConverted) || (
        <>
          <Button color="primary" onPress={importDm}>
            读取弹幕
          </Button>
          <p className="text-sm text-gray-400">
            可选择弹幕类型以筛选文件，也可选择全部文件挑选文件。
          </p>
        </>
      )}
      {dmPool.dans.length > 0 && (
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
          <Divider />
          <>
            <h2 className={subtitle()}>统计信息</h2>
            <div className="flex flex-col gap-2">
              <Input
                label="出现最多的内容"
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
          {dmPool.info.fromConverted || (
            <>
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
                          <span className="text-default-400 text-small">
                            秒
                          </span>
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
            </>
          )}
          <>
            <h2 className={subtitle()}>导出设置</h2>
            {dmPool.info.fromConverted || (
              <>
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
                        "ass"
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
                        startDownload(dmPool.toPb(), "pb.bin");
                      }}
                    >
                      DanUni(ProtoBuf)
                    </Button>
                  </ButtonGroup>
                  <ButtonGroup>
                    <Button
                      color="primary"
                      onPress={() => {
                        startDownload(
                          JSON.stringify(dmPool.toDplayer()),
                          "json"
                        );
                      }}
                    >
                      Dplayer
                    </Button>
                    <Button
                      color="secondary"
                      onPress={() => {
                        startDownload(
                          JSON.stringify(dmPool.toArtplayer()),
                          "json"
                        );
                      }}
                    >
                      Artplayer
                    </Button>
                    <Button
                      color="default"
                      onPress={() => {
                        startDownload(
                          JSON.stringify(dmPool.toDDplay()),
                          "json"
                        );
                      }}
                    >
                      弹弹Play
                    </Button>
                  </ButtonGroup>
                </div>
              </>
            )}

            <Button
              color="danger"
              onPress={() => {
                const success = () =>
                  addToast({
                    title: "导出弹幕完成",
                    description: "已释放缓存",
                    color: "success",
                    timeout: 1000,
                    shouldShowTimeoutProgress: true,
                  });

                if (url) {
                  URL.revokeObjectURL(url);
                  success();
                  router.push("/crawl");
                } else {
                  setFileType(undefined);
                  setDmPool(UniPool.create());
                  success();
                }
              }}
            >
              确认导出完成(退出)
            </Button>
          </>
        </>
      )}
    </div>
  );
}
