import json
import sys
from pathlib import Path

if getattr(sys, 'frozen', False):
    base_path = Path(sys.executable).parent
else:
    base_path = Path(__file__).parent

file_path = base_path / "GameData.json"


def _load():
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            data = json.load(f)

            # 【数据升级/迁移】：如果你的 json 还是旧版本的扁平结构，自动转换成新结构
            if "records" not in data:
                old_diff = data.get("difficulty", "Easy")
                old_rec = {
                    "victory_times": data.get("victory_times", 0),
                    "max_time": data.get("max_time", None),
                    "min_time": data.get("min_time", None)
                }
                data = {
                    "current_difficulty": old_diff,
                    "records": {old_diff: old_rec}
                }
            return data
    except (FileNotFoundError, json.JSONDecodeError):
        return {"current_difficulty": "Easy", "records": {}}


def _save(data):
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=4)


# --- 当前难度 ---
def load_current_difficulty():
    return _load().get("current_difficulty", "Easy")


def save_difficulty(difficulty):
    data = _load()
    data["current_difficulty"] = difficulty
    _save(data)


# --- 内部辅助函数：获取某个难度的记录 ---
def _get_diff_record(data, difficulty):
    if "records" not in data:
        data["records"] = {}
    if difficulty not in data["records"]:
        data["records"][difficulty] = {
            "victory_times": 0,
            "max_time": None,
            "min_time": None
        }
    return data["records"][difficulty]


# --- 具体数据的读写（现在都需要传入 difficulty 参数） ---
def load_victory_times(difficulty):
    return _get_diff_record(_load(), difficulty).get("victory_times", 0)


def save_victory_times(difficulty, v):
    data = _load()
    _get_diff_record(data, difficulty)["victory_times"] = v
    _save(data)


def load_max(difficulty):
    return _get_diff_record(_load(), difficulty).get("max_time", None)


def save_max(difficulty, v):
    data = _load()
    _get_diff_record(data, difficulty)["max_time"] = v
    _save(data)


def load_min(difficulty):
    return _get_diff_record(_load(), difficulty).get("min_time", None)


def save_min(difficulty, v):
    data = _load()
    _get_diff_record(data, difficulty)["min_time"] = v
    _save(data)


# --- 清空特定难度的记录 ---
def clear_history(difficulty):
    data = _load()
    if "records" in data and difficulty in data["records"]:
        del data["records"][difficulty]
    _save(data)