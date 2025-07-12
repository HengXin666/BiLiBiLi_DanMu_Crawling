import tkinter as tk
from tkinter import messagebox, font
from .api.reqDataSingleton import ReqDataSingleton

class CredentialManager:
    def __init__(self, master):
        self.master = master
        self.master.title("凭证管理")
        
        # 设置字体
        self.custom_font = font.Font(family="黑体", size=14)

        self.label = tk.Label(master, text="凭证:", font=self.custom_font)
        self.label.grid(row=0, column=0, padx=10, pady=10)

        self.credential_entry = tk.Entry(master, font=self.custom_font, width=40)
        self.credential_entry.grid(row=0, column=1, padx=10, pady=10)

        self.add_button = tk.Button(master, text="添加凭证", command=self.add_credential, font=self.custom_font)
        self.add_button.grid(row=1, column=0, padx=10, pady=10)

        self.remove_button = tk.Button(master, text="删除凭证", command=self.remove_credential, font=self.custom_font)
        self.remove_button.grid(row=1, column=1, padx=10, pady=10)

        self.listbox = tk.Listbox(master, font=self.custom_font, width=50, height=10)
        self.listbox.grid(row=2, column=0, columnspan=2, padx=10, pady=10)

        self.update_listbox()

    def add_credential(self):
        credential = self.credential_entry.get()
        if credential and credential not in ReqDataSingleton().cookies:
            ReqDataSingleton().cookies.append(credential)
            self.credential_entry.delete(0, tk.END)
            self.update_listbox()
        else:
            messagebox.showwarning("警告", "凭证不能为空或已存在")

    def remove_credential(self):
        selected = self.listbox.curselection()
        if selected:
            credential = self.listbox.get(selected)
            ReqDataSingleton().cookies.remove(credential)
            self.update_listbox()
        else:
            messagebox.showwarning("警告", "请选择一个凭证进行删除")

    def update_listbox(self):
        self.listbox.delete(0, tk.END)
        for credential in ReqDataSingleton().cookies:
            self.listbox.insert(tk.END, credential)

if __name__ == "__main__":
    root = tk.Tk()
    app = CredentialManager(root)
    root.mainloop()
    print(ReqDataSingleton().cookies)
