// GameData.cpp
#include "framework.h"
#include "GameData.h"
#include "Theme.h"

#include <cmath>
#include <cstdio>

// ==================================================================== JSON
namespace json {
namespace {

// ------------------------------------------------------------ UTF-8 工具
void AppendUtf8(std::string& out, unsigned int cp) {
    if (cp < 0x80) {
        out += static_cast<char>(cp);
    } else if (cp < 0x800) {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (cp >> 18));
        out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
}

// ------------------------------------------------------------ 递归下降解析
class Parser {
public:
    explicit Parser(const std::string& source) : m_s(source) {}

    bool Run(Value& out) {
        SkipWhitespace();
        if (!ParseValue(out)) return false;
        SkipWhitespace();
        if (m_pos != m_s.size()) return Fail("trailing characters");
        return true;
    }

    const std::string& Error() const { return m_error; }

private:
    bool Fail(const char* message) {
        if (m_error.empty()) m_error = message;
        return false;
    }

    void SkipWhitespace() {
        while (m_pos < m_s.size()) {
            const char c = m_s[m_pos];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                ++m_pos;
            } else {
                break;
            }
        }
    }

    bool ParseValue(Value& out) {
        SkipWhitespace();
        if (m_pos >= m_s.size()) return Fail("unexpected end of input");

        const char c = m_s[m_pos];
        if (c == '{') return ParseObject(out);
        if (c == '[') return ParseArray(out);
        if (c == '"') {
            out.type = Type::String;
            return ParseString(out.text);
        }
        if (c == 't') {
            if (!ParseLiteral("true")) return false;
            out.type = Type::Bool;
            out.boolean = true;
            return true;
        }
        if (c == 'f') {
            if (!ParseLiteral("false")) return false;
            out.type = Type::Bool;
            out.boolean = false;
            return true;
        }
        if (c == 'n') {
            if (!ParseLiteral("null")) return false;
            out.type = Type::Null;
            return true;
        }
        return ParseNumber(out);
    }

    bool ParseLiteral(const char* literal) {
        const size_t len = std::strlen(literal);
        if (m_s.compare(m_pos, len, literal) != 0) return Fail("invalid literal");
        m_pos += len;
        return true;
    }

    bool ParseNumber(Value& out) {
        const size_t start = m_pos;
        if (m_pos < m_s.size() && (m_s[m_pos] == '-' || m_s[m_pos] == '+')) ++m_pos;
        bool any = false;
        while (m_pos < m_s.size() && m_s[m_pos] >= '0' && m_s[m_pos] <= '9') {
            ++m_pos;
            any = true;
        }
        if (m_pos < m_s.size() && m_s[m_pos] == '.') {
            ++m_pos;
            while (m_pos < m_s.size() && m_s[m_pos] >= '0' && m_s[m_pos] <= '9') {
                ++m_pos;
                any = true;
            }
        }
        if (!any) return Fail("invalid number");
        if (m_pos < m_s.size() && (m_s[m_pos] == 'e' || m_s[m_pos] == 'E')) {
            ++m_pos;
            if (m_pos < m_s.size() && (m_s[m_pos] == '-' || m_s[m_pos] == '+')) ++m_pos;
            while (m_pos < m_s.size() && m_s[m_pos] >= '0' && m_s[m_pos] <= '9') ++m_pos;
        }
        out.type = Type::Number;
        out.number = std::strtod(m_s.substr(start, m_pos - start).c_str(), nullptr);
        return true;
    }

    bool ReadHex4(unsigned int& value) {
        if (m_pos + 4 > m_s.size()) return Fail("truncated \\u escape");
        value = 0;
        for (int i = 0; i < 4; ++i) {
            const char c = m_s[m_pos + i];
            unsigned int digit = 0;
            if (c >= '0' && c <= '9') {
                digit = static_cast<unsigned int>(c - '0');
            } else if (c >= 'a' && c <= 'f') {
                digit = static_cast<unsigned int>(c - 'a' + 10);
            } else if (c >= 'A' && c <= 'F') {
                digit = static_cast<unsigned int>(c - 'A' + 10);
            } else {
                return Fail("invalid \\u escape");
            }
            value = (value << 4) | digit;
        }
        m_pos += 4;
        return true;
    }

