// GuessNumber.cpp : 猜数字游戏（Win32 版）
//
// 这是 Python + PySide6 版本的 C++/Win32 移植：
//   Main.py            → wWinMain：建主窗口、注册窗口类、跑消息循环
//   QtViews.GameView    → m_page == 0 的游戏页（游戏逻辑在本文件下半部分）
//   QtViews.SettingsView→ m_page == 1 的设置页
//   QtThemeManager.py   → Theme.h / Theme.cpp
//   FileOperator.py     → GameData.h / GameData.cpp（同一份 GameData.json）
//

#include "framework.h"
#include "GuessNumber.h"
#include "GameData.h"
#include "Theme.h"

#include <climits>

namespace {

// ---------------------------------------------------------------- 常量
const wchar_t* const kDifficultyKeys[4] = { L"Easy", L"Medium", L"Hard", L"Impossible" };
const wchar_t* const kDifficultyNames[4] = { L"简单", L"中等", L"困难", L"难以置信" };
const wchar_t* const kModeKeys[4] = { L"light", L"dark", L"system", L"custom" };
const wchar_t* const kModeNames[4] = { L"浅色", L"深色", L"跟随系统", L"自定义" };

const COLORREF kAccentColor = RGB(0x4d, 0xaa, 0xfc);  // 提示文字的蓝色
const COLORREF kErrorColor = RGB(0xff, 0x00, 0x00);   // 错误提示的红色

// ---------------------------------------------------------------- 小工具
std::wstring Trim(const std::wstring& text) {
    const size_t begin = text.find_first_not_of(L" \t\r\n");
    if (begin == std::wstring::npos) return std::wstring();
    const size_t end = text.find_last_not_of(L" \t\r\n");
    return text.substr(begin, end - begin + 1);
}

// 等价于 Python 的 int(text)：允许前后空白与正负号，其余必须是十进制数字
bool ParseInteger(const std::wstring& text, long long& value) {
    const std::wstring trimmed = Trim(text);
    if (trimmed.empty()) return false;

    size_t index = 0;
    bool negative = false;
    if (trimmed[0] == L'+' || trimmed[0] == L'-') {
        negative = (trimmed[0] == L'-');
        index = 1;
    }
    if (index >= trimmed.size()) return false;

    long long result = 0;
    for (; index < trimmed.size(); ++index) {
        const wchar_t c = trimmed[index];
        if (c < L'0' || c > L'9') return false;
        if (result > (LLONG_MAX - 9) / 10) return false;
        result = result * 10 + static_cast<int>(c - L'0');
    }
    value = negative ? -result : result;
    return true;
}

// Python 的 % 对负数返回非负结果，C++ 的 % 不是，所以单独实现
long long FloorMod(long long value, long long divisor) {
    if (divisor == 0) return 0;
    long long result = value % divisor;
    if (result != 0 && ((result < 0) != (divisor < 0))) result += divisor;
    return result;
}

std::mt19937& RandomEngine() {
    static std::mt19937 engine{ std::random_device{}() };
    return engine;
}

long long RandomBetween(long long low, long long high) {
    if (high < low) std::swap(low, high);
    std::uniform_int_distribution<long long> distribution(low, high);
    return distribution(RandomEngine());
}

int DifficultyIndex(const std::wstring& key) {
    for (int i = 0; i < 4; ++i) {
        if (key == kDifficultyKeys[i]) return i;
    }
    return -1;
}

std::wstring DifficultyName(const std::wstring& key) {
    const int index = DifficultyIndex(key);
    return index >= 0 ? kDifficultyNames[index] : L"简单";
}

// ---------------------------------------------------------------- 绘制
void FillRoundRect(HDC dc, const RECT& rc, int radius, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
    RoundRect(dc, rc.left, rc.top, rc.right, rc.bottom, radius * 2, radius * 2);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(brush);
}

// ---------------------------------------------------------------- 按钮附加数据
struct ButtonExtra {
    WNDPROC oldProc = nullptr;
    std::wstring text;
    COLORREF bg = RGB(0xe1, 0xe1, 0xe1);
    COLORREF fg = RGB(0x00, 0x00, 0x00);
    bool hover = false;
};

class App;

App* g_app = nullptr;
WNDPROC g_inputEditProc = nullptr;
COLORREF g_customColors[16] = {};

// 所有按钮都自绘 + 子类化，用于模拟 QSS 的 hover 高亮
LRESULT CALLBACK OwnerButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    auto* extra = reinterpret_cast<ButtonExtra*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (message) {
    case WM_MOUSEMOVE:
        if (extra != nullptr && !extra->hover) {
            extra->hover = true;
            TRACKMOUSEEVENT track = {};
            track.cbSize = sizeof(track);
            track.dwFlags = TME_LEAVE;
            track.hwndTrack = hwnd;
            TrackMouseEvent(&track);
            InvalidateRect(hwnd, nullptr, TRUE);
        }
        break;
    case WM_MOUSELEAVE:
        if (extra != nullptr) {
            extra->hover = false;
            InvalidateRect(hwnd, nullptr, TRUE);
        }
        break;
    case WM_ERASEBKGND:
        return 1;  // 背景由 WM_DRAWITEM 完整覆盖，避免闪烁
    case WM_NCDESTROY: {
        const LRESULT result = (extra != nullptr && extra->oldProc != nullptr)
                                   ? CallWindowProcW(extra->oldProc, hwnd, message, wParam, lParam)
                                   : DefWindowProcW(hwnd, message, wParam, lParam);
        if (extra != nullptr) {
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            delete extra;
        }
        return result;
    }
    default:
        break;
    }

