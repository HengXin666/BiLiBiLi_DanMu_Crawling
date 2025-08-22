"use client";

import {
  Button,
  ButtonGroup,
  Code,
  Divider,
  Input,
  Kbd,
  Modal,
  ModalBody,
  ModalContent,
  ModalFooter,
  ModalHeader,
  NumberInput,
  Tooltip,
  useDisclosure,
} from "@heroui/react";
import { useRouter, useSearchParams } from "next/navigation";
import { useMemo, useState, useTransition } from "react";
import { DM_format, UniPool } from "@dan-uni/dan-any";
import {
  fileOpen,
  fileSave,
  FileWithHandle,
  supported as FSAsupported,
} from "browser-fs-access";

import { toast } from "@/config/toast";
import { title, subtitle } from "@/components/primitives";

import ImportInfo from "./importInfo";
import LibInfo from "./libInfo";
import Analytics from "./analytics";
import sync2async from "./sync2async";

const sanitizePath = (input: string): boolean =>
  /[ \/\\\*\?\<\>\|":]/.test(input);
const emptyUniPool = UniPool.create();

export default function DmHandlePage() {
  const searchParams = useSearchParams();
  const router = useRouter();
  const url = searchParams.get("url") || "",
    filename = searchParams.get("fn") || "";
  const [fileExt, setFileExt] = useState<string>("xml");
  const [fileName, setFileName] = useState<string>(filename);
  const [dmPool, setDmPool] = useState<UniPool>(emptyUniPool);
  const [fileType, setFileType] = useState<DM_format>();
  const [mergeLifetime, setMergeLifetime] = useState<number>(10);
  const [FSA, setFSA] = useState<FileSystemFileHandle>();
  const FSAWarning = useDisclosure();
  const [FSAWarningClose, setFSAWarningCLose] = useState<boolean>(FSAsupported);
  const [isPending, startTransition] = useTransition();
  const [isLoading2, setLoading] = useState(false);

  const isLoading = isPending || isLoading2;

  const fnValid = useMemo(() => {
    if (fileName === "") return false;

    return sanitizePath(fileName);
  }, [fileName]);

  const importDm = async () => {
    const fail = (description?: string) => {
      toast.error("弹幕导入失败", description);
      setFileType(undefined);
      setLoading(false);
    };
    const success = () => {
      toast.success("弹幕导入成功");
      setLoading(false);
    };

    setLoading(true);

    if (url)
      await fetch(url)
        .then((res) => res.text())
        .then(async (dmFile) => {
          setFileType("bili.xml");
          setDmPool(await sync2async(() => UniPool.fromBiliXML(dmFile)));
          success();
        })
        .catch(() => {
          fail();
          router.push("/dmHandle");
        });
    else {
      const file =
        FSA && !dmPool.info.fromConverted
          ? ((await FSA.getFile()) as FileWithHandle)
          : await fileOpen([
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
            ]).catch((e) => {
              const err = String(e);

              toast.error(
                "弹幕导入失败",
                err.includes("aborted") ? "用户手动取消文件选择" : err,
              );

              setLoading(false);
            });

      if (!file) {
        // 这里就不该报错
        // addToast({});

        return;
      }
      const getFileExtension = (filename: string) => {
        const parts = filename.split(".");

        return parts.length > 1 ? parts.pop() : "";
      };

      if (!(FSA && !dmPool.info.fromConverted) && file.handle)
        setFSA(file.handle);

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
          setDmPool(
            await sync2async(async () =>
              UniPool.fromBiliXML(await file.text()),
            ),
          );
        } catch (e) {
          fail(`解析XML失败，该XML不是bili格式[${e}]]`);
        }
      } else if (ext === "ass") {
        try {
          const pool = await sync2async(async () =>
            UniPool.fromASS(await file.text()),
          );

          if (pool.dans.length === 0) throw new Error("该ASS不含恢复信息");
          else {
            setFileType("common.ass");
            setDmPool(pool);
          }
        } catch (e) {
          fail(`解析ASS失败[${e}]`);
        }
      } else if (ext === "json") {
        try {
          const imp = await sync2async(async () =>
            UniPool.import(await file.text()),
          );

          setFileType(imp.fmt);
          setDmPool(imp.pool);
        } catch (e) {
          fail(`解析JSON失败[${e}]`);
        }
      } else {
        try {
          const imp = await sync2async(async () =>
            UniPool.import(await file.arrayBuffer()),
          );

          setFileType(imp.fmt);
          setDmPool(imp.pool);
        } catch (e) {
          fail(`解析二进制文件失败[${e}]`);
        }
      }

      success();
    }
  };
  const fileSaver = async (
    dm: string | Uint8Array | Promise<string | Uint8Array>,
    ext: string,
  ) => {
    fileSave(
      ext === "pb.bin" && FSAsupported
        ? (dm as unknown as Promise<Blob>)
        : new Blob([(await dm) as string]),
      {
        fileName: fileName + "." + ext,
        extensions: ["." + ext],
        startIn: "downloads",
      },
    )
      .then(() => {
        toast.success("弹幕已导出");
      })
      .catch((e) => {
        const err = String(e);

        toast.error(
          "弹幕导出失败",
          err.includes("aborted") ? "用户手动取消导出" : err,
        );
      })
      .finally(() => setLoading(false));
  };
  const startDownload = async (
    dm: string | Uint8Array | Promise<string | Uint8Array>,
    ext: string,
  ) => {
    setLoading(true);
    setFileExt(ext);

    if (FSAWarningClose) fileSaver(dm, ext);
    else {
      setLoading(false);
      FSAWarning.onOpen();
    }
  };

  // useEffect(() => {}, []);

  return (
    <div className="flex flex-col gap-4">
      <h1 className={title()}>弹幕处理</h1>

      <ImportInfo fileType={fileType} url={url} />

      <Divider />
      {(dmPool.dans.length > 0 && !dmPool.info.fromConverted) || (
        <>
          <Button color="primary" isLoading={isLoading} onPress={importDm}>
            读取弹幕
          </Button>
          <div className="text-sm text-gray-400">
            点击<Kbd>读取弹幕</Kbd>
            后，在打开的窗口可选择弹幕类型以筛选文件，也可选择全部文件挑选文件。
            <br />
            只有
            <Tooltip color="foreground" content="完全支持" offset={-3}>
              <Kbd>@dan-uni/dan-any</Kbd>
            </Tooltip>
            及
            <Tooltip color="foreground" content="兼容性支持" offset={-3}>
              <Kbd>biliy</Kbd>
            </Tooltip>
            导出的ASS可以被还原。
          </div>
        </>
      )}
      {dmPool.dans.length > 0 && (
        <>
          <LibInfo dmPool={dmPool} />
          <Divider />
          <Analytics dmPool={dmPool} />
          <Divider />
          {dmPool.info.fromConverted || (
            <>
              <>
                <h2 className={subtitle()}>处理选项</h2>
                <div className="flex flex-wrap gap-4 items-center">
                  <div>
                    <Button
                      color="danger"
                      isLoading={isLoading}
                      onPress={importDm}
                    >
                      还原全部处理
                    </Button>
                  </div>
                  <Divider />
                  <div className="flex flex-col gap-2 lg:flex-row">
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
                      isLoading={isLoading}
                      onPress={() => {
                        startTransition(() =>
                          sync2async(() => {
                            startTransition(() =>
                              setDmPool(dmPool.merge(mergeLifetime)),
                            );
                            toast.success("去重成功");
                          }),
                        );
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
            <>
              {dmPool.info.fromConverted || (
                <>
                  <Input
                    isRequired
                    endContent={
                      <div className="pointer-events-none flex items-center">
                        <span className="text-default-400 text-small">
                          .{fileExt}
                        </span>
                      </div>
                    }
                    isInvalid={fnValid}
                    label="文件名称"
                    labelPlacement="outside"
                    type="text"
                    value={fileName}
                    variant="bordered"
                    onValueChange={setFileName}
                  />
                  <div className="flex flex-col gap-2 lg:flex-row">
                    <Button
                      color="primary"
                      isLoading={isLoading}
                      onPress={() => {
                        startDownload(
                          sync2async(() => dmPool.toBiliXML()),
                          "xml",
                        );
                      }}
                    >
                      Bili(XML)
                    </Button>
                    <Button
                      color="primary"
                      isLoading={isLoading}
                      onPress={() => {
                        const canvas = document.createElement("canvas");

                        canvas.width = 50;
                        canvas.height = 50;
                        const ctx = canvas.getContext("2d");

                        startDownload(
                          sync2async(() =>
                            dmPool.toASS(ctx, {
                              filename: `${fileName}.xml`,
                              title: fileName,
                              raw: {
                                compressType: "gzip",
                                baseType: "base18384",
                              },
                            }),
                          ),
                          "ass",
                        );
                      }}
                    >
                      ASS
                    </Button>
                    <ButtonGroup isDisabled={isLoading}>
                      <Button
                        color="primary"
                        onPress={() => {
                          startDownload(
                            sync2async(() => JSON.stringify(dmPool.dans)),
                            "json",
                          );
                        }}
                      >
                        DanUni(JSON)
                      </Button>
                      <Button
                        color="secondary"
                        onPress={() => {
                          startDownload(
                            sync2async(() => dmPool.toPb()),
                            "pb.bin",
                          );
                        }}
                      >
                        DanUni(ProtoBuf)
                      </Button>
                    </ButtonGroup>
                    <ButtonGroup isDisabled={isLoading}>
                      <Button
                        color="primary"
                        onPress={() => {
                          startDownload(
                            sync2async(() =>
                              JSON.stringify(dmPool.toDplayer()),
                            ),
                            "json",
                          );
                        }}
                      >
                        Dplayer
                      </Button>
                      <Button
                        color="secondary"
                        onPress={() => {
                          startDownload(
                            sync2async(() =>
                              JSON.stringify(dmPool.toArtplayer()),
                            ),
                            "json",
                          );
                        }}
                      >
                        Artplayer
                      </Button>
                      <Button
                        color="default"
                        onPress={() => {
                          startDownload(
                            sync2async(() => JSON.stringify(dmPool.toDDplay())),
                            "json",
                          );
                        }}
                      >
                        弹弹Play
                      </Button>
                    </ButtonGroup>
                  </div>
                  <Modal
                    isOpen={FSAWarning.isOpen}
                    onOpenChange={FSAWarning.onOpenChange}
                  >
                    <ModalContent>
                      {(onClose) => (
                        <>
                          <ModalHeader>⚠️ 兼容性警告</ModalHeader>
                          <ModalBody>
                            <p>
                              当前环境不支持
                              <Code color="primary">FileSystemAccessAPI</Code>
                              ，点击保存将可能直接存储至<Code>下载</Code>或
                              <Code>Downloads</Code>文件夹。
                            </p>
                            <p>确认后再次点击导出格式对应按钮即可保存。</p>
                          </ModalBody>
                          <ModalFooter>
                            <Button
                              color="danger"
                              variant="flat"
                              onPress={onClose}
                            >
                              取消
                            </Button>
                            <Button
                              color="primary"
                              onPress={() => {
                                setFSAWarningCLose(true);
                                onClose();
                              }}
                            >
                              我已知悉
                            </Button>
                          </ModalFooter>
                        </>
                      )}
                    </ModalContent>
                  </Modal>
                </>
              )}

              <Button
                color="danger"
                isLoading={isLoading}
                onPress={() => {
                  const success = () =>
                    toast.success("导出弹幕完成", "已释放缓存");

                  if (url) {
                    URL.revokeObjectURL(url);
                    success();
                    router.push("/crawl");
                  } else {
                    setFileType(undefined);
                    setDmPool(emptyUniPool);
                    setFSA(undefined);
                    success();
                  }
                }}
              >
                确认导出完成(退出)
              </Button>
            </>
          </>
        </>
      )}
    </div>
  );
}