    bool ParseString(std::string& out) {
        out.clear();
        ++m_pos;  // 跳过起始引号
        while (m_pos < m_s.size()) {
            const unsigned char c = static_cast<unsigned char>(m_s[m_pos]);
            if (c == '"') {
                ++m_pos;
                return true;
            }
            if (c == '\\') {
                ++m_pos;
                if (m_pos >= m_s.size()) return Fail("bad escape");
                const char escape = m_s[m_pos++];
                switch (escape) {
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'n': out += '\n'; break;
                case 'r': out += '\r'; break;
                case 't': out += '\t'; break;
                case 'u': {
                    unsigned int cp = 0;
                    if (!ReadHex4(cp)) return false;
                    if (cp >= 0xD800 && cp <= 0xDBFF && m_pos + 5 < m_s.size() && m_s[m_pos] == '\\' &&
                        m_s[m_pos + 1] == 'u') {
                        m_pos += 2;
                        unsigned int low = 0;
                        if (!ReadHex4(low)) return false;
                        if (low >= 0xDC00 && low <= 0xDFFF) {
                            cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                        }
                    }
                    AppendUtf8(out, cp);
                    break;
                }
                default: return Fail("unsupported escape");
                }
                continue;
            }
            if (c < 0x20) return Fail("control character in string");
            out += static_cast<char>(c);
            ++m_pos;
        }
        return Fail("unterminated string");
    }

    bool ParseArray(Value& out) {
        out.type = Type::Array;
        out.items.clear();
        ++m_pos;  // '['
        SkipWhitespace();
        if (m_pos < m_s.size() && m_s[m_pos] == ']') {
            ++m_pos;
            return true;
        }
        for (;;) {
            Value item;
            if (!ParseValue(item)) return false;
            out.items.push_back(std::move(item));
            SkipWhitespace();
            if (m_pos >= m_s.size()) return Fail("unterminated array");
            if (m_s[m_pos] == ',') {
                ++m_pos;
                continue;
            }
            if (m_s[m_pos] == ']') {
                ++m_pos;
                return true;
            }
            return Fail("',' or ']' expected");
        }
    }

    bool ParseObject(Value& out) {
        out.type = Type::Object;
        out.members.clear();
        ++m_pos;  // '{'
        SkipWhitespace();
        if (m_pos < m_s.size() && m_s[m_pos] == '}') {
            ++m_pos;
            return true;
        }
        for (;;) {
            SkipWhitespace();
            if (m_pos >= m_s.size() || m_s[m_pos] != '"') return Fail("object key expected");
            std::string key;
            if (!ParseString(key)) return false;
            SkipWhitespace();
            if (m_pos >= m_s.size() || m_s[m_pos] != ':') return Fail("':' expected");
            ++m_pos;
            Value value;
            if (!ParseValue(value)) return false;
            out.members.emplace_back(std::move(key), std::move(value));
            SkipWhitespace();
            if (m_pos >= m_s.size()) return Fail("unterminated object");
            if (m_s[m_pos] == ',') {
                ++m_pos;
                continue;
            }
            if (m_s[m_pos] == '}') {
                ++m_pos;
                return true;
            }
            return Fail("',' or '}' expected");
        }
    }

    const std::string& m_s;
    size_t m_pos = 0;
    std::string m_error;
};

// ------------------------------------------------------------ 序列化
std::string FormatNumber(double value) {
    char buffer[64] = {};
    if (std::isfinite(value) && value == std::floor(value) && std::fabs(value) < 1e15) {
        std::snprintf(buffer, sizeof(buffer), "%lld", static_cast<long long>(value));
    } else {
        std::snprintf(buffer, sizeof(buffer), "%.17g", value);
    }
    return buffer;
}

void EscapeString(const std::string& text, std::string& out) {
    out += '"';
    for (const char ch : text) {
        const unsigned char c = static_cast<unsigned char>(ch);
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 0x20) {
                char buffer[8] = {};
                std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                out += buffer;
            } else {
                out += static_cast<char>(c);
            }
            break;
        }
    }
    out += '"';
}

