import tkinter as tk
from tkinter import messagebox, colorchooser
import random
from FileOperator import *
from UIComponents import RoundedButton, PRESETS, lighten_hex, darken_hex, rgb_to_hex, get_contrast_color


class MainGameView(tk.Frame):
    def __init__(self, master, app_controller):
        super().__init__(master)
        self.app = app_controller

        self.difficulty = get_saved_difficulty()
        if self.difficulty not in ["Easy", "Medium", "Hard", "Impossible"]:
            self.difficulty = "Easy"

        self.win_count = 0
        self.max_steps = None
        self.min_steps = None
        self.current_attempts = 0
        self.target_number = 0
        self.range_max = 0
        self.range_min = 0

        self.setup_ui()
        self.load_difficulty_data()

    def get_colors(self):
        return self.app.colors

    def setup_ui(self):
        c = self.get_colors()
        self.configure(bg=c["app_bg"])

        RoundedButton(self, text="⚙ 设置", command=self.app.show_settings,
                      bg_color=c["btn_bg"], fg_color=c["btn_fg"],
                      hover_color=lighten_hex(c["btn_bg"]), press_color=darken_hex(c["btn_bg"]),
                      width=80, height=30).pack(anchor="nw", padx=10, pady=10)

        game_area = tk.Frame(self, bg=c["app_bg"])
        game_area.pack(expand=True)

        self.range_hint_label = tk.Label(game_area, text="", font=("微软雅黑", 12), bg=c["app_bg"], fg=c["text"])
        self.range_hint_label.pack(pady=10)

        self.input_entry = tk.Entry(game_area, font=("微软雅黑", 14), justify="center",
                                    bg=c["btn_bg"], fg=c["text"], insertbackground=c["text"])
        self.input_entry.pack(pady=5)
        self.input_entry.bind("<Return>", lambda event: self.submit_guess())

        action_frame = tk.Frame(game_area, bg=c["app_bg"])
        action_frame.pack(pady=10)

        RoundedButton(action_frame, text="猜！", command=self.submit_guess,
                      bg_color=c["btn_bg"], fg_color=c["btn_fg"], hover_color=lighten_hex(c["btn_bg"]),
                      press_color=darken_hex(c["btn_bg"]), width=100).grid(row=0, column=0, padx=10)
        RoundedButton(action_frame, text="清空记录", command=self.handle_clear_records,
                      bg_color=c["btn_bg"], fg_color=c["btn_fg"], hover_color=lighten_hex(c["btn_bg"]),
                      press_color=darken_hex(c["btn_bg"]), width=120).grid(row=0, column=1, padx=10)

        self.status_hint_label = tk.Label(game_area, text="请输入你的猜测", font=("微软雅黑", 12), bg=c["app_bg"],
                                          fg=c["info"])
        self.status_hint_label.pack(pady=10)

        self.diff_container = tk.Frame(game_area, bg=c["app_bg"])
        self.diff_container.pack(pady=5)

        diff_names = {"Easy": "简单", "Medium": "中等", "Hard": "困难", "Impossible": "难以置信"}
        self.diff_toggle_btn = RoundedButton(self.diff_container,
                                             text=f"选择难度 (当前: {diff_names[self.difficulty]})",
                                             command=self.expand_diffs, bg_color=c["btn_bg"], fg_color=c["btn_fg"],
                                             hover_color=lighten_hex(c["btn_bg"]), press_color=darken_hex(c["btn_bg"]),
                                             width=200)
        self.diff_toggle_btn.pack()

        self.diff_options_frame = tk.Frame(self.diff_container, bg=c["app_bg"])
        difficulties = [("简单", "Easy"), ("中等", "Medium"), ("困难", "Hard"), ("难以置信", "Impossible")]
        for i, (name, level) in enumerate(difficulties):
            RoundedButton(self.diff_options_frame, text=name, command=lambda l=level: self.change_difficulty(l),
                          bg_color=c["btn_bg"], fg_color=c["btn_fg"], hover_color=lighten_hex(c["btn_bg"]),
                          press_color=darken_hex(c["btn_bg"]), width=80).grid(row=0, column=i, padx=3)

        self.stats_info_label = tk.Label(self, text="", font=("微软雅黑", 10), bg=c["app_bg"], fg=c["text"])
        self.stats_info_label.pack(side="bottom", pady=15)

    def load_difficulty_data(self):
        self.win_count = get_win_count(self.difficulty)
        self.max_steps = get_max_steps(self.difficulty)
        self.min_steps = get_min_steps(self.difficulty)
        self.refresh_stats_display()
        self.start_new_round()

    def start_new_round(self):
        self.current_attempts = 0
        if self.difficulty == "Easy":
            self.target_number, self.range_max, self.range_min = random.randint(1, 10), 10, 1
        elif self.difficulty == "Medium":
            self.target_number, self.range_max, self.range_min = random.randint(1, 100), 100, 1
        elif self.difficulty == "Hard":
            self.target_number, self.range_max, self.range_min = random.randint(1, 10000), 10000, 1
        elif self.difficulty == "Impossible":
            front, back = random.randint(1, 10000), random.randint(1, 10000)
            self.range_max, self.range_min = max(front, back), min(front, back)
            self.target_number = random.randint(self.range_min, self.range_max)

        self.range_hint_label.config(
            text=f"系统生成了一个 {self.range_min}~{self.range_max} 的数字\n(超出会自动取模，不支持负数)")
        self.status_hint_label.config(text="请输入你的猜测", fg=self.get_colors()["info"])
        self.input_entry.delete(0, tk.END)

    def expand_diffs(self):
        self.diff_toggle_btn.pack_forget()
        self.diff_options_frame.pack()

    def change_difficulty(self, level):
        self.difficulty = level
        self.diff_options_frame.pack_forget()
        diff_names = {"Easy": "简单", "Medium": "中等", "Hard": "困难", "Impossible": "难以置信"}
        self.diff_toggle_btn.itemconfig(self.diff_toggle_btn.text_id, text=f"选择难度 (当前: {diff_names[level]})")
        self.diff_toggle_btn.pack()
        self.load_difficulty_data()

    def refresh_stats_display(self):
        max_str = self.max_steps if self.max_steps is not None else "无"
        min_str = self.min_steps if self.min_steps is not None else "无"
        diff_names = {"Easy": "简单", "Medium": "中等", "Hard": "困难", "Impossible": "难以置信"}
        self.stats_info_label.config(
            text=f"【{diff_names[self.difficulty]}】胜场: {self.win_count} | 最大步数: {max_str} | 最小步数: {min_str}")

    def submit_guess(self):
        user_input = self.input_entry.get().strip()
        if not user_input: return
        try:
            guess = int(user_input)
        except ValueError:
            messagebox.showerror("错误", "请输入数字！")
            return
        if guess < 0:
            self.status_hint_label.config(text=f"输入 {guess} 非法", fg="red")
            return

        mapped_guess = guess
        if self.difficulty != "Easy":
            mapped_guess = (guess - 1) % self.range_max + 1
        else:
            while mapped_guess > self.range_max or mapped_guess < self.range_min:
                mapped_guess = (mapped_guess * 5) % self.range_max
                if mapped_guess == 0: mapped_guess = 1

        self.current_attempts += 1

        if mapped_guess == self.target_number:
            self.win_count += 1
            is_new_max = self.max_steps is None or self.current_attempts > self.max_steps
            is_new_min = self.min_steps is None or self.current_attempts < self.min_steps
            if is_new_max: self.max_steps = self.current_attempts
            if is_new_min: self.min_steps = self.current_attempts

            set_win_count(self.difficulty, self.win_count)
            if is_new_max: set_max_steps(self.difficulty, self.max_steps)
            if is_new_min: set_min_steps(self.difficulty, self.min_steps)

            messagebox.showinfo("恭喜", f"猜对了！你用了 {self.current_attempts} 步。")
            self.start_new_round()
        else:
            if mapped_guess > self.target_number:
                self.status_hint_label.config(text="太大了！", fg="red")
            else:
                self.status_hint_label.config(text="太小了！", fg="red")
            self.input_entry.delete(0, tk.END)
            self.input_entry.focus()
        self.refresh_stats_display()

    def handle_clear_records(self):
        diff_names = {"Easy": "简单", "Medium": "中等", "Hard": "困难", "Impossible": "难以置信"}
        if messagebox.askyesno("确认", f"确定要清空【{diff_names[self.difficulty]}】难度的所有记录吗？"):
            self.win_count, self.max_steps, self.min_steps = 0, None, None
            reset_difficulty_stats(self.difficulty)
            self.refresh_stats_display()
            self.status_hint_label.config(text="当前难度记录已清空", fg=self.get_colors()["info"])


