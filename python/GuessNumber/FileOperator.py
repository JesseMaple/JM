import json

file_path = "GameData.json"

_DEFAULT = {"victory_times": 0, "max_time": None, "min_time": None}


def _load():
    """读整个文件，文件不存在/内容坏了就返回默认值"""
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            data = json.load(f)
    except FileNotFoundError:
        data = {}
    except json.JSONDecodeError:
        data = {}
    return {**_DEFAULT, **data}          # 补齐缺失字段，防止旧文件没新字段


def _save(data):
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False)


# ---- 读 ----
def load_victory_times():
    return _load()["victory_times"]

def load_max():
    return _load()["max_time"]

def load_min():
    return _load()["min_time"]


# ---- 写（读-改-写，只动一个字段）----
def save_victory_times(victory_times):
    data = _load()
    data["victory_times"] = victory_times
    _save(data)

def save_max(max_time):
    data = _load()
    data["max_time"] = max_time
    _save(data)

def save_min(min_time):
    data = _load()
    data["min_time"] = min_time
    _save(data)


# ---- 清空（等价于把该字段写回默认值）----
def clear_victory_times():
    save_victory_times(_DEFAULT["victory_times"])

def clear_max():
    save_max(_DEFAULT["max_time"])

def clear_min():
    save_min(_DEFAULT["min_time"])