void SerializeValue(const Value& value, int indent, int depth, std::string& out) {
    const std::string pad(static_cast<size_t>(indent) * static_cast<size_t>(depth), ' ');
    const std::string padInner(static_cast<size_t>(indent) * static_cast<size_t>(depth + 1), ' ');

    switch (value.type) {
    case Type::Null:
        out += "null";
        break;
    case Type::Bool:
        out += value.boolean ? "true" : "false";
        break;
    case Type::Number:
        out += FormatNumber(value.number);
        break;
    case Type::String:
        EscapeString(value.text, out);
        break;
    case Type::Array:
        if (value.items.empty()) {
            out += "[]";
            break;
        }
        out += "[\n";
        for (size_t i = 0; i < value.items.size(); ++i) {
            out += padInner;
            SerializeValue(value.items[i], indent, depth + 1, out);
            if (i + 1 < value.items.size()) out += ',';
            out += '\n';
        }
        out += pad;
        out += ']';
        break;
    case Type::Object:
        if (value.members.empty()) {
            out += "{}";
            break;
        }
        out += "{\n";
        for (size_t i = 0; i < value.members.size(); ++i) {
            out += padInner;
            EscapeString(value.members[i].first, out);
            out += ": ";
            SerializeValue(value.members[i].second, indent, depth + 1, out);
            if (i + 1 < value.members.size()) out += ',';
            out += '\n';
        }
        out += pad;
        out += '}';
        break;
    }
}

}  // namespace

const Value* Value::Find(const char* key) const {
    for (const auto& member : members) {
        if (member.first == key) return &member.second;
    }
    return nullptr;
}

Value* Value::Find(const char* key) {
    for (auto& member : members) {
        if (member.first == key) return &member.second;
    }
    return nullptr;
}

bool Parse(const std::string& source, Value& out, std::string* error) {
    std::string text = source;
    // 跳过 UTF-8 BOM
    if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF &&
        static_cast<unsigned char>(text[1]) == 0xBB && static_cast<unsigned char>(text[2]) == 0xBF) {
        text.erase(0, 3);
    }
    Parser parser(text);
    if (!parser.Run(out)) {
        if (error) *error = parser.Error();
        return false;
    }
    return true;
}

std::string Serialize(const Value& value, int indent) {
    std::string out;
    SerializeValue(value, indent, 0, out);
    return out;
}

}  // namespace json

// ==================================================================== 编码
std::string WideToUtf8(const std::wstring& text) {
    if (text.empty()) return std::string();
    const int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0,
                                         nullptr, nullptr);
    if (size <= 0) return std::string();
    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), &result[0], size, nullptr, nullptr);
    return result;
}

std::wstring Utf8ToWide(const std::string& text) {
    if (text.empty()) return std::wstring();
    const int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0);
    if (size <= 0) return std::wstring();
    std::wstring result(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), &result[0], size);
    return result;
}

// ==================================================================== 存档
namespace {

json::Value MakeObject() {
    json::Value value;
    value.type = json::Type::Object;
    return value;
}

json::Value MakeString(const std::string& text) {
    json::Value value;
    value.type = json::Type::String;
    value.text = text;
    return value;
}

json::Value MakeNumber(double number) {
    json::Value value;
    value.type = json::Type::Number;
    value.number = number;
    return value;
}

json::Value MakeNull() {
    json::Value value;
    value.type = json::Type::Null;
    return value;
}

json::Value MakeTheme(const wchar_t* mode, const wchar_t* preset, COLORREF font, COLORREF bg, COLORREF button) {
    json::Value theme = MakeObject();
    theme.members.emplace_back("mode", MakeString(WideToUtf8(mode)));
    theme.members.emplace_back("preset", MakeString(WideToUtf8(preset)));
    theme.members.emplace_back("custom_font", MakeString(WideToUtf8(ColorToHex(font))));
    theme.members.emplace_back("custom_bg", MakeString(WideToUtf8(ColorToHex(bg))));
    theme.members.emplace_back("custom_btn", MakeString(WideToUtf8(ColorToHex(button))));
    return theme;
}

void SetMember(json::Value& object, const char* key, json::Value value) {
    for (auto& member : object.members) {
        if (member.first == key) {
            member.second = std::move(value);
            return;
        }
    }
    object.members.emplace_back(key, std::move(value));
}

std::wstring GetStringMember(const json::Value& object, const char* key, const wchar_t* fallback) {
    const json::Value* found = object.Find(key);
    if (!found || !found->IsString()) return fallback;
    return Utf8ToWide(found->text);
}

int GetIntMember(const json::Value& object, const char* key, int fallback) {
    const json::Value* found = object.Find(key);
    if (!found || !found->IsNumber()) return fallback;
    return static_cast<int>(found->number);
}

COLORREF GetColorMember(const json::Value& object, const char* key, COLORREF fallback) {
    const json::Value* found = object.Find(key);
    if (!found || !found->IsString()) return fallback;
    COLORREF color = fallback;
    if (!HexToColor(Utf8ToWide(found->text), color)) return fallback;
    return color;
}

void EnsureTheme(json::Value& data) {
    json::Value* theme = data.Find("theme");
    if (!theme || !theme->IsObject()) {
        SetMember(data, "theme", MakeTheme(L"light", L"none", RGB(0x00, 0x00, 0x00), RGB(0xf0, 0xf0, 0xf0),
                                           RGB(0xe1, 0xe1, 0xe1)));
        return;
    }
    if (!theme->Find("mode")) theme->members.emplace_back("mode", MakeString("light"));
    if (!theme->Find("preset")) theme->members.emplace_back("preset", MakeString("none"));
    if (!theme->Find("custom_font")) theme->members.emplace_back("custom_font", MakeString("#000000"));
    if (!theme->Find("custom_bg")) theme->members.emplace_back("custom_bg", MakeString("#f0f0f0"));
    if (!theme->Find("custom_btn")) theme->members.emplace_back("custom_btn", MakeString("#e1e1e1"));
}

}  // namespace

