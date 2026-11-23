// Theme.h
//
// 对应 Python 版 QtThemeManager.py：
//   * 5 个预设配色
//   * 对比色自动计算（黑/白文字）
//   * 跟随系统深色（读注册表 AppsUseLightTheme）
//   * 把 ThemeConfig 解析成 Win32 真正用得上的几个颜色
//
#pragma once

#include "framework.h"
#include "GameData.h"

struct PresetDef {
    const wchar_t* key;   // yellow / violet / sakura / deepseek / cyan
    const wchar_t* name;  // 明黄 / 紫罗兰 / 樱花粉 / 深空蓝 / 青色
    COLORREF bg;
    COLORREF button;
};

extern const PresetDef kPresets[];
extern const int kPresetCount;

struct ThemeColors {
    COLORREF bg = RGB(0xf0, 0xf0, 0xf0);
    COLORREF text = RGB(0x00, 0x00, 0x00);
    COLORREF btnBg = RGB(0xe1, 0xe1, 0xe1);
    COLORREF btnFg = RGB(0x00, 0x00, 0x00);
};

std::wstring ColorToHex(COLORREF color);                       // "#rrggbb"
bool HexToColor(const std::wstring& text, COLORREF& color);     // "#rrggbb" / "rrggbb"
COLORREF ContrastColor(COLORREF color);                         // 按亮度给黑或白
COLORREF BlendColor(COLORREF from, COLORREF to, double amount);  // 0 = from, 1 = to
bool IsSystemDarkTheme();
ThemeColors ResolveTheme(const ThemeConfig& config);
const PresetDef* FindPreset(const std::wstring& key);
