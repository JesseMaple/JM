import tkinter as tk
from tkinter import messagebox
import random
from FileOperator import *


class GuessGUI(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)
        self.title("猜数字游戏")
        self.geometry(
            f"{self.winfo_screenwidth() // 2}x{self.winfo_screenheight() // 2}+{self.winfo_screenwidth() // 4}+{self.winfo_screenheight() // 4}"
        )

        # 1. 初始化难度
        self.current_difficulty = load_current_difficulty()
        if self.current_difficulty not in ["Easy", "Medium", "Hard", "Impossible"]:
            self.current_difficulty = "Easy"

        # 2. 初始化变量
        self.count = 0
        self.max_time = None
        self.min_time = None
        self.times = 0
        self.generated = 0
        self.big_num = 0
        self.small_num = 0

        # UI 引用
        self.entry = None
        self.range_label = None
        self.hint_label = None
        self.stats_label = None
        self.btn_show_diff = None
        self.diff_btn_frame = None

        # 3. 创建控件
        self.create_widgets()

        # 4. 加载当前难度的数据并开始游戏
        self.load_stats_and_start()

        # 5. 绑定关闭事件
        self.protocol("WM_DELETE_WINDOW", self.on_closing)

    def load_stats_and_start(self):
        """根据当前难度，从文件中加载专属记录，并生成新数字"""
        # 拉取数据
        self.count = load_victory_times(self.current_difficulty)
        self.max_time = load_max(self.current_difficulty)
        self.min_time = load_min(self.current_difficulty)

        # 更新底部面板
        self.update_stats()
        # 重置游戏
        self.start_new_game()

    def start_new_game(self):
        """根据当前难度，生成新数字，重置状态"""
        self.times = 0
        if self.current_difficulty == "Easy":
            self.generated = random.randint(1, 10)
            self.big_num = 10
            self.small_num = 1
        elif self.current_difficulty == "Medium":
            self.generated = random.randint(1, 100)
            self.big_num = 100
            self.small_num = 1
        elif self.current_difficulty == "Hard":
            self.generated = random.randint(1, 10000)
            self.big_num = 10000
            self.small_num = 1
        elif self.current_difficulty == "Impossible":
            front = random.randint(1, 10000)
            back = random.randint(1, 10000)
            self.big_num = max(front, back)
            self.small_num = min(front, back)

        if self.range_label:
            self.range_label.config(
                text=f"系统生成了一个 {self.small_num}~{self.big_num} 的数字\n(超出会自动取模，不支持负数)")
        if self.hint_label:
            self.hint_label.config(text="请输入你的猜测", fg="blue")
        if self.entry:
            self.entry.delete(0, tk.END)

    def create_widgets(self):
        # 顶部范围提示
        self.range_label = tk.Label(self, text="", font=("微软雅黑", 12))
        self.range_label.pack(pady=10)

        # 输入框
        self.entry = tk.Entry(self, font=("微软雅黑", 14), justify="center")
        self.entry.pack(pady=5)
        self.entry.bind("<Return>", lambda event: self.check_guess())

        # 猜 / 清空 按钮区域
        btn_frame = tk.Frame(self)
        btn_frame.pack(pady=10)

        tk.Button(btn_frame, text="猜！", font=("微软雅黑", 12), width=10, command=self.check_guess).grid(row=0,
                                                                                                         column=0,
                                                                                                         padx=5)
        tk.Button(btn_frame, text="清空当前难度记录", font=("微软雅黑", 12), width=15, command=self.clear_history).grid(
            row=0, column=1, padx=5)

        # 动态提示文字
        self.hint_label = tk.Label(self, text="请输入你的猜测", font=("微软雅黑", 12), fg="blue")
        self.hint_label.pack(pady=10)

        # ================= 难度选择区域 =================
        self.diff_frame = tk.Frame(self)
        self.diff_frame.pack(pady=5)

        diff_names = {"Easy": "简单", "Medium": "中等", "Hard": "困难", "Impossible": "难以置信"}
        self.btn_show_diff = tk.Button(
            self.diff_frame,
            text=f"选择难度 (当前: {diff_names[self.current_difficulty]})",
            command=self.show_difficulty_menu
        )
        self.btn_show_diff.pack()

        self.diff_btn_frame = tk.Frame(self.diff_frame)

        difficulties = [("简单", "Easy"), ("中等", "Medium"), ("困难", "Hard"), ("难以置信", "Impossible")]
        for i, (name, level) in enumerate(difficulties):
            btn = tk.Button(self.diff_btn_frame, text=name, command=lambda l=level: self.set_difficulty(l))
            btn.grid(row=0, column=i, padx=5)

        # 底部数据统计文字
        self.stats_label = tk.Label(self, text="", font=("微软雅黑", 10), fg="gray")
        self.stats_label.pack(side="bottom", pady=15)

    def show_difficulty_menu(self):
        self.btn_show_diff.pack_forget()
        self.diff_btn_frame.pack()

    def set_difficulty(self, level):
        self.current_difficulty = level
        self.diff_btn_frame.pack_forget()
        diff_names = {"Easy": "简单", "Medium": "中等", "Hard": "困难", "Impossible": "难以置信"}
        self.btn_show_diff.config(text=f"选择难度 (当前: {diff_names[level]})")
        self.btn_show_diff.pack()

        # 【重点】：切换难度后，重新加载该难度的记录并重置游戏
        self.load_stats_and_start()

    def update_stats(self):
        max_str = self.max_time if self.max_time is not None else "无"
        min_str = self.min_time if self.min_time is not None else "无"
        diff_names = {"Easy": "简单", "Medium": "中等", "Hard": "困难", "Impossible": "难以置信"}
        # 底部面板现在会显示当前是哪个难度的记录
        self.stats_label.config(
            text=f"【{diff_names[self.current_difficulty]}】胜场: {self.count} | 最大: {max_str} | 最小: {min_str}")

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

        mapped_guess = guess
        if self.current_difficulty != "Easy":
            mapped_guess = (guess - 1) % self.big_num + 1
        else:
            # 修复了 Easy 难度取模可能变成 0 导致死循环的 Bug
            while mapped_guess > self.big_num or mapped_guess < self.small_num:
                mapped_guess = (mapped_guess * 5) % self.big_num
                if mapped_guess == 0:
                    mapped_guess = 1

        self.times += 1

        if mapped_guess == self.generated:
            self.count += 1
            is_new_max = False
            is_new_min = False

            if self.max_time is None or self.times > self.max_time:
                self.max_time = self.times
                is_new_max = True
            if self.min_time is None or self.times < self.min_time:
                self.min_time = self.times
                is_new_min = True

            # 保存数据时，传入当前难度
            save_victory_times(self.current_difficulty, self.count)
            if is_new_max: save_max(self.current_difficulty, self.max_time)
            if is_new_min: save_min(self.current_difficulty, self.min_time)

            messagebox.showinfo("恭喜", f"猜对了！你用了 {self.times} 步。\n已生成新数字，继续猜吧！")
            self.start_new_game()
        else:
            if mapped_guess > self.generated:
                self.hint_label.config(text="太大了！", fg="red")
            else:
                self.hint_label.config(text="太小了！", fg="red")
            self.entry.delete(0, tk.END)
            self.entry.focus()

        self.update_stats()

    def clear_history(self):
        diff_names = {"Easy": "简单", "Medium": "中等", "Hard": "困难", "Impossible": "难以置信"}
        if messagebox.askyesno("确认", f"确定要清空【{diff_names[self.current_difficulty]}】难度的所有记录吗？"):
            self.count = 0
            self.max_time = None
            self.min_time = None

            # 清空当前难度的记录
            clear_history(self.current_difficulty)

            self.update_stats()
            self.hint_label.config(text="当前难度记录已清空", fg="blue")

    def on_closing(self):
        answer = messagebox.askyesno("退出", "是否确认退出游戏？\n（系统会自动保存你的游戏进度）")
        if answer:
            save_difficulty(self.current_difficulty)
            # 退出前再次保存当前难度的数据，双重保险
            save_victory_times(self.current_difficulty, self.count)
            save_max(self.current_difficulty, self.max_time)
            save_min(self.current_difficulty, self.min_time)
            self.destroy()