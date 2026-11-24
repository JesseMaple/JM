import winreg

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
    """根据背景亮度自动返回黑色或白色"""
    try:
        rgb = tuple(int(hex_color[i:i + 2], 16) for i in (1, 3, 5))
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


def get_theme_qss(config):
    """核心：根据配置生成全局 QSS 样式表"""
    mode = config.get("mode", "light")

    # 默认浅色
    bg = "#f0f0f0"
    text = "#000000"
    btn_bg = "#e1e1e1"
    btn_fg = "#000000"

    if mode == "dark" or (mode == "system" and is_system_dark()):
        bg = "#2b2b2b"
        text = "#ffffff"
        btn_bg = "#3c3f41"
        btn_fg = "#ffffff"
    elif mode == "custom":
        c_bg = config.get("custom_bg", "#f0f0f0")
        c_font = config.get("custom_font", "#000000")
        c_btn = config.get("custom_btn", "#e1e1e1")

        bg = c_bg
        text = c_font
        btn_bg = c_btn
        btn_fg = get_contrast_color(c_btn)  # 自动计算按钮文字颜色

    # 返回一段类似 CSS 的字符串，Qt 会自动渲染
    return f"""
        QWidget {{
            background-color: {bg};
            color: {text};
            font-family: "微软雅黑";
            font-size: 14px;
        }}
        QPushButton {{
            background-color: {btn_bg};
            color: {btn_fg};
            border-radius: 8px; /* 圆角！ */
            padding: 8px 16px;
            border: none;
        }}
        QPushButton:hover {{
            background-color: {btn_bg}DD; /* 模拟变亮 */
        }}
        QPushButton:pressed {{
            background-color: {btn_bg}99; /* 模拟变暗 */
        }}
        QLineEdit {{
            background-color: {btn_bg};
            color: {text};
            border-radius: 6px;
            padding: 5px;
            border: 1px solid {btn_bg};
        }}
        QLabel {{
            background-color: transparent;
        }}
    """