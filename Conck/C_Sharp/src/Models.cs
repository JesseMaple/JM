using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.Json.Serialization;

namespace ContactBook;

/// <summary>一条地址：标签可以为空，为空时界面上不显示标签。</summary>
public sealed class AddressEntry
{
    public string Tag { get; set; } = "";
    public string Text { get; set; } = "";

    [JsonIgnore]
    public string Display => string.IsNullOrWhiteSpace(Tag) ? Text : $"{Tag} {Text}";
}

/// <summary>联系人。</summary>
public sealed class Person
{
    public string Id { get; set; } = Guid.NewGuid().ToString("N");
    public string Name { get; set; } = "";
    /// <summary>头像文件名（相对 avatars 目录），空字符串表示透明头像。</summary>
    public string AvatarFile { get; set; } = "";
    public List<string> Phones { get; set; } = new();
    public List<string> Emails { get; set; } = new();
    public List<AddressEntry> Addresses { get; set; } = new();
    public List<string> Groups { get; set; } = new();

    [JsonIgnore]
    public char Initial => NameSort.InitialOf(Name);

    [JsonIgnore]
    public string PhoneText => NameSort.Summarize(Phones);

    [JsonIgnore]
    public string EmailText => NameSort.Summarize(Emails);

    [JsonIgnore]
    public string AddressText => NameSort.Summarize(Addresses.Select(a => a.Display));

    [JsonIgnore]
    public string GroupText => NameSort.Summarize(Groups);

    [JsonIgnore]
    public string DisplayName => string.IsNullOrWhiteSpace(Name) ? "未命名" : Name;

    public Person Clone()
    {
        return new Person
        {
            Id = Id,
            Name = Name,
            AvatarFile = AvatarFile,
            Phones = new List<string>(Phones),
            Emails = new List<string>(Emails),
            Addresses = Addresses.Select(a => new AddressEntry { Tag = a.Tag, Text = a.Text }).ToList(),
            Groups = new List<string>(Groups)
        };
    }

    public void CopyFrom(Person other)
    {
        Name = other.Name;
        AvatarFile = other.AvatarFile;
        Phones = new List<string>(other.Phones);
        Emails = new List<string>(other.Emails);
        Addresses = other.Addresses.Select(a => new AddressEntry { Tag = a.Tag, Text = a.Text }).ToList();
        Groups = new List<string>(other.Groups);
    }
}

/// <summary>姓名排序：先按首字母 A→Z（其它字符归到 #），同字母时大写在前。</summary>
public static class NameSort
{
    private static readonly (int Start, int End, char Letter)[] GbRanges =
    {
        (0xB0A1, 0xB0C4, 'A'), (0xB0C5, 0xB2C0, 'B'), (0xB2C1, 0xB4ED, 'C'),
        (0xB4EE, 0xB6E9, 'D'), (0xB6EA, 0xB7A1, 'E'), (0xB7A2, 0xB8C0, 'F'),
        (0xB8C1, 0xB9FD, 'G'), (0xB9FE, 0xBBF6, 'H'), (0xBBF7, 0xBFA5, 'J'),
        (0xBFA6, 0xC0AB, 'K'), (0xC0AC, 0xC2E7, 'L'), (0xC2E8, 0xC4C2, 'M'),
        (0xC4C3, 0xC5B5, 'N'), (0xC5B6, 0xC5BD, 'O'), (0xC5BE, 0xC6D9, 'P'),
        (0xC6DA, 0xC8BA, 'Q'), (0xC8BB, 0xC8F5, 'R'), (0xC8F6, 0xCBF9, 'S'),
        (0xCBFA, 0xCDD9, 'T'), (0xCDDA, 0xCEF3, 'W'), (0xCEF4, 0xD188, 'X'),
        (0xD1B9, 0xD4D0, 'Y'), (0xD4D1, 0xD7F9, 'Z'),
    };

    private static readonly System.Text.Encoding? Gb2312 = CreateGb2312();

    private static System.Text.Encoding? CreateGb2312()
    {
        try
        {
            System.Text.Encoding.RegisterProvider(System.Text.CodePagesEncodingProvider.Instance);
            return System.Text.Encoding.GetEncoding(936);
        }
        catch (Exception)
        {
            return null;
        }
    }

    /// <summary>取汉字拼音首字母（GB2312 编码按拼音排序），非汉字返回 null。</summary>
    public static char? PinyinInitial(char c)
    {
        if (Gb2312 is null) return null;
        if (c < 0x4E00 || c > 0x9FFF) return null;

        try
        {
            var bytes = Gb2312.GetBytes(c.ToString());
            if (bytes.Length != 2) return null;
            int code = bytes[0] * 256 + bytes[1];
            foreach (var range in GbRanges)
            {
                if (code >= range.Start && code <= range.End) return range.Letter;
            }
        }
        catch (Exception)
        {
            return null;
        }
        return null;
    }

    public static char InitialOf(string? name)
    {
        var trimmed = (name ?? "").TrimStart();
        if (trimmed.Length == 0) return '#';
        var c = trimmed[0];
        if (c >= 'A' && c <= 'Z') return c;
        if (c >= 'a' && c <= 'z') return char.ToUpperInvariant(c);
        return PinyinInitial(c) ?? '#';
    }

    public static int Rank(char initial) => initial == '#' ? 27 : initial - 'A' + 1;

    /// <summary>首字母分组顺序；组内用 Ordinal 比较，正好让大写字母排在相同字母的小写之前。</summary>
    public static int Compare(Person a, Person b)
    {
        int r = Rank(a.Initial).CompareTo(Rank(b.Initial));
        if (r != 0) return r;
        int n = string.CompareOrdinal(a.Name ?? "", b.Name ?? "");
        if (n != 0) return n;
        return string.CompareOrdinal(a.Id, b.Id);
    }

    public static string Summarize(IEnumerable<string> items, int max = 2)
    {
        var list = items.Where(s => !string.IsNullOrWhiteSpace(s)).ToList();
        if (list.Count == 0) return "";
        if (list.Count <= max) return string.Join("，", list);
        return string.Join("，", list.Take(max)) + " 等";
    }
}

/// <summary>分组标签的配色（循环使用）。</summary>
public static class GroupPalette
{
    private static readonly (string Background, string Foreground)[] Colors =
    {
        ("#EDE3F8", "#7B3FA8"),
        ("#DDF3E7", "#1F8A5B"),
        ("#FBEEDB", "#A5761F"),
        ("#E2F0FD", "#0B69B5"),
        ("#FBE3EA", "#B23A5B"),
    };

    public static (string Background, string Foreground) For(string group, int index = -1)
    {
        if (index < 0)
        {
            index = 0;
            foreach (var ch in group ?? "")
                index = (index * 31 + ch) & 0x7FFFFFFF;
        }
        return Colors[Math.Abs(index) % Colors.Length];
    }
}
