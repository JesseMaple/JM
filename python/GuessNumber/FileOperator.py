import json
import sys
from pathlib import Path

if getattr(sys, 'frozen', False):
    base_path = Path(sys.executable).parent
else:
    base_path = Path(__file__).parent

file_path = base_path / "GameData.json"


def _read_data():
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            data = json.load(f)
            if "records" not in data:
                old_diff = data.get("difficulty", "Easy")
                old_rec = {
                    "win_count": data.get("victory_times", 0),
                    "max_steps": data.get("max_time", None),
                    "min_steps": data.get("min_time", None)
                }
                data = {"current_difficulty": old_diff, "records": {old_diff: old_rec}}

            # 确保主题配置包含自定义颜色
            if "theme" not in data:
                data["theme"] = {
                    "mode": "light", "preset": "none",
                    "custom_font": "#000000", "custom_bg": "#f0f0f0", "custom_btn": "#e1e1e1"
                }
            else:
                # 兼容旧版配置升级
                if "custom_font" not in data["theme"]:
                    data["theme"]["custom_font"] = "#000000"
                    data["theme"]["custom_bg"] = "#f0f0f0"
                    data["theme"]["custom_btn"] = "#e1e1e1"
            return data
    except (FileNotFoundError, json.JSONDecodeError):
        return {"current_difficulty": "Easy", "records": {}, "theme": {
            "mode": "light", "preset": "none",
            "custom_font": "#000000", "custom_bg": "#f0f0f0", "custom_btn": "#e1e1e1"
        }}


def _write_data(data):
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=4)


def get_saved_difficulty():
    return _read_data().get("current_difficulty", "Easy")


def set_saved_difficulty(difficulty):
    data = _read_data()
    data["current_difficulty"] = difficulty
    _write_data(data)


def get_theme_config():
    return _read_data().get("theme")


def set_theme_config(theme_config):
    data = _read_data()
    data["theme"] = theme_config
    _write_data(data)


def _get_difficulty_record(data, difficulty):
    if "records" not in data:
        data["records"] = {}
    if difficulty not in data["records"]:
        data["records"][difficulty] = {"win_count": 0, "max_steps": None, "min_steps": None}
    return data["records"][difficulty]


def get_win_count(difficulty):
    return _get_difficulty_record(_read_data(), difficulty).get("win_count", 0)


def set_win_count(difficulty, count):
    data = _read_data()
    _get_difficulty_record(data, difficulty)["win_count"] = count
    _write_data(data)


def get_max_steps(difficulty):
    return _get_difficulty_record(_read_data(), difficulty).get("max_steps", None)


def set_max_steps(difficulty, steps):
    data = _read_data()
    _get_difficulty_record(data, difficulty)["max_steps"] = steps
    _write_data(data)


def get_min_steps(difficulty):
    return _get_difficulty_record(_read_data(), difficulty).get("min_steps", None)


def set_min_steps(difficulty, steps):
    data = _read_data()
    _get_difficulty_record(data, difficulty)["min_steps"] = steps
    _write_data(data)


def reset_difficulty_stats(difficulty):
    data = _read_data()
    if "records" in data and difficulty in data["records"]:
        del data["records"][difficulty]
    _write_data(data)