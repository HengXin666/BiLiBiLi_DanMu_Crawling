import { atom } from "jotai";
import { atomWithStorage } from "jotai/utils";

export const BACKEND_URL = atomWithStorage(
  "BACKEND_URL",
  "http://127.0.0.1:28299",
);

/**
 * - 0 成功
 * - 1 失败
 * - 2 初始
 * - 3 加载
 */
export const BACKEND_URL_OK = atom(2);