    if (extra != nullptr && extra->oldProc != nullptr) {
        return CallWindowProcW(extra->oldProc, hwnd, message, wParam, lParam);
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

// ---------------------------------------------------------------- 应用
class App {
public:
    explicit App(HINSTANCE instance) : m_hInst(instance), m_store(GameStore::CreateDefault()) {}
    ~App();

    bool Create(int showCommand);
    void SubmitGuess();

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void CreateControls();
    void Layout();
    void ShowPage(int page);
    void PaintBackground(HDC dc);
    void DrawOwnedButton(const DRAWITEMSTRUCT& item);

    HWND CreateOwnerButton(int id, const std::wstring& text, COLORREF background, COLORREF foreground);
    HWND CreateText(int id, const std::wstring& text, DWORD style, HFONT font);
    HWND CreateEdit(int id, DWORD extraStyle);

    // 主题
    void ApplyThemeFromStore();
    void ApplyThemeColors();
    void RefreshSettingsUi();
    void ApplyChanges();

    // 游戏
    void LoadDifficultyData();
    void StartNewRound();
    void RefreshStats();
    void RefreshHintColors();
    void UpdateHint(const std::wstring& text, bool isError);
    void RefreshDifficultyToggle();
    void RefreshDiffVisibility();
    void ChangeDifficulty(int index);
    void HandleClearRecords();
    void SaveProgress();

    // 设置页
    void ChangeMode(int index);
    void ChangePreset(int index);
    void PickColor(int which);

    bool Confirm(const wchar_t* title, const std::wstring& text);

    HINSTANCE m_hInst = nullptr;
    GameStore m_store;

    HWND m_hwnd = nullptr;
    HFONT m_font = nullptr;
    HFONT m_fontBold = nullptr;
    HBRUSH m_bgBrush = nullptr;
    HBRUSH m_fieldBrush = nullptr;

    ThemeConfig m_themeConfig;
    ThemeColors m_colors;
    COLORREF m_hintColor = kAccentColor;
    int m_page = 0;

    // 游戏状态
    std::wstring m_difficulty = L"Easy";
    int m_winCount = 0;
    bool m_hasMaxSteps = false;
    int m_maxSteps = 0;
    bool m_hasMinSteps = false;
    int m_minSteps = 0;
    int m_attempts = 0;
    long long m_targetNumber = 0;
    long long m_rangeMin = 0;
    long long m_rangeMax = 0;
    bool m_diffExpanded = false;

    // 游戏页控件
    HWND m_btnSettings = nullptr;
    HWND m_rangeLabel = nullptr;
    HWND m_input = nullptr;
    HWND m_btnGuess = nullptr;
    HWND m_btnClear = nullptr;
    HWND m_hintLabel = nullptr;
    HWND m_btnDiff = nullptr;
    HWND m_diffButtons[4] = {};
    HWND m_statsLabel = nullptr;

    // 设置页控件
    HWND m_btnBack = nullptr;
    HWND m_modeTitle = nullptr;
    HWND m_presetTitle = nullptr;
    HWND m_colorTitle = nullptr;
    HWND m_modeButtons[4] = {};
    HWND m_presetButtons[5] = {};
    HWND m_colorLabels[3] = {};
    HWND m_colorEdits[3] = {};
    HWND m_colorButtons[3] = {};

    std::vector<HWND> m_gamePage;
    std::vector<HWND> m_settingsPage;
};

// 输入框：回车 = 点“猜！”
LRESULT CALLBACK InputEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_KEYDOWN && wParam == VK_RETURN) {
        if (g_app != nullptr) g_app->SubmitGuess();
        return 0;
    }
    if (message == WM_CHAR && (wParam == VK_RETURN || wParam == L'\r')) {
        return 0;  // 别发出“叮”的一声
    }
    return CallWindowProcW(g_inputEditProc, hwnd, message, wParam, lParam);
}

// “关于”对话框的消息处理（定义在文件末尾）
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// ---------------------------------------------------------------- 创建控件
HWND App::CreateOwnerButton(int id, const std::wstring& text, COLORREF background, COLORREF foreground) {
    HWND button = CreateWindowExW(0, L"BUTTON", text.c_str(),
                                  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW, 0, 0, 0, 0, m_hwnd,
                                  reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), m_hInst, nullptr);
    if (button == nullptr) return nullptr;

    auto* extra = new ButtonExtra();
    extra->text = text;
    extra->bg = background;
    extra->fg = foreground;
    extra->oldProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrW(button, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(OwnerButtonProc)));
    SetWindowLongPtrW(button, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(extra));
    SendMessageW(button, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), TRUE);
    return button;
}

