import json
import sys
from pathlib import Path

# 判断是否为打包后的 exe 环境，确定基准路径
if getattr(sys, 'frozen', False):
    base_path = Path(sys.executable).parent
else:
    base_path = Path(__file__).parent

file_path = base_path / "GameData.json"


def _read_data():
    """内部方法：读取整个 JSON 数据，并处理数据迁移"""
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            data = json.load(f)

            # 数据迁移：如果发现是老版本的扁平结构，自动转成新版嵌套结构
            if "records" not in data:
                old_diff = data.get("difficulty", "Easy")
                old_rec = {
                    "win_count": data.get("victory_times", 0),
                    "max_steps": data.get("max_time", None),
                    "min_steps": data.get("min_time", None)
                }
                data = {"current_difficulty": old_diff, "records": {old_diff: old_rec}}

            # 确保主题配置里有自定义颜色的字段
            if "theme" not in data:
                data["theme"] = {
                    "mode": "light", "preset": "none",
                    "custom_font": "#000000", "custom_bg": "#f0f0f0", "custom_btn": "#e1e1e1"
                }
            else:
                # 兼容旧版配置升级：如果老 JSON 里没这三个颜色字段，补上默认值
                if "custom_font" not in data["theme"]:
                    data["theme"]["custom_font"] = "#000000"
                    data["theme"]["custom_bg"] = "#f0f0f0"
                    data["theme"]["custom_btn"] = "#e1e1e1"
            return data
    except (FileNotFoundError, json.JSONDecodeError):
        # 如果文件不存在或者内容坏了，返回全量默认数据
        return {"current_difficulty": "Easy", "records": {}, "theme": {
            "mode": "light", "preset": "none",
            "custom_font": "#000000", "custom_bg": "#f0f0f0", "custom_btn": "#e1e1e1"
        }}


def _write_data(data):
    """内部方法：写入整个 JSON 数据，格式化排版"""
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=4)


def get_saved_difficulty():
    """读取上次保存的难度"""
    return _read_data().get("current_difficulty", "Easy")


def set_saved_difficulty(difficulty):
    """保存当前难度"""
    data = _read_data()
    data["current_difficulty"] = difficulty
    _write_data(data)


def get_theme_config():
    """获取完整的主题配置"""
    return _read_data().get("theme")


def set_theme_config(theme_config):
    """写入完整的主题配置"""
    data = _read_data()
    data["theme"] = theme_config
    _write_data(data)


def _get_difficulty_record(data, difficulty):
    """内部辅助方法：安全地获取某个难度的数据记录，没有就自动建个空壳"""
    if "records" not in data:
        data["records"] = {}
    if difficulty not in data["records"]:
        data["records"][difficulty] = {"win_count": 0, "max_steps": None, "min_steps": None}
    return data["records"][difficulty]


def get_win_count(difficulty):
    """获取指定难度的胜场次数"""
    return _get_difficulty_record(_read_data(), difficulty).get("win_count", 0)


def set_win_count(difficulty, count):
    """保存指定难度的胜场次数"""
    data = _read_data()
    _get_difficulty_record(data, difficulty)["win_count"] = count
    _write_data(data)


def get_max_steps(difficulty):
    """获取指定难度的最大步数"""
    return _get_difficulty_record(_read_data(), difficulty).get("max_steps", None)


def set_max_steps(difficulty, steps):
    """保存指定难度的最大步数"""
    data = _read_data()
    _get_difficulty_record(data, difficulty)["max_steps"] = steps
    _write_data(data)


def get_min_steps(difficulty):
    """获取指定难度的最小步数"""
    return _get_difficulty_record(_read_data(), difficulty).get("min_steps", None)


def set_min_steps(difficulty, steps):
    """保存指定难度的最小步数"""
    data = _read_data()
    _get_difficulty_record(data, difficulty)["min_steps"] = steps
    _write_data(data)


def reset_difficulty_stats(difficulty):
    """清空指定难度的全部历史记录"""
    data = _read_data()
    if "records" in data and difficulty in data["records"]:
        del data["records"][difficulty]
    _write_data(data)