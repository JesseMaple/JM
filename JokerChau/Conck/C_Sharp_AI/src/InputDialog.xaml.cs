using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace ContactBook;

public partial class InputDialog : Window
{
    private readonly List<TextBox> _boxes = new();
    private readonly List<bool> _required = new();

    public InputDialog(string title, params (string Label, string Placeholder, bool Required)[] fields)
    {
        InitializeComponent();
        TitleText.Text = title;
        Title = title;

        foreach (var field in fields)
        {
            var stack = new StackPanel { Margin = new Thickness(0, 0, 0, 14) };
            stack.Children.Add(new TextBlock
            {
                Text = field.Label,
                FontSize = 12,
                Foreground = (Brush)FindResource("MutedBrush"),
                Margin = new Thickness(2, 0, 0, 6)
            });

            var box = new TextBox
            {
                FontSize = 14,
                Height = 40,
                Padding = new Thickness(10, 8, 10, 6),
                Background = Brushes.Transparent,
                BorderThickness = new Thickness(0),
                VerticalContentAlignment = VerticalAlignment.Center
            };
            if (!string.IsNullOrEmpty(field.Placeholder)) box.ToolTip = field.Placeholder;

            var shell = new Border
            {
                CornerRadius = new CornerRadius(10),
                Background = Brushes.White,
                BorderBrush = new SolidColorBrush(Color.FromRgb(0xDC, 0xE6, 0xF0)),
                BorderThickness = new Thickness(1),
                Child = box
            };

            stack.Children.Add(shell);
            FieldsPanel.Children.Add(stack);
            _boxes.Add(box);
            _required.Add(field.Required);
        }

        Loaded += (_, _) =>
        {
            if (_boxes.Count > 0) _boxes[0].Focus();
        };
    }

    public string[] Values => _boxes.Select(b => b.Text.Trim()).ToArray();

    private void Ok_Click(object sender, RoutedEventArgs e)
    {
        var values = Values;
        for (int i = 0; i < values.Length; i++)
        {
            if (_required[i] && values[i].Length == 0)
            {
                MessageBox.Show(this, "这一项不能为空。", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
                _boxes[i].Focus();
                return;
            }
        }
        DialogResult = true;
    }

    private void Cancel_Click(object sender, RoutedEventArgs e) => DialogResult = false;

    // ---------------- 便捷入口 ----------------

    public static string? AskSingle(Window owner, string title, string label, string placeholder = "")
    {
        var dialog = new InputDialog(title, (label, placeholder, true)) { Owner = owner };
        return dialog.ShowDialog() == true ? dialog.Values[0] : null;
    }

    public static (string Tag, string Text)? AskAddress(Window owner)
    {
        var dialog = new InputDialog("添加地址",
            ("地址标签（可留空，默认为无标签）", "例如：家、公司", false),
            ("地址", "例如：北京市朝阳区建国路 88 号", true)) { Owner = owner };

        if (dialog.ShowDialog() != true) return null;
        var values = dialog.Values;
        return (values[0], values[1]);
    }
}
