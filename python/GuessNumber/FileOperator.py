import json
import sys
from pathlib import Path

if getattr(sys, 'frozen', False):
    # 打包模式：以 exe 所在目录为基准
    base_path = Path(sys.executable).parent
else:
    # 开发模式：以脚本所在目录为基准
    base_path = Path(__file__).parent

file_path = base_path / "GameData.json"

def _load():
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            return json.load(f)
    except (FileNotFoundError, json.JSONDecodeError):
        return {}

def _save(data):
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(data, f)

def load_victory_times():
    return _load().get("victory_times", 0)

def load_max():
    return _load().get("max_time", None)

def load_min():
    return _load().get("min_time", None)

def save_victory_times(v):
    data = _load()
    data["victory_times"] = v
    _save(data)

def save_max(v):
    data = _load()
    data["max_time"] = v
    _save(data)

def save_min(v):
    data = _load()
    data["min_time"] = v
    _save(data)

def clear_victory_times():
    data = _load()
    if "victory_times" in data: del data["victory_times"]
    _save(data)

def clear_max():
    data = _load()
    if "max_time" in data: del data["max_time"]
    _save(data)

def clear_min():
    data = _load()
    if "min_time" in data: del data["min_time"]
    _save(data)