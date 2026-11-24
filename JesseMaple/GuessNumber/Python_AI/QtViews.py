from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel,
                               QPushButton, QLineEdit, QGridLayout, QColorDialog, QMessageBox, QSpacerItem, QSizePolicy)
from PySide6.QtCore import Qt
import random
from FileOperator import *
from QtThemeManager import PRESETS, get_contrast_color, rgb_to_hex


class GameView(QWidget):
    def __init__(self, app):
        super().__init__()
        self.app = app
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

    def setup_ui(self):
        layout = QVBoxLayout(self)

        # 顶部设置按钮
        top_layout = QHBoxLayout()
        btn_settings = QPushButton("⚙ 设置")
        btn_settings.setFixedSize(80, 30)
        btn_settings.clicked.connect(self.app.show_settings)
        top_layout.addWidget(btn_settings)
        top_layout.addStretch()
        layout.addLayout(top_layout)

        layout.addStretch()

        # 中间游戏区
        self.range_label = QLabel("系统生成了一个...")
        self.range_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.range_label)

        self.input_field = QLineEdit()
        self.input_field.setFixedSize(200, 40)
        self.input_field.setAlignment(Qt.AlignCenter)
        self.input_field.returnPressed.connect(self.submit_guess)
        layout.addWidget(self.input_field, alignment=Qt.AlignCenter)

        # 按钮区
        action_layout = QHBoxLayout()
        btn_guess = QPushButton("猜！")
        btn_guess.setFixedSize(100, 35)
        btn_guess.clicked.connect(self.submit_guess)

        btn_clear = QPushButton("清空记录")
        btn_clear.setFixedSize(100, 35)
        btn_clear.clicked.connect(self.handle_clear_records)

        action_layout.addStretch()
        action_layout.addWidget(btn_guess)
        action_layout.addWidget(btn_clear)
        action_layout.addStretch()
        layout.addLayout(action_layout)

        self.hint_label = QLabel("请输入你的猜测")
        self.hint_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.hint_label)

        # 难度折叠区
        self.diff_toggle_btn = QPushButton(f"选择难度 (当前: {self.get_diff_name()})")
        self.diff_toggle_btn.setFixedSize(200, 35)
        self.diff_toggle_btn.clicked.connect(self.expand_diffs)
        layout.addWidget(self.diff_toggle_btn, alignment=Qt.AlignCenter)

        # 隐藏的难度选项容器
        self.diff_options_widget = QWidget()
        diff_layout = QHBoxLayout(self.diff_options_widget)
        diff_layout.setContentsMargins(0, 0, 0, 0)
        difficulties = [("简单", "Easy"), ("中等", "Medium"), ("困难", "Hard"), ("难以置信", "Impossible")]
        for name, level in difficulties:
            btn = QPushButton(name)
            btn.setFixedSize(80, 30)
            btn.clicked.connect(lambda checked=False, l=level: self.change_difficulty(l))
            diff_layout.addWidget(btn)

        self.diff_options_widget.setVisible(False)
        layout.addWidget(self.diff_options_widget, alignment=Qt.AlignCenter)
        layout.addStretch()

        # 底部统计
        self.stats_label = QLabel("")
        self.stats_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.stats_label)

    def get_diff_name(self):
        names = {"Easy": "简单", "Medium": "中等", "Hard": "困难", "Impossible": "难以置信"}
        return names.get(self.difficulty, "简单")

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

        self.range_label.setText(
            f"系统生成了一个 {self.range_min}~{self.range_max} 的数字\n(超出会自动取模，不支持负数)")
        self.hint_label.setText("请输入你的猜测")
        self.hint_label.setStyleSheet("color: #4daafc;")
        self.input_field.clear()
        self.input_field.setFocus()

    def expand_diffs(self):
        self.diff_toggle_btn.setVisible(False)
        self.diff_options_widget.setVisible(True)

    def change_difficulty(self, level):
        self.difficulty = level
        self.diff_options_widget.setVisible(False)
        self.diff_toggle_btn.setText(f"选择难度 (当前: {self.get_diff_name()})")
        self.diff_toggle_btn.setVisible(True)
        self.load_difficulty_data()

    def refresh_stats_display(self):
        max_str = self.max_steps if self.max_steps is not None else "无"
        min_str = self.min_steps if self.min_steps is not None else "无"
        self.stats_label.setText(
            f"【{self.get_diff_name()}】胜场: {self.win_count} | 最大步数: {max_str} | 最小步数: {min_str}")

    def submit_guess(self):
        user_input = self.input_field.text().strip()
        if not user_input: return
        try:
            guess = int(user_input)
        except ValueError:
            QMessageBox.critical(self, "错误", "请输入数字！")
            return

        if guess < 0:
            self.hint_label.setText(f"输入 {guess} 非法")
            self.hint_label.setStyleSheet("color: red;")
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

            QMessageBox.information(self, "恭喜", f"猜对了！你用了 {self.current_attempts} 步。")
            self.start_new_round()
        else:
            if mapped_guess > self.target_number:
                self.hint_label.setText("太大了！")
            else:
                self.hint_label.setText("太小了！")
            self.hint_label.setStyleSheet("color: red;")
            self.input_field.clear()
            self.input_field.setFocus()
        self.refresh_stats_display()

    def handle_clear_records(self):
        reply = QMessageBox.question(self, '确认', f"确定要清空【{self.get_diff_name()}】难度的所有记录吗？",
                                     QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
        if reply == QMessageBox.Yes:
            self.win_count, self.max_steps, self.min_steps = 0, None, None
            reset_difficulty_stats(self.difficulty)
            self.refresh_stats_display()
            self.hint_label.setText("当前难度记录已清空")
            self.hint_label.setStyleSheet("color: #4daafc;")


class SettingsView(QWidget):
    def __init__(self, app):
        super().__init__()
        self.app = app
        self.theme_config = get_theme_config()
        self.setup_ui()

    def setup_ui(self):
        layout = QVBoxLayout(self)

        # 顶部返回
        top_layout = QHBoxLayout()
        btn_back = QPushButton("<- 返回")
        btn_back.setFixedSize(80, 30)
        btn_back.clicked.connect(self.app.show_main)
        top_layout.addWidget(btn_back)
        top_layout.addStretch()
        layout.addLayout(top_layout)

        # 内容区
        content_layout = QVBoxLayout()
        content_layout.setContentsMargins(40, 10, 40, 10)

        # 1. 选择模式
        content_layout.addWidget(QLabel("<b>【选择模式】</b>"))
        mode_layout = QHBoxLayout()
        modes = [("浅色", "light"), ("深色", "dark"), ("跟随系统", "system"), ("自定义", "custom")]
        self.mode_btns = {}
        for text, val in modes:
            btn = QPushButton(text)
            btn.setFixedSize(80, 30)
            btn.clicked.connect(lambda checked=False, m=val: self.change_mode(m))
            mode_layout.addWidget(btn)
            self.mode_btns[val] = btn
        mode_layout.addStretch()
        content_layout.addLayout(mode_layout)

        # 2. 选择预设
        content_layout.addWidget(QLabel("<b>【选择预设】</b>"))
        preset_layout = QHBoxLayout()
        self.preset_btns = {}
        for key, p_data in PRESETS.items():
            btn = QPushButton(p_data["name"])
            btn.setFixedSize(80, 30)
            # 按钮自身设置特定颜色
            btn.setStyleSheet(
                f"background-color: {rgb_to_hex(p_data['btn'])}; color: {get_contrast_color(rgb_to_hex(p_data['btn']))};")
            btn.clicked.connect(lambda checked=False, p=key: self.change_preset(p))
            preset_layout.addWidget(btn)
            self.preset_btns[key] = btn
        preset_layout.addStretch()
        content_layout.addLayout(preset_layout)

        # 3. 选择颜色
        content_layout.addWidget(QLabel("<b>【选择颜色】</b>"))
        color_grid = QGridLayout()

        color_grid.addWidget(QLabel("字体："), 0, 0)
        self.entry_font = QLineEdit()
        self.entry_font.setReadOnly(True)
        self.entry_font.setFixedWidth(120)
        self.entry_font.setAlignment(Qt.AlignCenter)
        color_grid.addWidget(self.entry_font, 0, 1)
        btn_font = QPushButton("选择")
        btn_font.setFixedSize(60, 25)
        btn_font.clicked.connect(lambda: self.open_color_picker("font"))
        color_grid.addWidget(btn_font, 0, 2)

        color_grid.addWidget(QLabel("背景："), 1, 0)
        self.entry_bg = QLineEdit()
        self.entry_bg.setReadOnly(True)
        self.entry_bg.setFixedWidth(120)
        self.entry_bg.setAlignment(Qt.AlignCenter)
        color_grid.addWidget(self.entry_bg, 1, 1)
        btn_bg = QPushButton("选择")
        btn_bg.setFixedSize(60, 25)
        btn_bg.clicked.connect(lambda: self.open_color_picker("bg"))
        color_grid.addWidget(btn_bg, 1, 2)

        color_grid.addWidget(QLabel("按钮："), 2, 0)
        self.entry_btn = QLineEdit()
        self.entry_btn.setReadOnly(True)
        self.entry_btn.setFixedWidth(120)
        self.entry_btn.setAlignment(Qt.AlignCenter)
        color_grid.addWidget(self.entry_btn, 2, 1)
        btn_btn = QPushButton("选择")
        btn_btn.setFixedSize(60, 25)
        btn_btn.clicked.connect(lambda: self.open_color_picker("btn"))
        color_grid.addWidget(btn_btn, 2, 2)

        content_layout.addLayout(color_grid)
        layout.addLayout(content_layout)
        layout.addStretch()

        self.refresh_ui()

    def refresh_ui(self):
        self.theme_config = get_theme_config()
        mode = self.theme_config["mode"]
        is_custom = (mode == "custom")

        for btn in self.preset_btns.values():
            btn.setEnabled(is_custom)

        self.entry_font.setText(self.theme_config.get("custom_font", "#000000"))
        self.entry_bg.setText(self.theme_config.get("custom_bg", "#f0f0f0"))
        self.entry_btn.setText(self.theme_config.get("custom_btn", "#e1e1e1"))

    def open_color_picker(self, target_type):
        if self.theme_config["mode"] != "custom":
            QMessageBox.information(self, "提示", "请先切换至【自定义】模式！")
            return

        current_color = {
            "font": self.theme_config.get("custom_font", "#000000"),
            "bg": self.theme_config.get("custom_bg", "#f0f0f0"),
            "btn": self.theme_config.get("custom_btn", "#e1e1e1")
        }[target_type]

        color = QColorDialog.getColor(Qt.black, self, "选择颜色")
        if color.isValid():
            hex_color = color.name()
            if target_type == "font":
                self.theme_config["custom_font"] = hex_color
            elif target_type == "bg":
                self.theme_config["custom_bg"] = hex_color
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
        self.theme_config["custom_font"] = get_contrast_color(bg_hex)
        self.apply_changes()

    def apply_changes(self):
        set_theme_config(self.theme_config)
        self.app.refresh_theme()