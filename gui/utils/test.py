import tkinter as tk
import threading
import time
import queue

class TimerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("计时器")
        
        self.label = tk.Label(root, text="0.0 秒")
        self.label.pack(pady=20)
        
        self.start_button = tk.Button(root, text="开始", command=self.start_timer)
        self.start_button.pack(pady=10)
        
        self.stop_button = tk.Button(root, text="停止", command=self.stop_timer)
        self.stop_button.pack(pady=10)

        self.running = False
        self.timer_queue = queue.Queue() # 线程安全
        
        self.update_timer()

    def start_timer(self):
        if not self.running:
            self.running = True
            self.timer_thread = threading.Thread(target=self.run_timer)
            self.timer_thread.start()

    def run_timer(self):
        elapsed_time = 0.0
        while self.running:
            time.sleep(0.1)
            elapsed_time += 0.1
            self.timer_queue.put(elapsed_time)

    def stop_timer(self):
        self.running = False

    def update_timer(self):
        try:
            while True:
                elapsed_time = self.timer_queue.get_nowait()
                self.label.config(text=f"{elapsed_time:.1f} 秒")
        except queue.Empty:
            pass
        self.root.after(100, self.update_timer)

if __name__ == "__main__":
    root = tk.Tk()
    app = TimerApp(root)
    root.mainloop()
