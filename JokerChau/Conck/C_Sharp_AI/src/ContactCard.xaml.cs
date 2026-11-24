using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Effects;
using System.Windows.Media.Imaging;

namespace ContactBook;

public partial class ContactCard : UserControl
{
    private bool _selected;

    public ContactCard()
    {
        InitializeComponent();
        Root.Effect = (Effect)FindResource("CardShadow");
    }

    public Person Person { get; private set; } = null!;

    public event EventHandler? Selected;
    public event EventHandler? Activated;

    public void Bind(Person person)
    {
        Person = person;

        NameText.Text = person.DisplayName;
        ToolTip = person.DisplayName;

        BindAvatar(person);

        PhoneText.Text = person.PhoneText;
        PhoneText.ToolTip = string.Join("\n", person.Phones);
        PhoneRow.Visibility = person.Phones.Count > 0 ? Visibility.Visible : Visibility.Collapsed;

        EmailText.Text = person.EmailText;
        EmailText.ToolTip = string.Join("\n", person.Emails);
        EmailRow.Visibility = person.Emails.Count > 0 ? Visibility.Visible : Visibility.Collapsed;

        AddressText.Text = person.AddressText;
        AddressText.ToolTip = string.Join("\n", person.Addresses.ConvertAll(a => a.Display));
        AddressRow.Visibility = person.Addresses.Count > 0 ? Visibility.Visible : Visibility.Collapsed;

        GroupsPanel.Children.Clear();
        for (int i = 0; i < person.Groups.Count; i++)
        {
            var group = person.Groups[i];
            var (bg, fg) = GroupPalette.For(group, i);
            var chip = new Border
            {
                Background = (Brush)new BrushConverter().ConvertFromString(bg)!,
                CornerRadius = new CornerRadius(9),
                Padding = new Thickness(9, 2, 9, 3),
                Margin = new Thickness(0, 0, 6, 4),
                Child = new TextBlock
                {
                    Text = group,
                    FontSize = 11.5,
                    Foreground = (Brush)new BrushConverter().ConvertFromString(fg)!
                }
            };
            GroupsPanel.Children.Add(chip);
        }
    }

    private void BindAvatar(Person person)
    {
        var path = Store.AvatarFullPath(person.AvatarFile);
        if (path is null)
        {
            // 娌℃湁澶村儚鏃剁敤閫忔槑澶村儚銆?            AvatarBorder.Background = Brushes.Transparent;
            AvatarBorder.BorderThickness = new Thickness(0);
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
            AvatarBorder.BorderThickness = new Thickness(1);
            AvatarBorder.BorderBrush = (Brush)FindResource("LineBrush");
        }
        catch (Exception)
        {
            AvatarBorder.Background = Brushes.Transparent;
            AvatarBorder.BorderThickness = new Thickness(0);
        }
    }

    public void SetSelected(bool selected)
    {
        _selected = selected;
        if (selected)
        {
            Root.Background = (Brush)FindResource("SelectedBgBrush");
            Root.BorderBrush = (Brush)FindResource("PrimaryBrush");
            Root.BorderThickness = new Thickness(1.6);
            Root.Effect = (Effect)FindResource("CardShadowHot");
        }
        else
        {
            Root.Background = (Brush)FindResource("CardBgBrush");
            Root.BorderBrush = (Brush)FindResource("LineBrush");
            Root.BorderThickness = new Thickness(1);
            Root.Effect = (Effect)FindResource("CardShadow");
        }
    }

    private void Root_MouseEnter(object sender, System.Windows.Input.MouseEventArgs e)
    {
        if (_selected) return;
        Root.Background = (Brush)FindResource("CardHoverBrush");
        Root.BorderBrush = (Brush)new BrushConverter().ConvertFromString("#B7DCF8")!;
        Root.Effect = (Effect)FindResource("CardShadowHot");
    }

    private void Root_MouseLeave(object sender, System.Windows.Input.MouseEventArgs e)
    {
        if (_selected) return;
        Root.Background = (Brush)FindResource("CardBgBrush");
        Root.BorderBrush = (Brush)FindResource("LineBrush");
        Root.Effect = (Effect)FindResource("CardShadow");
    }

    private void Root_MouseLeftButtonDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
    {
        if (e.ClickCount == 2)
        {
            Activated?.Invoke(this, EventArgs.Empty);
            e.Handled = true;
            return;
        }
        Selected?.Invoke(this, EventArgs.Empty);
    }
}

