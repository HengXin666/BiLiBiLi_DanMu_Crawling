import { addToast } from "@heroui/react";

export const toast = {
  success: (message: string, details?: string) => {
    addToast({
      title: message,
      description: details,
      color: "success",
    });
  },
  error: (message: string, details?: string) => {
    addToast({
      title: message,
      description: details,
      color: "danger",
    });
  },
  info: (message: string, details?: string) => {
    addToast({
      title: message,
      description: details,
      color: "default",
    });
  },
  warning: (message: string, details?: string) => {
    addToast({
      title: message,
      description: details,
      color: "warning",
    });
  },
};
