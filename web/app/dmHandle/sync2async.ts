export default function sync2async<T extends (...args: any) => any>(
  func: Function,
  ...args: any[]
): Promise<ReturnType<T>> {
  return new Promise((resolve) => {
    setTimeout(() => resolve(func(...args)), 0);
  });
}
