using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace ContactBook;

public partial class ContactWindow : Window
{
    private readonly Person? _original;
    private readonly Person _draft;
    private string _avatarFile;

    public ContactWindow(Person? person)
    {
        InitializeComponent();

        _original = person;
        _draft = person?.Clone() ?? new Person();
        _avatarFile = _draft.AvatarFile;

        Title = person is null ? "新建联系人" : "编辑联系人";
        RenderAll();

        Loaded += (_, _) =>
        {
            var area = SystemParameters.WorkArea;
            Width = Math.Min(Width, Math.Max(560, area.Width - 40));
            Height = Math.Min(Height, Math.Max(480, area.Height - 40));

            if (Owner is not null)
            {
                Left = Owner.Left + (Owner.ActualWidth - Width) / 2;
                Top = Owner.Top + (Owner.ActualHeight - Height) / 2;
            }
            else
            {
                Left = area.Left + (area.Width - Width) / 2;
                Top = area.Top + (area.Height - Height) / 2;
            }

            Left = Math.Max(area.Left, Math.Min(Left, area.Right - Width));
            Top = Math.Max(area.Top, Math.Min(Top, area.Bottom - Height));
        };
    }

    private static Brush Hex(string color) => (Brush)new BrushConverter().ConvertFromString(color)!;

    private void RenderAll()
    {
        RenderName();
        RenderAvatar();
        RenderPhones();
        RenderEmails();
        RenderAddresses();
        RenderGroups();
    }

    // ---------------- 名字 ----------------

    private void RenderName()
    {
        NameText.Text = _draft.DisplayName;
        NameBox.Text = _draft.Name;
    }

    private void NameText_Click(object sender, MouseButtonEventArgs e)
    {
        NameBox.Text = _draft.Name;
        NameText.Visibility = Visibility.Collapsed;
        NameBox.Visibility = Visibility.Visible;
        NameBox.Focus();
        NameBox.SelectAll();
    }

    private void CommitName()
    {
        _draft.Name = NameBox.Text.Trim();
        NameText.Text = _draft.DisplayName;
        NameBox.Visibility = Visibility.Collapsed;
        NameText.Visibility = Visibility.Visible;
    }

    private void NameBox_LostFocus(object sender, RoutedEventArgs e)
    {
        if (NameBox.Visibility == Visibility.Visible) CommitName();
    }

