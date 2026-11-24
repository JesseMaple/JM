// GameData.h
//
// 对应 Python 版 FileOperator.py：
//   * 一个极简 JSON 实现（只服务 GameData.json，不引入第三方库）
//   * GameStore 负责读写 GameData.json，字段名与 Python 版完全一致，
//     这样 C++ 版和 Python 版可以共用同一份存档。
//
#pragma once

#include "framework.h"

// ---------------------------------------------------------------- mini JSON
namespace json {

enum class Type { Null, Bool, Number, String, Array, Object };

struct Value {
    Type type = Type::Null;
    bool boolean = false;
    double number = 0.0;
    std::string text;                                    // UTF-8
    std::vector<Value> items;                            // Array 元素
    std::vector<std::pair<std::string, Value>> members;  // Object 成员（保持写入顺序）

    bool IsNull() const { return type == Type::Null; }
    bool IsBool() const { return type == Type::Bool; }
    bool IsNumber() const { return type == Type::Number; }
    bool IsString() const { return type == Type::String; }
    bool IsArray() const { return type == Type::Array; }
    bool IsObject() const { return type == Type::Object; }

    const Value* Find(const char* key) const;
    Value* Find(const char* key);
};

bool Parse(const std::string& source, Value& out, std::string* error);
std::string Serialize(const Value& value, int indent = 4);

}  // namespace json

// ---------------------------------------------------------------- 编码转换
std::string WideToUtf8(const std::wstring& text);
std::wstring Utf8ToWide(const std::string& text);

// ---------------------------------------------------------------- 数据结构
struct DifficultyRecord {
    int winCount = 0;
    bool hasMaxSteps = false;
    int maxSteps = 0;
    bool hasMinSteps = false;
    int minSteps = 0;
};

// 对应 Python 版 theme 配置：mode / preset / custom_font / custom_bg / custom_btn
struct ThemeConfig {
    std::wstring mode = L"light";  // light / dark / system / custom
    std::wstring preset = L"none";
    COLORREF customFont = RGB(0x00, 0x00, 0x00);
    COLORREF customBg = RGB(0xf0, 0xf0, 0xf0);
    COLORREF customBtn = RGB(0xe1, 0xe1, 0xe1);
};

// ---------------------------------------------------------------- 存档
class GameStore {
public:
    explicit GameStore(std::wstring path);

    static GameStore CreateDefault();  // exe 同目录下的 GameData.json

    const std::wstring& Path() const { return m_path; }

    std::wstring GetSavedDifficulty() const;
    void SetSavedDifficulty(const std::wstring& difficulty);

    ThemeConfig GetThemeConfig() const;
    void SetThemeConfig(const ThemeConfig& theme);

    DifficultyRecord GetRecord(const std::wstring& difficulty) const;
    void SetWinCount(const std::wstring& difficulty, int count);
    void SetMaxSteps(const std::wstring& difficulty, bool hasValue, int steps);
    void SetMinSteps(const std::wstring& difficulty, bool hasValue, int steps);
    void ResetDifficultyStats(const std::wstring& difficulty);

private:
    static json::Value MakeDefault();
    json::Value Load() const;
    void Save(const json::Value& data) const;
    static json::Value& EnsureRecord(json::Value& data, const std::wstring& difficulty);

    std::wstring m_path;
};