class SettingsView(tk.Frame):
    def __init__(self, master, app_controller):
        super().__init__(master)
        self.app = app_controller
        self.theme_config = get_theme_config()
        self.setup_ui()

    def get_colors(self):
        return self.app.colors

    def setup_ui(self):
        c = self.get_colors()
        self.configure(bg=c["app_bg"])

        # 顶部返回区
        top_bar = tk.Frame(self, bg=c["app_bg"])
        top_bar.pack(fill="x", pady=10, padx=10)
        RoundedButton(top_bar, text="<- 返回", command=self.app.show_main,
                      bg_color=c["btn_bg"], fg_color=c["btn_fg"], hover_color=lighten_hex(c["btn_bg"]),
                      press_color=darken_hex(c["btn_bg"]), width=80, height=30).pack(side="left")

        content = tk.Frame(self, bg=c["app_bg"])
        content.pack(expand=True, fill="both", padx=40)

        # --- 功能区 1：选择模式 ---
        tk.Label(content, text="【选择模式】", font=("微软雅黑", 12, "bold"), bg=c["app_bg"], fg=c["text"]).pack(
            anchor="w", pady=(10, 5))
        mode_frame = tk.Frame(content, bg=c["app_bg"])
        mode_frame.pack(anchor="w", pady=5)

        modes = [("浅色", "light"), ("深色", "dark"), ("跟随系统", "system"), ("自定义", "custom")]
        self.mode_btns = {}
        for text, mode_val in modes:
            btn = RoundedButton(mode_frame, text=text, command=lambda m=mode_val: self.change_mode(m),
                                bg_color=c["btn_bg"], fg_color=c["btn_fg"], hover_color=lighten_hex(c["btn_bg"]),
                                press_color=darken_hex(c["btn_bg"]), width=80, height=30)
            btn.pack(side="left", padx=(0, 10))
            self.mode_btns[mode_val] = btn

        # --- 功能区 2：选择预设 ---
        tk.Label(content, text="【选择预设】", font=("微软雅黑", 12, "bold"), bg=c["app_bg"], fg=c["text"]).pack(
            anchor="w", pady=(20, 5))
        self.preset_frame = tk.Frame(content, bg=c["app_bg"])
        self.preset_frame.pack(anchor="w", pady=5)

        self.preset_btns = {}
        for key, p_data in PRESETS.items():
            btn = RoundedButton(self.preset_frame, text=p_data["name"], command=lambda p=key: self.change_preset(p),
                                bg_color=rgb_to_hex(p_data["btn"]),
                                fg_color=get_contrast_color(rgb_to_hex(p_data["btn"])),
                                hover_color=lighten_hex(rgb_to_hex(p_data["btn"])),
                                press_color=darken_hex(rgb_to_hex(p_data["btn"])), width=80, height=30)
            btn.pack(side="left", padx=(0, 10))
            self.preset_btns[key] = btn

        # --- 功能区 3：选择颜色 (重构) ---
        tk.Label(content, text="【选择颜色】", font=("微软雅黑", 12, "bold"), bg=c["app_bg"], fg=c["text"]).pack(
            anchor="w", pady=(20, 5))
        color_frame = tk.Frame(content, bg=c["app_bg"])
        color_frame.pack(anchor="w", pady=5)

        # 字体颜色行
        tk.Label(color_frame, text="字体：", bg=c["app_bg"], fg=c["text"], font=("微软雅黑", 10), width=5,
                 anchor="w").grid(row=0, column=0, pady=5)
        self.entry_font = tk.Entry(color_frame, width=15, state="readonly", justify="center")
        self.entry_font.grid(row=0, column=1, padx=10)
        RoundedButton(color_frame, text="选择", command=lambda: self.open_color_picker("font"),
                      bg_color=c["btn_bg"], fg_color=c["btn_fg"], hover_color=lighten_hex(c["btn_bg"]),
                      press_color=darken_hex(c["btn_bg"]),
                      width=60, height=25).grid(row=0, column=2)

        # 背景颜色行
        tk.Label(color_frame, text="背景：", bg=c["app_bg"], fg=c["text"], font=("微软雅黑", 10), width=5,
                 anchor="w").grid(row=1, column=0, pady=5)
        self.entry_bg = tk.Entry(color_frame, width=15, state="readonly", justify="center")
        self.entry_bg.grid(row=1, column=1, padx=10)
        RoundedButton(color_frame, text="选择", command=lambda: self.open_color_picker("bg"),
                      bg_color=c["btn_bg"], fg_color=c["btn_fg"], hover_color=lighten_hex(c["btn_bg"]),
                      press_color=darken_hex(c["btn_bg"]),
                      width=60, height=25).grid(row=1, column=2)

        # 按钮颜色行
        tk.Label(color_frame, text="按钮：", bg=c["app_bg"], fg=c["text"], font=("微软雅黑", 10), width=5,
                 anchor="w").grid(row=2, column=0, pady=5)
        self.entry_btn = tk.Entry(color_frame, width=15, state="readonly", justify="center")
        self.entry_btn.grid(row=2, column=1, padx=10)
        RoundedButton(color_frame, text="选择", command=lambda: self.open_color_picker("btn"),
                      bg_color=c["btn_bg"], fg_color=c["btn_fg"], hover_color=lighten_hex(c["btn_bg"]),
                      press_color=darken_hex(c["btn_bg"]),
                      width=60, height=25).grid(row=2, column=2)

        self.update_ui_state()

    def update_ui_state(self):
        mode = self.theme_config["mode"]
        is_custom = (mode == "custom")

        for btn in self.preset_btns.values():
            btn.config(cursor="hand2" if is_custom else "arrow")

        self.entry_font.config(state="normal")
        self.entry_font.delete(0, tk.END)
        self.entry_font.insert(0, self.theme_config.get("custom_font", "#000000"))
        self.entry_font.config(state="readonly")

        self.entry_bg.config(state="normal")
        self.entry_bg.delete(0, tk.END)
        self.entry_bg.insert(0, self.theme_config.get("custom_bg", "#f0f0f0"))
        self.entry_bg.config(state="readonly")

        self.entry_btn.config(state="normal")
        self.entry_btn.delete(0, tk.END)
        self.entry_btn.insert(0, self.theme_config.get("custom_btn", "#e1e1e1"))
        self.entry_btn.config(state="readonly")

    def open_color_picker(self, target_type):
        """呼出调色盘"""
        if self.theme_config["mode"] != "custom":
            messagebox.showinfo("提示", "请先切换至【自定义】模式！", parent=self)
            return

        current_color = {
            "font": self.theme_config.get("custom_font", "#000000"),
            "bg": self.theme_config.get("custom_bg", "#f0f0f0"),
            "btn": self.theme_config.get("custom_btn", "#e1e1e1")
        }[target_type]

        color_result = colorchooser.askcolor(initialcolor=current_color, title="选择颜色", parent=self)

        if color_result[1] is not None:
            hex_color = color_result[1]

            if target_type == "font":
                self.theme_config["custom_font"] = hex_color
            elif target_type == "bg":
                self.theme_config["custom_bg"] = hex_color
                # 如果修改了背景，自动使用对比度色作为字体颜色，防止看不清
                self.theme_config["custom_font"] = get_contrast_color(hex_color)
            elif target_type == "btn":
                self.theme_config["custom_btn"] = hex_color

            self.theme_config["preset"] = "none"
            self.apply_changes()

    def change_mode(self, mode):
        self.theme_config["mode"] = mode
        if mode != "custom":
            self.theme_config["preset"] = "none"
        else:
            if self.theme_config.get("preset") == "none":
                self.theme_config["preset"] = "yellow"
                bg_hex = rgb_to_hex(PRESETS["yellow"]["bg"])
                self.theme_config["custom_bg"] = bg_hex
                self.theme_config["custom_btn"] = rgb_to_hex(PRESETS["yellow"]["btn"])
                self.theme_config["custom_font"] = get_contrast_color(bg_hex)
        self.apply_changes()

    def change_preset(self, preset):
        if self.theme_config["mode"] != "custom":
            return
        self.theme_config["preset"] = preset

        p = PRESETS[preset]
        bg_hex = rgb_to_hex(p["bg"])
        btn_hex = rgb_to_hex(p["btn"])

        self.theme_config["custom_bg"] = bg_hex
        self.theme_config["custom_btn"] = btn_hex
        # 选择预设时，自动计算背景色的对比度字体
        self.theme_config["custom_font"] = get_contrast_color(bg_hex)

        self.apply_changes()

    def apply_changes(self):
        set_theme_config(self.theme_config)
        self.app.refresh_theme()