json::Value GameStore::MakeDefault() {
    json::Value data = MakeObject();
    data.members.emplace_back("current_difficulty", MakeString("Easy"));
    data.members.emplace_back("records", MakeObject());
    data.members.emplace_back("theme", MakeTheme(L"light", L"none", RGB(0x00, 0x00, 0x00), RGB(0xf0, 0xf0, 0xf0),
                                                 RGB(0xe1, 0xe1, 0xe1)));
    return data;
}

GameStore::GameStore(std::wstring path) : m_path(std::move(path)) {}

GameStore GameStore::CreateDefault() {
    wchar_t buffer[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    std::wstring directory;
    if (length > 0) {
        directory.assign(buffer, length);
        const size_t slash = directory.find_last_of(L"\\/");
        if (slash != std::wstring::npos) {
            directory.erase(slash + 1);
        } else {
            directory.clear();
        }
    }
    return GameStore(directory + L"GameData.json");
}

json::Value GameStore::Load() const {
    FILE* file = nullptr;
    if (_wfopen_s(&file, m_path.c_str(), L"rb") != 0 || file == nullptr) {
        return MakeDefault();
    }

    std::string bytes;
    char chunk[4096];
    size_t read = 0;
    while ((read = fread(chunk, 1, sizeof(chunk), file)) > 0) {
        bytes.append(chunk, read);
    }
    fclose(file);

    json::Value data;
    std::string error;
    if (!json::Parse(bytes, data, &error) || !data.IsObject()) {
        return MakeDefault();
    }

    // 数据迁移：老版本的扁平结构 → 新版嵌套结构
    if (!data.Find("records")) {
        const std::wstring oldDifficulty = GetStringMember(data, "difficulty", L"Easy");
        const int oldWins = GetIntMember(data, "victory_times", 0);
        const json::Value* oldMax = data.Find("max_time");
        const json::Value* oldMin = data.Find("min_time");

        json::Value record = MakeObject();
        record.members.emplace_back("win_count", MakeNumber(oldWins));
        record.members.emplace_back("max_steps", (oldMax && oldMax->IsNumber()) ? MakeNumber(oldMax->number) : MakeNull());
        record.members.emplace_back("min_steps", (oldMin && oldMin->IsNumber()) ? MakeNumber(oldMin->number) : MakeNull());

        json::Value records = MakeObject();
        records.members.emplace_back(WideToUtf8(oldDifficulty), std::move(record));

        data = MakeObject();
        data.members.emplace_back("current_difficulty", MakeString(WideToUtf8(oldDifficulty)));
        data.members.emplace_back("records", std::move(records));
        data.members.emplace_back("theme", MakeTheme(L"light", L"none", RGB(0x00, 0x00, 0x00), RGB(0xf0, 0xf0, 0xf0),
                                                     RGB(0xe1, 0xe1, 0xe1)));
    }

    if (!data.Find("current_difficulty")) {
        data.members.emplace_back("current_difficulty", MakeString("Easy"));
    }

    EnsureTheme(data);
    return data;
}

void GameStore::Save(const json::Value& data) const {
    FILE* file = nullptr;
    if (_wfopen_s(&file, m_path.c_str(), L"wb") != 0 || file == nullptr) {
        return;
    }
    const std::string text = json::Serialize(data, 4);
    fwrite(text.data(), 1, text.size(), file);
    fclose(file);
}

json::Value& GameStore::EnsureRecord(json::Value& data, const std::wstring& difficulty) {
    json::Value* records = data.Find("records");
    if (records == nullptr || !records->IsObject()) {
        SetMember(data, "records", MakeObject());
        records = data.Find("records");
    }

    const std::string key = WideToUtf8(difficulty);
    json::Value* record = records->Find(key.c_str());
    if (record == nullptr || !record->IsObject()) {
        json::Value fresh = MakeObject();
        fresh.members.emplace_back("win_count", MakeNumber(0));
        fresh.members.emplace_back("max_steps", MakeNull());
        fresh.members.emplace_back("min_steps", MakeNull());
        bool replaced = false;
        for (auto& member : records->members) {
            if (member.first == key) {
                member.second = fresh;
                replaced = true;
                break;
            }
        }
        if (!replaced) records->members.emplace_back(key, fresh);
        record = records->Find(key.c_str());
    }
    return *record;
}

std::wstring GameStore::GetSavedDifficulty() const {
    const json::Value data = Load();
    return GetStringMember(data, "current_difficulty", L"Easy");
}

void GameStore::SetSavedDifficulty(const std::wstring& difficulty) {
    json::Value data = Load();
    SetMember(data, "current_difficulty", MakeString(WideToUtf8(difficulty)));
    Save(data);
}

ThemeConfig GameStore::GetThemeConfig() const {
    const json::Value data = Load();
    ThemeConfig config;
    const json::Value* theme = data.Find("theme");
    if (!theme || !theme->IsObject()) return config;

    config.mode = GetStringMember(*theme, "mode", L"light");
    config.preset = GetStringMember(*theme, "preset", L"none");
    config.customFont = GetColorMember(*theme, "custom_font", config.customFont);
    config.customBg = GetColorMember(*theme, "custom_bg", config.customBg);
    config.customBtn = GetColorMember(*theme, "custom_btn", config.customBtn);
    return config;
}

void GameStore::SetThemeConfig(const ThemeConfig& theme) {
    json::Value data = Load();
    json::Value node = MakeObject();
    node.members.emplace_back("mode", MakeString(WideToUtf8(theme.mode)));
    node.members.emplace_back("preset", MakeString(WideToUtf8(theme.preset)));
    node.members.emplace_back("custom_font", MakeString(WideToUtf8(ColorToHex(theme.customFont))));
    node.members.emplace_back("custom_bg", MakeString(WideToUtf8(ColorToHex(theme.customBg))));
    node.members.emplace_back("custom_btn", MakeString(WideToUtf8(ColorToHex(theme.customBtn))));
    SetMember(data, "theme", std::move(node));
    Save(data);
}

DifficultyRecord GameStore::GetRecord(const std::wstring& difficulty) const {
    DifficultyRecord result;
    const json::Value data = Load();
    const json::Value* records = data.Find("records");
    if (!records || !records->IsObject()) return result;

    const std::string key = WideToUtf8(difficulty);
    const json::Value* record = records->Find(key.c_str());
    if (!record || !record->IsObject()) return result;

    result.winCount = GetIntMember(*record, "win_count", 0);
    if (const json::Value* maxSteps = record->Find("max_steps")) {
        if (maxSteps->IsNumber()) {
            result.hasMaxSteps = true;
            result.maxSteps = static_cast<int>(maxSteps->number);
        }
    }
    if (const json::Value* minSteps = record->Find("min_steps")) {
        if (minSteps->IsNumber()) {
            result.hasMinSteps = true;
            result.minSteps = static_cast<int>(minSteps->number);
        }
    }
    return result;
}

void GameStore::SetWinCount(const std::wstring& difficulty, int count) {
    json::Value data = Load();
    json::Value& record = EnsureRecord(data, difficulty);
    SetMember(record, "win_count", MakeNumber(count));
    Save(data);
}

void GameStore::SetMaxSteps(const std::wstring& difficulty, bool hasValue, int steps) {
    json::Value data = Load();
    json::Value& record = EnsureRecord(data, difficulty);
    SetMember(record, "max_steps", hasValue ? MakeNumber(steps) : MakeNull());
    Save(data);
}

void GameStore::SetMinSteps(const std::wstring& difficulty, bool hasValue, int steps) {
    json::Value data = Load();
    json::Value& record = EnsureRecord(data, difficulty);
    SetMember(record, "min_steps", hasValue ? MakeNumber(steps) : MakeNull());
    Save(data);
}

void GameStore::ResetDifficultyStats(const std::wstring& difficulty) {
    json::Value data = Load();
    json::Value* records = data.Find("records");
    if (!records || !records->IsObject()) return;

    const std::string key = WideToUtf8(difficulty);
    for (auto it = records->members.begin(); it != records->members.end(); ++it) {
        if (it->first == key) {
            records->members.erase(it);
            break;
        }
    }
    Save(data);
}