HWND App::CreateText(int id, const std::wstring& text, DWORD style, HFONT font) {
    HWND control = CreateWindowExW(0, L"STATIC", text.c_str(), WS_CHILD | WS_VISIBLE | style, 0, 0, 0, 0, m_hwnd,
                                   reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), m_hInst, nullptr);
    if (control != nullptr) SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    return control;
}

HWND App::CreateEdit(int id, DWORD extraStyle) {
    HWND control = CreateWindowExW(0, L"EDIT", L"",
                                   WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL | extraStyle, 0,
                                   0, 0, 0, m_hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), m_hInst,
                                   nullptr);
    if (control != nullptr) SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), TRUE);
    return control;
}

void App::CreateControls() {
    m_font = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                         CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");
    m_fontBold = CreateFontW(-14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");

    // ---------- 游戏页
    m_btnSettings = CreateOwnerButton(IDC_BTN_SETTINGS, L"设置", m_colors.btnBg, m_colors.btnFg);
    m_rangeLabel = CreateText(IDC_LBL_RANGE, L"", SS_CENTER | SS_NOPREFIX, m_font);
    m_input = CreateEdit(IDC_EDIT_INPUT, ES_CENTER);
    if (m_input != nullptr) {
        g_inputEditProc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtrW(m_input, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(InputEditProc)));
    }
    m_btnGuess = CreateOwnerButton(IDC_BTN_GUESS, L"猜！", m_colors.btnBg, m_colors.btnFg);
    m_btnClear = CreateOwnerButton(IDC_BTN_CLEAR, L"清空记录", m_colors.btnBg, m_colors.btnFg);
    m_hintLabel = CreateText(IDC_LBL_HINT, L"请输入你的猜测", SS_CENTER | SS_NOPREFIX, m_font);
    m_btnDiff = CreateOwnerButton(IDC_BTN_DIFF, L"", m_colors.btnBg, m_colors.btnFg);
    for (int i = 0; i < 4; ++i) {
        m_diffButtons[i] = CreateOwnerButton(IDC_BTN_DIFF_EASY + i, kDifficultyNames[i], m_colors.btnBg,
                                             m_colors.btnFg);
    }
    m_statsLabel = CreateText(IDC_LBL_STATS, L"", SS_CENTER | SS_NOPREFIX, m_font);

    // ---------- 设置页
    m_btnBack = CreateOwnerButton(IDC_BTN_BACK, L"<- 返回", m_colors.btnBg, m_colors.btnFg);
    m_modeTitle = CreateText(IDC_LBL_MODE_TITLE, L"【选择模式】", SS_LEFT, m_fontBold);
    m_presetTitle = CreateText(IDC_LBL_PRESET_TITLE, L"【选择预设】", SS_LEFT, m_fontBold);
    m_colorTitle = CreateText(IDC_LBL_COLOR_TITLE, L"【选择颜色】", SS_LEFT, m_fontBold);
    for (int i = 0; i < 4; ++i) {
        m_modeButtons[i] = CreateOwnerButton(IDC_BTN_MODE_LIGHT + i, kModeNames[i], m_colors.btnBg, m_colors.btnFg);
    }
    for (int i = 0; i < kPresetCount && i < 5; ++i) {
        m_presetButtons[i] = CreateOwnerButton(IDC_BTN_PRESET_YELLOW + i, kPresets[i].name, kPresets[i].button,
                                               ContrastColor(kPresets[i].button));
    }
    const wchar_t* colorLabels[3] = { L"字体：", L"背景：", L"按钮：" };
    for (int i = 0; i < 3; ++i) {
        m_colorLabels[i] = CreateText(IDC_LBL_COLOR_FONT + i, colorLabels[i], SS_LEFT, m_font);
        m_colorEdits[i] = CreateEdit(IDC_EDIT_COLOR_FONT + i, ES_CENTER | ES_READONLY);
        m_colorButtons[i] = CreateOwnerButton(IDC_BTN_COLOR_FONT + i, L"选择", m_colors.btnBg, m_colors.btnFg);
    }

    m_gamePage = { m_btnSettings, m_rangeLabel, m_input, m_btnGuess, m_btnClear, m_hintLabel, m_btnDiff,
                   m_diffButtons[0], m_diffButtons[1], m_diffButtons[2], m_diffButtons[3], m_statsLabel };
    m_settingsPage = { m_btnBack, m_modeTitle, m_presetTitle, m_colorTitle, m_modeButtons[0], m_modeButtons[1],
                       m_modeButtons[2], m_modeButtons[3], m_presetButtons[0], m_presetButtons[1],
                       m_presetButtons[2], m_presetButtons[3], m_presetButtons[4], m_colorLabels[0],
                       m_colorLabels[1], m_colorLabels[2], m_colorEdits[0], m_colorEdits[1], m_colorEdits[2],
                       m_colorButtons[0], m_colorButtons[1], m_colorButtons[2] };

    RefreshDifficultyToggle();
}

