"use client";

import React, { useEffect, useState } from "react";
import { Modal, ModalBody, ModalFooter, ModalHeader, Button, Input, Checkbox, ModalContent } from "@nextui-org/react";
import { BACKEND_URL } from "@/config/env";

interface ExportXmlModalProps {
  isOpen: boolean;
  onClose: () => void;
  configId: string;
  cid: number;
  defaultFileName: string;
}

export function ExportXmlModal ({ isOpen, onClose, configId, cid, defaultFileName }: ExportXmlModalProps) {
  const [fileName, setFileName] = useState<string>(defaultFileName);
  const [includeWeight, setIncludeWeight] = useState<boolean>(false);
  const [loading, setLoading] = useState<boolean>(false);

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
      const res = await fetch(`${BACKEND_URL}/allDm/exportXml`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        // py: ExportXmlOptions
        body: JSON.stringify({ configId, cid, fileName, includeWeight }),
      });

      // 从响应头获取真实文件名
      const contentDisposition = res.headers.get('Content-Disposition');
      let filename = fileName.endsWith(".xml") ? fileName : `${fileName}.xml`;

      // 解析后端返回的编码文件名（如果存在）
      if (contentDisposition) {
        const match = contentDisposition.match(/filename\*=(?:UTF-8'')?(.+)/i);
        if (match) filename = decodeURIComponent(match[1]);
      }

      const blob = await res.blob();
      const url = URL.createObjectURL(blob);
      const link = document.createElement("a");
      link.href = url;
      link.download = filename; // 使用解码后的文件名
      link.click();
      URL.revokeObjectURL(url);
    } finally {
      setLoading(false);
      onClose();
    }
  };

  return (
    <Modal isOpen={isOpen} onClose={onClose} isDismissable={false}>
      <ModalContent>
        <ModalHeader>导出弹幕</ModalHeader>
        <ModalBody>
          <Input
            label="文件名称"
            value={fileName}
            onChange={(e) => setFileName(e.target.value)}
            description=".xml 会自动加上"
          />
          <Checkbox isSelected={includeWeight} onValueChange={setIncludeWeight}>
            导出弹幕权重
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
