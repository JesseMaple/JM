using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Encodings.Web;
using System.Text.Json;

namespace ContactBook;

/// <summary>本地数据存储：%APPDATA%\ContactBook\contacts.json + avatars 目录。</summary>
public static class Store
{
    private static bool _rootResolved;

    private static readonly JsonSerializerOptions JsonOptions = new()    {
        WriteIndented = true,
        Encoder = JavaScriptEncoder.UnsafeRelaxedJsonEscaping,
        PropertyNameCaseInsensitive = true
    };

    public static string RootDir { get; private set; } = DefaultRoot;

    /// <summary>%APPDATA% 不可写时是否启用了备用目录（数据放在 exe 旁边的 data 或 LOCALAPPDATA）。</summary>
    public static bool UsedFallback { get; private set; }

    private static string DefaultRoot => Path.Combine(
        Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "ContactBook");

    public static string AvatarDir => Path.Combine(RootDir, "avatars");

    public static string DataFile => Path.Combine(RootDir, "contacts.json");

    public static List<Person> Contacts { get; private set; } = new();

    public static void Load()
    {
        // 数据目录只在进程内解析一次：避免运行期间权限变化导致数据“换了个文件夹”，看起来像联系人丢了。
        if (!_rootResolved)
        {
            RootDir = ResolveWritableRoot(out bool fallback);
            UsedFallback = fallback;
            _rootResolved = true;
        }

        try
        {
            Directory.CreateDirectory(RootDir);
            Directory.CreateDirectory(AvatarDir);
        }
        catch (Exception)
        {
            // 目录不可写时仍然可以只读运行（内存中的示例数据）。
            Contacts = Seed();
            return;
        }

        if (!File.Exists(DataFile))
        {
            Contacts = Seed();
            TrySave();
            return;
        }

        try
        {
            var json = File.ReadAllText(DataFile);
            var loaded = JsonSerializer.Deserialize<List<Person>>(json, JsonOptions);
            Contacts = loaded ?? new List<Person>();
            Normalize(Contacts);
        }
        catch (Exception)
        {
            // 数据损坏时保留一份备份，避免直接覆盖用户数据。
            try
            {
                File.Copy(DataFile, DataFile + ".broken", true);
            }
            catch
            {
                // 忽略备份失败。
            }
            Contacts = Seed();
            TrySave();
        }
    }

    private static void TrySave()
    {
        try
        {
            Save();
        }
        catch (Exception)
        {
            // 只读环境（例如受限沙箱）下继续以内存数据运行。
        }
    }

    /// <summary>依次尝试 %APPDATA%、%LOCALAPPDATA%、exe 同目录的 data，选第一个真正可写的。</summary>
    private static string ResolveWritableRoot(out bool fallback)
    {
        fallback = false;

        var local = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "ContactBook");
        var portable = Path.Combine(AppContext.BaseDirectory, "data");

