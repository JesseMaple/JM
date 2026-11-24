// Theme.cpp
#include "framework.h"
#include "Theme.h"

#include <cmath>

const PresetDef kPresets[] = {
    { L"yellow", L"明黄", RGB(0xff, 0xfd, 0xe6), RGB(0xff, 0xd7, 0x00) },
    { L"violet", L"紫罗兰", RGB(0xf5, 0xf0, 0xff), RGB(0x8a, 0x2b, 0xe2) },
    { L"sakura", L"樱花粉", RGB(0xff, 0xf0, 0xf5), RGB(0xff, 0x69, 0xb4) },
    { L"deepseek", L"深空蓝", RGB(0xf0, 0xf8, 0xff), RGB(0x4d, 0x6b, 0xfe) },
    { L"cyan", L"青色", RGB(0xf0, 0xff, 0xff), RGB(0x00, 0xce, 0xd1) },
};

const int kPresetCount = static_cast<int>(sizeof(kPresets) / sizeof(kPresets[0]));

namespace {

int HexDigit(wchar_t c) {
    if (c >= L'0' && c <= L'9') return c - L'0';
    if (c >= L'a' && c <= L'f') return c - L'a' + 10;
    if (c >= L'A' && c <= L'F') return c - L'A' + 10;
    return -1;
}

}  // namespace

std::wstring ColorToHex(COLORREF color) {
    wchar_t buffer[8] = {};
    swprintf_s(buffer, L"#%02x%02x%02x", GetRValue(color), GetGValue(color), GetBValue(color));
    return buffer;
}

bool HexToColor(const std::wstring& text, COLORREF& color) {
    std::wstring value = text;
    if (!value.empty() && value[0] == L'#') value.erase(0, 1);
    if (value.size() != 6) return false;

    int digits[6] = {};
    for (int i = 0; i < 6; ++i) {
        digits[i] = HexDigit(value[static_cast<size_t>(i)]);
        if (digits[i] < 0) return false;
    }
    color = RGB(digits[0] * 16 + digits[1], digits[2] * 16 + digits[3], digits[4] * 16 + digits[5]);
    return true;
}

COLORREF ContrastColor(COLORREF color) {
    const double luminance =
        (0.299 * GetRValue(color) + 0.587 * GetGValue(color) + 0.114 * GetBValue(color)) / 255.0;
    return luminance > 0.5 ? RGB(0x00, 0x00, 0x00) : RGB(0xff, 0xff, 0xff);
}

COLORREF BlendColor(COLORREF from, COLORREF to, double amount) {
    const auto mix = [amount](int a, int b) {
        const int value = static_cast<int>(std::lround(a + (b - a) * amount));
        return static_cast<BYTE>(value < 0 ? 0 : (value > 255 ? 255 : value));
    };
    return RGB(mix(GetRValue(from), GetRValue(to)), mix(GetGValue(from), GetGValue(to)),
               mix(GetBValue(from), GetBValue(to)));
}

bool IsSystemDarkTheme() {
    DWORD value = 1;
    DWORD size = sizeof(value);
    const LONG status = RegGetValueW(HKEY_CURRENT_USER,
                                     L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                                     L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &value, &size);
    if (status != ERROR_SUCCESS) return false;
    return value == 0;
}

ThemeColors ResolveTheme(const ThemeConfig& config) {
    ThemeColors colors;

    if (config.mode == L"dark" || (config.mode == L"system" && IsSystemDarkTheme())) {
        colors.bg = RGB(0x2b, 0x2b, 0x2b);
        colors.text = RGB(0xff, 0xff, 0xff);
        colors.btnBg = RGB(0x3c, 0x3f, 0x41);
        colors.btnFg = RGB(0xff, 0xff, 0xff);
    } else if (config.mode == L"custom") {
        colors.bg = config.customBg;
        colors.text = config.customFont;
        colors.btnBg = config.customBtn;
        colors.btnFg = ContrastColor(config.customBtn);
    } else {
        colors.bg = RGB(0xf0, 0xf0, 0xf0);
        colors.text = RGB(0x00, 0x00, 0x00);
        colors.btnBg = RGB(0xe1, 0xe1, 0xe1);
        colors.btnFg = RGB(0x00, 0x00, 0x00);
    }
    return colors;
}

const PresetDef* FindPreset(const std::wstring& key) {
    for (int i = 0; i < kPresetCount; ++i) {
        if (key == kPresets[i].key) return &kPresets[i];
    }
    return nullptr;
}
