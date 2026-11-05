import tkinter as tk
import winreg

# 移除了预设里的固定 fg（字体颜色），全部由背景亮度自动计算
PRESETS = {
    "yellow": {"name": "明黄", "bg": (255, 253, 230), "btn": (255, 215, 0)},
    "violet": {"name": "紫罗兰", "bg": (245, 240, 255), "btn": (138, 43, 226)},
    "sakura": {"name": "樱花粉", "bg": (255, 240, 245), "btn": (255, 105, 180)},
    "deepseek": {"name": "深空蓝", "bg": (240, 248, 255), "btn": (77, 107, 254)},
    "cyan": {"name": "青色", "bg": (240, 255, 255), "btn": (0, 206, 209)}
}

def rgb_to_hex(rgb):
    return f"#{rgb[0]:02x}{rgb[1]:02x}{rgb[2]:02x}"

def get_contrast_color(hex_color):
    """根据十六进制背景颜色的亮度，自动返回黑色或白色字体"""
    try:
        rgb = tuple(int(hex_color[i:i + 2], 16) for i in (1, 3, 5))
        # 标准亮度公式：Y = 0.299*R + 0.587*G + 0.114*B
        luminance = (0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2]) / 255
        return "#000000" if luminance > 0.5 else "#ffffff"
    except:
        return "#000000"

def is_system_dark():
    try:
        key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize")
        value, _ = winreg.QueryValueEx(key, "AppsUseLightTheme")
        return value == 0
    except:
        return False

class ThemeManager:
    @staticmethod
    def get_theme_colors(config):
        mode = config.get("mode", "light")
        preset = config.get("preset", "none")

        # 获取十六进制的自定义颜色
        c_font = config.get("custom_font", "#000000")
        c_bg = config.get("custom_bg", "#f0f0f0")
        c_btn = config.get("custom_btn", "#e1e1e1")

        colors = {
            "app_bg": "#f0f0f0", "text": "#000000", "btn_bg": "#e1e1e1", "btn_fg": "#000000",
            "success": "#008000", "error": "#ff0000", "info": "blue"
        }

        if mode == "dark":
            colors.update(
                {"app_bg": "#2b2b2b", "text": "#ffffff", "btn_bg": "#3c3f41", "btn_fg": "#ffffff", "info": "#4daafc"})
        elif mode == "system":
            if is_system_dark():
                colors.update({"app_bg": "#2b2b2b", "text": "#ffffff", "btn_bg": "#3c3f41", "btn_fg": "#ffffff",
                               "info": "#4daafc"})
        elif mode == "custom":
            colors.update({
                "app_bg": c_bg,
                "text": c_font,
                "btn_bg": c_btn,
                "btn_fg": get_contrast_color(c_btn), # 按钮上的文字自适应对比度
                "info": c_btn
            })
        return colors


class RoundedButton(tk.Canvas):
    def __init__(self, master, text, command, bg_color, fg_color, hover_color, press_color, width=120, height=35,
                 radius=10, **kwargs):
        super().__init__(master, width=width, height=height, bg=master["bg"], highlightthickness=0, **kwargs)
        self.command = command
        self.bg_color = bg_color
        self.fg_color = fg_color
        self.hover_color = hover_color
        self.press_color = press_color
        self.radius = radius

        self.rect = self._create_rounded_rect(1, 1, width - 1, height - 1, radius, fill=self.bg_color)
        self.text_id = self.create_text(width / 2, height / 2, text=text, fill=self.fg_color, font=("微软雅黑", 11))

        self.bind('<Enter>', self._on_enter)
        self.bind('<Leave>', self._on_leave)
        self.bind('<ButtonPress-1>', self._on_press)
        self.bind('<ButtonRelease-1>', self._on_release)

    def _create_rounded_rect(self, x1, y1, x2, y2, r, **kwargs):
        points = [x1 + r, y1, x1 + r, y1, x2 - r, y1, x2 - r, y1, x2, y1, x2, y1 + r, x2, y1 + r,
                  x2, y2 - r, x2, y2 - r, x2, y2, x2 - r, y2, x2 - r, y2, x1 + r, y2, x1 + r, y2,
                  x1, y2, x1, y2 - r, x1, y2 - r, x1, y1 + r, x1, y1 + r, x1, y1]
        return self.create_polygon(points, smooth=True, **kwargs)

    def _on_enter(self, e):
        self.itemconfig(self.rect, fill=self.hover_color)
        self.config(cursor="hand2")

    def _on_leave(self, e):
        self.itemconfig(self.rect, fill=self.bg_color)
        self.config(cursor="")

    def _on_press(self, e):
        self.itemconfig(self.rect, fill=self.press_color)

    def _on_release(self, e):
        self.itemconfig(self.rect, fill=self.hover_color)
        if self.command:
            self.command()

def lighten_hex(hex_color, factor=0.2):
    rgb = tuple(int(hex_color[i:i + 2], 16) for i in (1, 3, 5))
    new_rgb = tuple(min(255, int(c + (255 - c) * factor)) for c in rgb)
    return rgb_to_hex(new_rgb)

def darken_hex(hex_color, factor=0.2):
    rgb = tuple(int(hex_color[i:i + 2], 16) for i in (1, 3, 5))
    new_rgb = tuple(max(0, int(c * (1 - factor))) for c in rgb)
    return rgb_to_hex(new_rgb)