        if (IsWritable(DefaultRoot)) return DefaultRoot;
        if (IsWritable(local))
        {
            fallback = true;
            return local;
        }
        if (IsWritable(portable))
        {
            fallback = true;
            return portable;
        }
        return DefaultRoot;
    }

    private static bool IsWritable(string dir)
    {
        try
        {
            Directory.CreateDirectory(dir);
            var probe = Path.Combine(dir, ".write-test");
            File.WriteAllText(probe, "ok");
            File.Delete(probe);
            return true;
        }
        catch (Exception)
        {
            return false;
        }
    }

    private static void Normalize(IEnumerable<Person> people)
    {
        foreach (var p in people)
        {
            p.Phones ??= new List<string>();
            p.Emails ??= new List<string>();
            p.Addresses ??= new List<AddressEntry>();
            p.Groups ??= new List<string>();
            p.AvatarFile ??= "";
            if (string.IsNullOrEmpty(p.Id)) p.Id = Guid.NewGuid().ToString("N");
        }
    }

    public static void Save()
    {
        Directory.CreateDirectory(RootDir);
        var json = JsonSerializer.Serialize(Contacts, JsonOptions);
        var tmp = DataFile + ".tmp";
        File.WriteAllText(tmp, json);
        File.Move(tmp, DataFile, true);
    }

    /// <summary>把选中的图片复制到 avatars 目录，返回文件名。</summary>
    public static string ImportAvatar(string sourcePath)
    {
        Directory.CreateDirectory(AvatarDir);
        var ext = Path.GetExtension(sourcePath).ToLowerInvariant();
        if (ext != ".png" && ext != ".jpg" && ext != ".jpeg") ext = ".png";
        var name = Guid.NewGuid().ToString("N") + ext;
        File.Copy(sourcePath, Path.Combine(AvatarDir, name), true);
        return name;
    }

    public static string? AvatarFullPath(string? file)
    {
        if (string.IsNullOrWhiteSpace(file)) return null;
        var full = Path.Combine(AvatarDir, file);
        return File.Exists(full) ? full : null;
    }

    private static List<Person> Seed()
    {
        return new List<Person>
        {
            NewPerson("张伟", new[] { "13800001111", "010-88886666" }, new[] { "zhangwei@example.com" },
                new[] { ("家", "北京市朝阳区建国路 88 号"), ("公司", "北京市海淀区中关村大街 1 号") },
                new[] { "朋友", "同事" }),
            NewPerson("李娜", new[] { "13911112222" }, new[] { "lina@example.com" },
                new[] { ("公司", "上海市浦东新区世纪大道 100 号") }, new[] { "同事" }),
            NewPerson("王芳", new[] { "13722223333" }, new[] { "wangfang@example.com" },
                new[] { ("家", "杭州市西湖区文三路 21 号") }, new[] { "家人" }),
            NewPerson("陈晨", new[] { "13633334444", "13633334445" }, new[] { "chenchen@example.com" },
                new[] { ("学校", "南京市鼓楼区汉口路 22 号") }, new[] { "同学" }),
            NewPerson("刘洋", new[] { "13544445555" }, new[] { "liuyang@example.com" },
                new[] { ("公司", "广州市天河区体育西路 103 号") }, new[] { "客户" }),
            NewPerson("赵敏", new[] { "13355556666" }, new[] { "zhaomin@example.com" },
                new[] { ("家", "成都市武侯区人民南路 4 段 1 号") }, new[] { "家人" }),
            NewPerson("孙悦", new[] { "13266667777" }, new[] { "sunyue@example.com" },
                new[] { ("家", "武汉市洪山区珞喻路 129 号") }, new[] { "朋友" }),
            NewPerson("周杰", new[] { "13177778888" }, new[] { "zhoujie@example.com" },
                new[] { ("公司", "苏州市工业园区星海街 9 号") }, new[] { "同事", "客户" }),
            NewPerson("吴敏", new[] { "13088889999" }, new[] { "wumin@example.com" },
                new[] { ("学校", "西安市碑林区咸宁西路 28 号") }, new[] { "同学" }),
            NewPerson("Alice Chen", new[] { "18800008888" }, new[] { "alice@example.com" },
                new[] { ("公司", "深圳市南山区科技园南区 8 栋") }, new[] { "客户", "朋友" }),
            NewPerson("alice wu", new[] { "18700007777" }, new[] { "alice.wu@example.com" },
                new[] { ("家", "厦门市思明区环岛路 33 号") }, new[] { "朋友" }),
            NewPerson("bob li", new[] { "18600006666" }, new[] { "bob@example.com" },
                new[] { ("公司", "东莞市松山湖科技十路 2 号") }, new[] { "供应商" }),
            NewPerson("Emily Wang", new[] { "18900009999" }, new[] { "emily@example.com" },
                new[] { ("公司", "重庆市渝中区解放碑 18 号") }, new[] { "客户" }),
        };
    }

    private static Person NewPerson(string name, string[] phones, string[] emails,
        (string Tag, string Text)[] addresses, string[] groups)
    {
        return new Person
        {
            Name = name,
            Phones = phones.ToList(),
            Emails = emails.ToList(),
            Addresses = addresses.Select(a => new AddressEntry { Tag = a.Tag, Text = a.Text }).ToList(),
            Groups = groups.ToList()
        };
    }
}
