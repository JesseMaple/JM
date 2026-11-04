# 适配FileOperator

import tkinter as tk
from tkinter import messagebox
import random
from FileOperator import *


class GuessGUI(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)  # 初始化父类窗口
        self.title("猜数字游戏") # 窗口名
        self.geometry( # 设置窗口大小
            f"{self.winfo_screenwidth()//2}x{self.winfo_screenheight()//2}+{self.winfo_screenwidth()//4}+{self.winfo_screenheight()//4}"
        )

        # 初始化游戏数据
        self.count = load_victory_times() # 从文件加载胜利次数
        self.max_time = load_max() # 从文件加载最多步数
        self.min_time = load_min() # 从文件加载最少步数
        self.times = 0 # 猜测的次数
        self.generated = random.randint(1, 100) # 生成初始随机数

        self.entry = None # 创建entry成员
        self.hint_label = None # 提示词
        self.stats_label = None # 底部的统计数据

        self.create_widgets() # 在窗口创建所有控件
        self.update_stats() # 更新状态

    def create_widgets(self):
        # 提示文字
        tk.Label(self, text="系统生成了一个 1~100 的数字\n(超出100会自动取模，不支持负数)", font=("微软雅黑", 12)).pack(
            pady=10)

        # 输入框
        self.entry = tk.Entry(self, font=("微软雅黑", 14), justify="center")
        self.entry.pack(pady=5)
        self.entry.bind("<Return>", lambda event: self.check_guess())

        # 按钮区域
        btn_frame = tk.Frame(self)
        btn_frame.pack(pady=10)

        tk.Button(btn_frame, text="猜！", font=("微软雅黑", 12), width=10, command=self.check_guess).grid(row=0,
                                                                                                         column=0,
                                                                                                         padx=5)
        tk.Button(btn_frame, text="清空记录", font=("微软雅黑", 12), width=10, command=self.clear_history).grid(row=0,
                                                                                                                column=1,
                                                                                                                padx=5)

        # 动态提示文字 (比如 太大了/太小了)
        self.hint_label = tk.Label(self, text="请输入你的猜测", font=("微软雅黑", 12), fg="blue")
        self.hint_label.pack(pady=10)

        # 底部数据统计文字
        self.stats_label = tk.Label(self, text="", font=("微软雅黑", 10), fg="gray")
        self.stats_label.pack(side="bottom", pady=10)

    def update_stats(self):
        # 更新底部数据展示
        max_str = self.max_time if self.max_time is not None else "无"
        min_str = self.min_time if self.min_time is not None else "无"
        self.stats_label.config(text=f"胜场: {self.count} | 最大步数: {max_str} | 最小步数: {min_str}")

    def check_guess(self):
        user_input = self.entry.get().strip()
        if not user_input:
            return

        try:
            guess = int(user_input)
        except ValueError:
            messagebox.showerror("错误", "请输入数字！")
            return

        if guess < 0:
            self.hint_label.config(text=f"输入 {guess} 非法，请输入正整数", fg="red")
            return

        # 取模逻辑
        mapped_guess = (guess - 1) % 100 + 1

        self.times += 1

        if mapped_guess == self.generated:
            self.count += 1
            # 更新最值
            is_new_max = False
            is_new_min = False

            if self.max_time is None or self.times > self.max_time:
                self.max_time = self.times
                is_new_max = True
            if self.min_time is None or self.times < self.min_time:
                self.min_time = self.times
                is_new_min = True

            # 持久化保存
            save_victory_times(self.count)
            if is_new_max: save_max(self.max_time)
            if is_new_min: save_min(self.min_time)

            messagebox.showinfo("恭喜", f"猜对了！你用了 {self.times} 步。\n已生成新数字，继续猜吧！")
            self.generated = random.randint(1, 100)
            self.times = 0
        elif mapped_guess > self.generated:
            self.hint_label.config(text="太大了！", fg="red")
        else:
            self.hint_label.config(text="太小了！", fg="red")

        self.entry.delete(0, tk.END)  # 清空输入框
        self.update_stats()  # 刷新底部面板

    def clear_history(self):
        if messagebox.askyesno("确认", "确定要清空所有历史记录吗？"):
            self.count = 0
            self.max_time = None
            self.min_time = None
            clear_victory_times()
            clear_max()
            clear_min()
            self.update_stats()
            self.hint_label.config(text="历史记录已清空", fg="blue")