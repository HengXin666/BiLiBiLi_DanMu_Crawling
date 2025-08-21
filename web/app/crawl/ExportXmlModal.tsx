"use client";

import React, { useEffect, useState } from "react";
import {
  Modal,
  ModalBody,
  ModalFooter,
  ModalHeader,
  Button,
  Input,
  Checkbox,
  ModalContent,
} from "@heroui/react";
import { useRouter } from "next/navigation";
import { fileSave } from "browser-fs-access";
import { useAtomValue } from "jotai";

import { BACKEND_URL } from "@/config/env";

interface ExportXmlModalProps {
  isOpen: boolean;
  onClose: () => void;
  configId: string;
  cid: number;
  defaultFileName: string;
}

export function ExportXmlModal({
  isOpen,
  onClose,
  configId,
  cid,
  defaultFileName,
}: ExportXmlModalProps) {
  const router = useRouter();
  const [fileName, setFileName] = useState<string>(defaultFileName);
  const [includeWeight, setIncludeWeight] = useState<boolean>(false);
  const [dmHandel, setDmHandle] = useState<boolean>(false);
  const [loading, setLoading] = useState<boolean>(false);
  const backendUrl = useAtomValue(BACKEND_URL);

  useEffect(() => {
    // init
    if (isOpen) {
      // 替换的非法字符
      const sanitizePath = (input: string): string => {
        return input.replace(/[ \/\\\*\?\<\>\|":]/g, "_");
      };

      setFileName(sanitizePath(`${cid}_${defaultFileName}`));
      setIncludeWeight(true);
    }
  }, [isOpen, defaultFileName]);

  const handleExport = async () => {
    setLoading(true);
    try {
      const res = await fetch(`${backendUrl}/allDm/exportXml`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        // py: ExportXmlOptions
        body: JSON.stringify({ configId, cid, fileName, includeWeight }),
      });

      // 从响应头获取真实文件名
      const contentDisposition = res.headers.get("Content-Disposition");
      let filename = fileName.endsWith(".xml") ? fileName : `${fileName}.xml`;

      // 解析后端返回的编码文件名（如果存在）
      if (contentDisposition) {
        const match = contentDisposition.match(/filename\*=(?:UTF-8'')?(.+)/i);

        if (match) filename = decodeURIComponent(match[1]);
      }

      const blob = await res.blob();

      if (dmHandel) {
        const url = URL.createObjectURL(blob);

        router.push(`/dmHandle?url=${url}&fn=${fileName}`);
      } else {
        fileSave(blob, {
          fileName: filename,
          extensions: [".xml"],
          startIn: "downloads", // 文件选择器默认打开的目录
        });
      }
    } finally {
      setLoading(false);
      onClose();
    }
  };

  return (
    <Modal isDismissable={false} isOpen={isOpen} onClose={onClose}>
      <ModalContent>
        <ModalHeader>导出弹幕</ModalHeader>
        <ModalBody>
          <Input
            endContent={
              <div className="pointer-events-none flex items-center">
                <span className="text-default-400 text-small">.xml</span>
              </div>
            }
            label="文件名称"
            value={fileName}
            onChange={(e) => setFileName(e.target.value)}
          />
          <Checkbox isSelected={includeWeight} onValueChange={setIncludeWeight}>
            导出弹幕权重
          </Checkbox>
          <Checkbox isSelected={dmHandel} onValueChange={setDmHandle}>
            稍后进行弹幕处理
          </Checkbox>
        </ModalBody>
        <ModalFooter>
          <Button color="default" onPress={onClose}>
            取消
          </Button>
          <Button color="primary" isLoading={loading} onPress={handleExport}>
            确认导出
          </Button>
        </ModalFooter>
      </ModalContent>
    </Modal>
  );
}