    private void NameBox_KeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key == Key.Enter)
        {
            CommitName();
            e.Handled = true;
        }
        else if (e.Key == Key.Escape)
        {
            NameBox.Text = _draft.Name;
            NameBox.Visibility = Visibility.Collapsed;
            NameText.Visibility = Visibility.Visible;
            e.Handled = true;
        }
    }

    // ---------------- 头像 ----------------

    private void RenderAvatar()
    {
        var path = Store.AvatarFullPath(_avatarFile);
        if (path is null)
        {
            // 默认透明头像：只保留可点击的虚框提示。
            AvatarBorder.Background = Brushes.Transparent;
            AvatarBorder.BorderBrush = Hex("#C9D8E8");
            AvatarHint.Visibility = Visibility.Visible;
            return;
        }

        try
        {
            var bitmap = new BitmapImage();
            bitmap.BeginInit();
            bitmap.CacheOption = BitmapCacheOption.OnLoad;
            bitmap.UriSource = new Uri(path);
            bitmap.EndInit();
            bitmap.Freeze();

            AvatarBorder.Background = new ImageBrush(bitmap) { Stretch = Stretch.UniformToFill };
            AvatarBorder.BorderBrush = Hex("#DCE6F0");
            AvatarHint.Visibility = Visibility.Collapsed;
        }
        catch (Exception)
        {
            AvatarBorder.Background = Brushes.Transparent;
            AvatarHint.Visibility = Visibility.Visible;
        }
    }

    private void Avatar_Click(object sender, MouseButtonEventArgs e)
    {
        var dialog = new Microsoft.Win32.OpenFileDialog
        {
            Title = "选择头像图片",
            Filter = "图片文件 (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg",
            CheckFileExists = true
        };

        if (dialog.ShowDialog(this) != true) return;

        try
        {
            _avatarFile = Store.ImportAvatar(dialog.FileName);
            RenderAvatar();
        }
        catch (Exception ex)
        {
            MessageBox.Show(this, "导入头像失败：" + ex.Message, "错误",
                MessageBoxButton.OK, MessageBoxImage.Error);
        }
    }

    // ---------------- 标签块 ----------------

    private Border MakeChip(string text, Brush background, Brush foreground, Action remove,
        double maxWidth = 430)
    {
        var content = new StackPanel { Orientation = Orientation.Horizontal };
        content.Children.Add(new TextBlock
        {
            Text = text,
            FontSize = 13.5,
            Foreground = foreground,
            VerticalAlignment = VerticalAlignment.Center,
            TextTrimming = TextTrimming.CharacterEllipsis,
            MaxWidth = maxWidth,
            ToolTip = text
        });

        var close = new Button
        {
            Style = (Style)FindResource("ChipCloseButton"),
            Margin = new Thickness(8, 0, 0, 0),
            VerticalAlignment = VerticalAlignment.Center
        };
        close.Click += (_, _) => remove();
        content.Children.Add(close);

        return new Border
        {
            Background = background,
            CornerRadius = new CornerRadius(12),
            Padding = new Thickness(12, 4, 8, 5),
            Margin = new Thickness(0, 0, 8, 8),
            Child = content
        };
    }

    private void RenderPhones()
    {
        PhonePanel.Children.Clear();
        for (int i = 0; i < _draft.Phones.Count; i++)
        {
            var index = i;
            PhonePanel.Children.Add(MakeChip(_draft.Phones[index], Hex("#EFF7FE"), Hex("#1B5E8C"),
                () =>
                {
                    _draft.Phones.RemoveAt(index);
                    RenderPhones();
                }));
        }
        if (_draft.Phones.Count == 0) PhonePanel.Children.Add(EmptyHint("暂无电话，点击 + 添加"));
    }

    private void RenderEmails()
    {
        EmailPanel.Children.Clear();
        for (int i = 0; i < _draft.Emails.Count; i++)
        {
            var index = i;
            EmailPanel.Children.Add(MakeChip(_draft.Emails[index], Hex("#EFF7FE"), Hex("#1B5E8C"),
                () =>
                {
                    _draft.Emails.RemoveAt(index);
                    RenderEmails();
                }));
        }
        if (_draft.Emails.Count == 0) EmailPanel.Children.Add(EmptyHint("暂无邮箱，点击 + 添加"));
    }

    private void RenderAddresses()
    {
        AddressPanel.Children.Clear();
        for (int i = 0; i < _draft.Addresses.Count; i++)
        {
            var index = i;
            AddressPanel.Children.Add(MakeChip(_draft.Addresses[index].Display, Hex("#EFF7FE"), Hex("#1B5E8C"),
                () =>
                {
                    _draft.Addresses.RemoveAt(index);
                    RenderAddresses();
                }, 380));
        }
        if (_draft.Addresses.Count == 0) AddressPanel.Children.Add(EmptyHint("暂无地址，点击 + 添加"));
    }

    private void RenderGroups()
    {
        GroupPanel.Children.Clear();
        for (int i = 0; i < _draft.Groups.Count; i++)
        {
            var index = i;
            var (bg, fg) = GroupPalette.For(_draft.Groups[index], i);
            GroupPanel.Children.Add(MakeChip(_draft.Groups[index], Hex(bg), Hex(fg),
                () =>
                {
                    _draft.Groups.RemoveAt(index);
                    RenderGroups();
                }, 260));
        }
        if (_draft.Groups.Count == 0) GroupPanel.Children.Add(EmptyHint("暂无分组，点击 + 添加"));
    }

    private static TextBlock EmptyHint(string text) => new()
    {
        Text = text,
        FontSize = 12.5,
        Foreground = new SolidColorBrush(Color.FromRgb(0x9A, 0xAA, 0xBB)),
        Margin = new Thickness(2, 6, 0, 0)
    };

    // ---------------- 添加 / 删除 ----------------

    private void AddPhone_Click(object sender, RoutedEventArgs e)
    {
        var value = InputDialog.AskSingle(this, "添加电话", "电话号码", "例如：13800001111");
        if (string.IsNullOrWhiteSpace(value)) return;
        if (_draft.Phones.Contains(value))
        {
            MessageBox.Show(this, "这个电话号码已经存在了。", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
            return;
        }
        _draft.Phones.Add(value);
        RenderPhones();
    }

    private void AddEmail_Click(object sender, RoutedEventArgs e)
    {
        var value = InputDialog.AskSingle(this, "添加邮箱", "邮箱地址", "例如：name@example.com");
        if (string.IsNullOrWhiteSpace(value)) return;
        if (_draft.Emails.Contains(value))
        {
            MessageBox.Show(this, "这个邮箱已经存在了。", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
            return;
        }
        _draft.Emails.Add(value);
        RenderEmails();
    }

    private void AddAddress_Click(object sender, RoutedEventArgs e)
    {
        var result = InputDialog.AskAddress(this);
        if (result is null) return;

        var (tag, text) = result.Value;
        _draft.Addresses.Add(new AddressEntry { Tag = tag, Text = text });
        RenderAddresses();
    }

    private void AddGroup_Click(object sender, RoutedEventArgs e)
    {
        var value = InputDialog.AskSingle(this, "添加分组", "分组名称", "例如：家人、同事、客户");
        if (string.IsNullOrWhiteSpace(value)) return;
        if (_draft.Groups.Contains(value))
        {
            MessageBox.Show(this, "这个分组已经存在了。", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
            return;
        }
        _draft.Groups.Add(value);
        RenderGroups();
    }

    // ---------------- 保存 ----------------

    private void Save_Click(object sender, RoutedEventArgs e)
    {
        if (NameBox.Visibility == Visibility.Visible) CommitName();
        _draft.Name = _draft.Name.Trim();

        if (string.IsNullOrWhiteSpace(_draft.Name))
        {
            MessageBox.Show(this, "请先填写联系人名字。", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
            NameText_Click(this, null!);
            return;
        }

        _draft.AvatarFile = _avatarFile;

        if (_original is null)
        {
            Store.Contacts.Add(_draft);
        }
        else
        {
            var target = Store.Contacts.FirstOrDefault(p => p.Id == _original.Id);
            if (target is null) Store.Contacts.Add(_draft);
            else target.CopyFrom(_draft);
        }

        try
        {
            Store.Save();
        }
        catch (Exception ex)
        {
            Store.Load();
            MessageBox.Show(this, "保存失败：\n" + ex.Message, "保存失败",
                MessageBoxButton.OK, MessageBoxImage.Error);
            return;
        }
        DialogResult = true;
    }

    private void Cancel_Click(object sender, RoutedEventArgs e) => DialogResult = false;
}