// ---------------------------------------------------------------- 布局
void App::Layout() {
    if (m_hwnd == nullptr || m_input == nullptr) return;

    RECT client = {};
    GetClientRect(m_hwnd, &client);
    const int width = client.right - client.left;
    const int height = client.bottom - client.top;
    const int center = width / 2;

    // 游戏页
    MoveWindow(m_btnSettings, 20, 16, 80, 30, TRUE);
    MoveWindow(m_rangeLabel, 0, 92, width, 46, TRUE);
    MoveWindow(m_input, center - 100, 170, 200, 40, TRUE);
    MoveWindow(m_btnGuess, center - 105, 224, 100, 35, TRUE);
    MoveWindow(m_btnClear, center + 5, 224, 100, 35, TRUE);
    MoveWindow(m_hintLabel, 0, 278, width, 24, TRUE);
    MoveWindow(m_btnDiff, center - 100, 320, 200, 35, TRUE);
    for (int i = 0; i < 4; ++i) {
        MoveWindow(m_diffButtons[i], center - 172 + i * 88, 322, 80, 30, TRUE);
    }
    MoveWindow(m_statsLabel, 0, height - 52, width, 22, TRUE);

    // 设置页
    MoveWindow(m_btnBack, 20, 16, 80, 30, TRUE);
    MoveWindow(m_modeTitle, 56, 64, 320, 22, TRUE);
    for (int i = 0; i < 4; ++i) {
        MoveWindow(m_modeButtons[i], 56 + i * 88, 92, 80, 30, TRUE);
    }
    MoveWindow(m_presetTitle, 56, 140, 320, 22, TRUE);
    for (int i = 0; i < 5; ++i) {
        MoveWindow(m_presetButtons[i], 56 + i * 88, 168, 80, 30, TRUE);
    }
    MoveWindow(m_colorTitle, 56, 216, 320, 22, TRUE);
    for (int i = 0; i < 3; ++i) {
        MoveWindow(m_colorLabels[i], 56, 250 + i * 42, 60, 24, TRUE);
        MoveWindow(m_colorEdits[i], 118, 248 + i * 42, 120, 28, TRUE);
        MoveWindow(m_colorButtons[i], 246, 249 + i * 42, 60, 26, TRUE);
    }
}

// ---------------------------------------------------------------- 绘制
void App::PaintBackground(HDC dc) {
    if (m_hwnd == nullptr) return;

    RECT client = {};
    GetClientRect(m_hwnd, &client);
    if (m_bgBrush != nullptr) {
        FillRect(dc, &client, m_bgBrush);
        return;
    }
    HBRUSH fallback = CreateSolidBrush(m_colors.bg);
    FillRect(dc, &client, fallback);
    DeleteObject(fallback);
}

void App::DrawOwnedButton(const DRAWITEMSTRUCT& item) {
    auto* extra = reinterpret_cast<ButtonExtra*>(GetWindowLongPtrW(item.hwndItem, GWLP_USERDATA));
    if (extra == nullptr) return;

    const bool disabled = (item.itemState & ODS_DISABLED) != 0;
    const bool pressed = (item.itemState & ODS_SELECTED) != 0;

    COLORREF background = extra->bg;
    COLORREF foreground = extra->fg;
    if (disabled) {
        background = BlendColor(background, m_colors.bg, 0.72);
        foreground = BlendColor(extra->fg, m_colors.bg, 0.55);
    } else if (pressed) {
        background = BlendColor(background, RGB(0, 0, 0), 0.18);
    } else if (extra->hover) {
        background = BlendColor(background, RGB(255, 255, 255), 0.20);
    }

    RECT rect = item.rcItem;
    if (m_bgBrush != nullptr) FillRect(item.hDC, &rect, m_bgBrush);  // 先抹平圆角外的残留
    FillRoundRect(item.hDC, rect, 8, background);

    SetBkMode(item.hDC, TRANSPARENT);
    SetTextColor(item.hDC, foreground);
    HGDIOBJ oldFont = SelectObject(item.hDC, m_font);
    RECT textRect = rect;
    DrawTextW(item.hDC, extra->text.c_str(), -1, &textRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    SelectObject(item.hDC, oldFont);
}

// ---------------------------------------------------------------- 主题
void App::ApplyThemeFromStore() {
    m_themeConfig = m_store.GetThemeConfig();
    m_colors = ResolveTheme(m_themeConfig);
}

void SetButtonColors(HWND button, COLORREF background, COLORREF foreground) {
    if (button == nullptr) return;
    auto* extra = reinterpret_cast<ButtonExtra*>(GetWindowLongPtrW(button, GWLP_USERDATA));
    if (extra == nullptr) return;
    extra->bg = background;
    extra->fg = foreground;
    InvalidateRect(button, nullptr, TRUE);
}

void App::ApplyThemeColors() {
    if (m_bgBrush != nullptr) DeleteObject(m_bgBrush);
    m_bgBrush = CreateSolidBrush(m_colors.bg);
    if (m_fieldBrush != nullptr) DeleteObject(m_fieldBrush);
    m_fieldBrush = CreateSolidBrush(m_colors.btnBg);

    HWND normalButtons[] = { m_btnSettings, m_btnGuess,   m_btnClear,      m_btnDiff,          m_diffButtons[0],
                             m_diffButtons[1], m_diffButtons[2], m_diffButtons[3], m_btnBack,   m_modeButtons[0],
                             m_modeButtons[1], m_modeButtons[2], m_modeButtons[3], m_colorButtons[0],
                             m_colorButtons[1], m_colorButtons[2] };
    for (HWND button : normalButtons) {
        SetButtonColors(button, m_colors.btnBg, m_colors.btnFg);
    }

    for (int i = 0; i < kPresetCount && i < 5; ++i) {
        SetButtonColors(m_presetButtons[i], kPresets[i].button, ContrastColor(kPresets[i].button));
    }

    for (HWND control : m_gamePage) {
        if (control != nullptr) InvalidateRect(control, nullptr, TRUE);
    }
    for (HWND control : m_settingsPage) {
        if (control != nullptr) InvalidateRect(control, nullptr, TRUE);
    }
    InvalidateRect(m_hwnd, nullptr, TRUE);
}

// ---------------------------------------------------------------- 页面
void App::ShowPage(int page) {
    m_page = page;
    const bool showGame = (page == 0);

    for (HWND control : m_gamePage) {
        if (control != nullptr) ShowWindow(control, showGame ? SW_SHOW : SW_HIDE);
    }
    for (HWND control : m_settingsPage) {
        if (control != nullptr) ShowWindow(control, showGame ? SW_HIDE : SW_SHOW);
    }

    SetWindowTextW(m_hwnd, showGame ? L"猜数字游戏" : L"猜数字游戏 - 设置");

    if (showGame) {
        RefreshDiffVisibility();
        SetFocus(m_input);
    } else {
        m_themeConfig = m_store.GetThemeConfig();
        RefreshSettingsUi();
    }
    InvalidateRect(m_hwnd, nullptr, TRUE);
}

// ---------------------------------------------------------------- 游戏逻辑
void App::LoadDifficultyData() {
    m_difficulty = m_store.GetSavedDifficulty();
    if (DifficultyIndex(m_difficulty) < 0) m_difficulty = L"Easy";

    const DifficultyRecord record = m_store.GetRecord(m_difficulty);
    m_winCount = record.winCount;
    m_hasMaxSteps = record.hasMaxSteps;
    m_maxSteps = record.maxSteps;
    m_hasMinSteps = record.hasMinSteps;
    m_minSteps = record.minSteps;

    RefreshDifficultyToggle();
    RefreshStats();
    StartNewRound();
}

void App::StartNewRound() {
    m_attempts = 0;

    if (m_difficulty == L"Easy") {
        m_rangeMin = 1;
        m_rangeMax = 10;
        m_targetNumber = RandomBetween(1, 10);
    } else if (m_difficulty == L"Medium") {
        m_rangeMin = 1;
        m_rangeMax = 100;
        m_targetNumber = RandomBetween(1, 100);
    } else if (m_difficulty == L"Hard") {
        m_rangeMin = 1;
        m_rangeMax = 10000;
        m_targetNumber = RandomBetween(1, 10000);
    } else {
        const long long front = RandomBetween(1, 10000);
        const long long back = RandomBetween(1, 10000);
        m_rangeMax = (front > back) ? front : back;
        m_rangeMin = (front < back) ? front : back;
        m_targetNumber = RandomBetween(m_rangeMin, m_rangeMax);
    }

    SetWindowTextW(m_rangeLabel, (std::wstring(L"系统生成了一个 ") + std::to_wstring(m_rangeMin) + L"~" +
                                  std::to_wstring(m_rangeMax) + L" 的数字\n(超出会自动取模，不支持负数)")
                                     .c_str());
    UpdateHint(L"请输入你的猜测", false);
    SetWindowTextW(m_input, L"");
    SetFocus(m_input);
}

void App::RefreshStats() {
    const std::wstring maxText = m_hasMaxSteps ? std::to_wstring(m_maxSteps) : std::wstring(L"无");
    const std::wstring minText = m_hasMinSteps ? std::to_wstring(m_minSteps) : std::wstring(L"无");
    const std::wstring text = std::wstring(L"【") + DifficultyName(m_difficulty) + L"】胜场: " +
                              std::to_wstring(m_winCount) + L" | 最大步数: " + maxText + L" | 最小步数: " + minText;
    SetWindowTextW(m_statsLabel, text.c_str());
}

void App::RefreshHintColors() {
    InvalidateRect(m_hintLabel, nullptr, TRUE);
}

void App::UpdateHint(const std::wstring& text, bool isError) {
    m_hintColor = isError ? kErrorColor : kAccentColor;
    SetWindowTextW(m_hintLabel, text.c_str());
    RefreshHintColors();
}

void App::RefreshDifficultyToggle() {
    if (m_btnDiff == nullptr) return;
    auto* extra = reinterpret_cast<ButtonExtra*>(GetWindowLongPtrW(m_btnDiff, GWLP_USERDATA));
    if (extra == nullptr) return;
    extra->text = std::wstring(L"选择难度 (当前: ") + DifficultyName(m_difficulty) + L")";
    SetWindowTextW(m_btnDiff, extra->text.c_str());
    InvalidateRect(m_btnDiff, nullptr, TRUE);
}

void App::RefreshDiffVisibility() {
    for (HWND button : m_diffButtons) {
        if (button != nullptr) ShowWindow(button, m_diffExpanded ? SW_SHOW : SW_HIDE);
    }
    if (m_btnDiff != nullptr) ShowWindow(m_btnDiff, m_diffExpanded ? SW_HIDE : SW_SHOW);
}

void App::ChangeDifficulty(int index) {
    if (index < 0 || index >= 4) return;
    m_difficulty = kDifficultyKeys[index];
    m_diffExpanded = false;
    RefreshDiffVisibility();
    LoadDifficultyData();
    RefreshStats();
}

void App::SubmitGuess() {
    if (m_input == nullptr) return;

    wchar_t buffer[128] = {};
    GetWindowTextW(m_input, buffer, 128);
    const std::wstring raw = Trim(buffer);
    if (raw.empty()) return;

    long long guess = 0;
    if (!ParseInteger(raw, guess)) {
        MessageBoxW(m_hwnd, L"请输入数字！", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    if (guess < 0) {
        UpdateHint(std::wstring(L"输入 ") + std::to_wstring(guess) + L" 非法", true);
        return;
    }

    long long mapped = guess;
    if (m_difficulty != L"Easy") {
        mapped = FloorMod(guess - 1, m_rangeMax) + 1;
    } else {
        int guard = 0;
        while ((mapped > m_rangeMax || mapped < m_rangeMin) && guard++ < 64) {
            mapped = FloorMod(mapped * 5, m_rangeMax);
            if (mapped == 0) mapped = 1;
        }
    }

    ++m_attempts;

    if (mapped == m_targetNumber) {
        ++m_winCount;
        const bool isNewMax = !m_hasMaxSteps || m_attempts > m_maxSteps;
        const bool isNewMin = !m_hasMinSteps || m_attempts < m_minSteps;
        if (isNewMax) {
            m_hasMaxSteps = true;
            m_maxSteps = m_attempts;
        }
        if (isNewMin) {
            m_hasMinSteps = true;
            m_minSteps = m_attempts;
        }

        m_store.SetWinCount(m_difficulty, m_winCount);
        if (isNewMax) m_store.SetMaxSteps(m_difficulty, true, m_maxSteps);
        if (isNewMin) m_store.SetMinSteps(m_difficulty, true, m_minSteps);

        const std::wstring message = std::wstring(L"猜对了！你用了 ") + std::to_wstring(m_attempts) + L" 步。";
        MessageBoxW(m_hwnd, message.c_str(), L"恭喜", MB_OK | MB_ICONINFORMATION);
        StartNewRound();
    } else {
        if (mapped > m_targetNumber) {
            UpdateHint(L"太大了！", true);
        } else {
            UpdateHint(L"太小了！", true);
        }
        SetWindowTextW(m_input, L"");
        SetFocus(m_input);
    }

    RefreshStats();
}

void App::HandleClearRecords() {
    if (!Confirm(L"确认", std::wstring(L"确定要清空【") + DifficultyName(m_difficulty) + L"】难度的所有记录吗？")) {
        return;
    }
    m_winCount = 0;
    m_hasMaxSteps = false;
    m_maxSteps = 0;
    m_hasMinSteps = false;
    m_minSteps = 0;
    m_store.ResetDifficultyStats(m_difficulty);
    RefreshStats();
    UpdateHint(L"当前难度记录已清空", false);
}

void App::SaveProgress() {
    if (m_page != 0) return;  // 和 Python 版一致：只有停留在游戏页时才保存进度
    m_store.SetSavedDifficulty(m_difficulty);
    m_store.SetWinCount(m_difficulty, m_winCount);
    m_store.SetMaxSteps(m_difficulty, m_hasMaxSteps, m_maxSteps);
    m_store.SetMinSteps(m_difficulty, m_hasMinSteps, m_minSteps);
}

// ---------------------------------------------------------------- 设置页
void App::RefreshSettingsUi() {
    const bool custom = (m_themeConfig.mode == L"custom");
    for (HWND button : m_presetButtons) {
        if (button == nullptr) continue;
        EnableWindow(button, custom ? TRUE : FALSE);
        InvalidateRect(button, nullptr, TRUE);
    }
    SetWindowTextW(m_colorEdits[0], ColorToHex(m_themeConfig.customFont).c_str());
    SetWindowTextW(m_colorEdits[1], ColorToHex(m_themeConfig.customBg).c_str());
    SetWindowTextW(m_colorEdits[2], ColorToHex(m_themeConfig.customBtn).c_str());
}

void App::ApplyChanges() {
    m_store.SetThemeConfig(m_themeConfig);
    ApplyThemeFromStore();
    ApplyThemeColors();
    RefreshSettingsUi();
    InvalidateRect(m_hwnd, nullptr, TRUE);
}

void App::ChangeMode(int index) {
    if (index < 0 || index >= 4) return;

    m_themeConfig = m_store.GetThemeConfig();
    m_themeConfig.mode = kModeKeys[index];

    if (m_themeConfig.mode != L"custom") {
        m_themeConfig.preset = L"none";
    } else if (m_themeConfig.preset == L"none") {
        const PresetDef* preset = FindPreset(L"yellow");
        if (preset != nullptr) {
            m_themeConfig.preset = preset->key;
            m_themeConfig.customBg = preset->bg;
            m_themeConfig.customBtn = preset->button;
            m_themeConfig.customFont = ContrastColor(preset->bg);
        }
    }
    ApplyChanges();
}

void App::ChangePreset(int index) {
    if (index < 0 || index >= kPresetCount) return;
    if (m_themeConfig.mode != L"custom") return;

    m_themeConfig.preset = kPresets[index].key;
    m_themeConfig.customBg = kPresets[index].bg;
    m_themeConfig.customBtn = kPresets[index].button;
    m_themeConfig.customFont = ContrastColor(kPresets[index].bg);
    ApplyChanges();
}

void App::PickColor(int which) {
    if (m_themeConfig.mode != L"custom") {
        MessageBoxW(m_hwnd, L"请先切换至【自定义】模式！", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }

    COLORREF initial = m_themeConfig.customFont;
    if (which == 1) initial = m_themeConfig.customBg;
    if (which == 2) initial = m_themeConfig.customBtn;

    CHOOSECOLORW chooser = {};
    chooser.lStructSize = sizeof(chooser);
    chooser.hwndOwner = m_hwnd;
    chooser.rgbResult = initial;
    chooser.lpCustColors = g_customColors;
    chooser.Flags = CC_FULLOPEN | CC_RGBINIT;
    if (!ChooseColorW(&chooser)) return;

    if (which == 0) {
        m_themeConfig.customFont = chooser.rgbResult;
    } else if (which == 1) {
        m_themeConfig.customBg = chooser.rgbResult;
        m_themeConfig.customFont = ContrastColor(m_themeConfig.customBg);
    } else {
        m_themeConfig.customBtn = chooser.rgbResult;
    }
    m_themeConfig.preset = L"none";
    ApplyChanges();
}

bool App::Confirm(const wchar_t* title, const std::wstring& text) {
    return MessageBoxW(m_hwnd, text.c_str(), title, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES;
}

// ---------------------------------------------------------------- 消息处理
LRESULT App::HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        m_hwnd = hwnd;
        CreateControls();
        LoadDifficultyData();
        ApplyThemeFromStore();
        ApplyThemeColors();
        RefreshSettingsUi();
        ShowPage(0);
        Layout();
        return 0;

    case WM_SIZE:
        Layout();
        return 0;

    case WM_ERASEBKGND:
        PaintBackground(reinterpret_cast<HDC>(wParam));
        return 1;

    case WM_GETMINMAXINFO: {
        auto* info = reinterpret_cast<MINMAXINFO*>(lParam);
        info->ptMinTrackSize.x = 760;
        info->ptMinTrackSize.y = 480;
        return 0;
    }

    case WM_CTLCOLORSTATIC: {
        HDC dc = reinterpret_cast<HDC>(wParam);
        HWND control = reinterpret_cast<HWND>(lParam);
        bool isReadOnlyEdit = false;
        for (HWND edit : m_colorEdits) {
            if (edit == control) isReadOnlyEdit = true;
        }
        if (isReadOnlyEdit) {
            SetBkMode(dc, OPAQUE);
            SetBkColor(dc, m_colors.btnBg);
            SetTextColor(dc, m_colors.text);
            HBRUSH brush = (m_fieldBrush != nullptr) ? m_fieldBrush : GetSysColorBrush(COLOR_WINDOW);
            return static_cast<LRESULT>(reinterpret_cast<INT_PTR>(brush));
        }
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, (control == m_hintLabel) ? m_hintColor : m_colors.text);
        HBRUSH brush = (m_bgBrush != nullptr) ? m_bgBrush : GetSysColorBrush(COLOR_WINDOW);
        return static_cast<LRESULT>(reinterpret_cast<INT_PTR>(brush));
    }

    case WM_CTLCOLOREDIT: {
        HDC dc = reinterpret_cast<HDC>(wParam);
        SetBkMode(dc, OPAQUE);
        SetBkColor(dc, m_colors.btnBg);
        SetTextColor(dc, m_colors.text);
        HBRUSH brush = (m_fieldBrush != nullptr) ? m_fieldBrush : GetSysColorBrush(COLOR_WINDOW);
        return static_cast<LRESULT>(reinterpret_cast<INT_PTR>(brush));
    }

    case WM_DRAWITEM: {
        const auto* item = reinterpret_cast<const DRAWITEMSTRUCT*>(lParam);
        if (item != nullptr && item->CtlType == ODT_BUTTON) {
            DrawOwnedButton(*item);
            return TRUE;
        }
        return FALSE;
    }

    case WM_COMMAND: {
        const int id = LOWORD(wParam);
        if (id == IDM_EXIT) {
            SendMessageW(hwnd, WM_CLOSE, 0, 0);
            return 0;
        }
        if (id == IDM_ABOUT) {
            DialogBoxW(m_hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hwnd, AboutDlgProc);
            return 0;
        }
        if (id == IDC_BTN_SETTINGS) { ShowPage(1); return 0; }
        if (id == IDC_BTN_BACK) { ShowPage(0); return 0; }
        if (id == IDC_BTN_GUESS) { SubmitGuess(); return 0; }
        if (id == IDC_BTN_CLEAR) { HandleClearRecords(); return 0; }
        if (id == IDC_BTN_DIFF) {
            m_diffExpanded = true;
            RefreshDiffVisibility();
            return 0;
        }
        if (id >= IDC_BTN_DIFF_EASY && id <= IDC_BTN_DIFF_IMPOSSIBLE) {
            ChangeDifficulty(id - IDC_BTN_DIFF_EASY);
            return 0;
        }
        if (id >= IDC_BTN_MODE_LIGHT && id <= IDC_BTN_MODE_CUSTOM) {
            ChangeMode(id - IDC_BTN_MODE_LIGHT);
            return 0;
        }
        if (id >= IDC_BTN_PRESET_YELLOW && id <= IDC_BTN_PRESET_CYAN) {
            ChangePreset(id - IDC_BTN_PRESET_YELLOW);
            return 0;
        }
        if (id == IDC_BTN_COLOR_FONT) { PickColor(0); return 0; }
        if (id == IDC_BTN_COLOR_BG) { PickColor(1); return 0; }
        if (id == IDC_BTN_COLOR_BTN) { PickColor(2); return 0; }
        return 0;
    }

    case WM_CLOSE:
        if (Confirm(L"退出", L"是否确认退出游戏？\n（系统会自动保存你的游戏进度）")) {
            SaveProgress();
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK App::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
    }
    auto* app = reinterpret_cast<App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (app != nullptr) return app->HandleMessage(hwnd, message, wParam, lParam);
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

bool App::Create(int showCommand) {
    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = App::WindowProc;
    windowClass.hInstance = m_hInst;
    windowClass.hIcon = LoadIconW(m_hInst, MAKEINTRESOURCEW(IDI_GUESSNUMBER));
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = nullptr;
    windowClass.lpszMenuName = MAKEINTRESOURCEW(IDC_GUESSNUMBER);
    windowClass.lpszClassName = L"GuessNumberMainWindow";
    windowClass.hIconSm = LoadIconW(m_hInst, MAKEINTRESOURCEW(IDI_SMALL));
    if (RegisterClassExW(&windowClass) == 0) return false;

    RECT rect = { 0, 0, 800, 500 };
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, TRUE, 0);

    HWND window = CreateWindowExW(0, windowClass.lpszClassName, L"猜数字游戏",
                                  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
                                  rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, m_hInst, this);
    if (window == nullptr) return false;

    ShowWindow(window, showCommand);
    UpdateWindow(window);
    return true;
}

App::~App() {
    if (m_font != nullptr) DeleteObject(m_font);
    if (m_fontBold != nullptr) DeleteObject(m_fontBold);
    if (m_bgBrush != nullptr) DeleteObject(m_bgBrush);
    if (m_fieldBrush != nullptr) DeleteObject(m_fieldBrush);
}

// ---------------------------------------------------------------- “关于”对话框
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return static_cast<INT_PTR>(TRUE);

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return static_cast<INT_PTR>(TRUE);
        }
        break;
    default:
        break;
    }
    return static_cast<INT_PTR>(FALSE);
}

}  // namespace

// ---------------------------------------------------------------- 入口
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    App app(hInstance);
    g_app = &app;

    if (!app.Create(nCmdShow)) return FALSE;

    HACCEL accelerators = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDC_GUESSNUMBER));

    MSG message = {};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (!TranslateAcceleratorW(message.hwnd, accelerators, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    g_app = nullptr;
    return static_cast<int>(message.wParam);
}
