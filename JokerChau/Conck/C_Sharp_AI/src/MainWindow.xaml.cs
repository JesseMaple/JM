using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Threading;

namespace ContactBook;

public partial class MainWindow : Window
{
    private readonly List<ContactCard> _cards = new();
    private readonly DispatcherTimer _searchTimer;
    private readonly DispatcherTimer _collapseTimer;

    private ContactCard? _selectedCard;
    private Person? _selectedPerson;
    private bool _sidebarExpanded;
    private string _activeFilter = "";

    public MainWindow()
    {
        InitializeComponent();

        _searchTimer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(300) };
        _searchTimer.Tick += (_, _) =>
        {
            _searchTimer.Stop();
            RunSearch();
        };

        _collapseTimer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(100) };
        _collapseTimer.Tick += (_, _) =>
        {
            _collapseTimer.Stop();
            if (!Sidebar.IsMouseOver) CollapseSidebar();
        };

        Sidebar.MouseEnter += (_, _) =>
        {
            _collapseTimer.Stop();
            ExpandSidebar();
        };
        Sidebar.MouseLeave += (_, _) =>
        {
            _collapseTimer.Stop();
            _collapseTimer.Start();
        };

        SizeChanged += (_, _) => PositionResults();
        Loaded += (_, _) =>
        {
            FitToScreen();
            PositionResults();
        };

        ReloadCards();
        BuildSidebar();

        DataPathText.Text = (Store.UsedFallback ? "数据目录（备用）：" : "数据目录：") + Store.RootDir;
        DataPathText.ToolTip = Store.RootDir;
    }

    private void FitToScreen()
    {
        var area = SystemParameters.WorkArea;

        if (Width > area.Width - 16 || Height > area.Height - 16)
        {
            Width = Math.Min(Width, area.Width - 16);
            Height = Math.Min(Height, area.Height - 16);
            Left = area.Left + (area.Width - Width) / 2;
            Top = area.Top + (area.Height - Height) / 2;
        }
        MinWidth = Math.Min(MinWidth, Width);
        MinHeight = Math.Min(MinHeight, Height);
    }

    // ============================ 卡片列表 ============================

    private void ReloadCards()
    {
        CardsPanel.Children.Clear();
        _cards.Clear();
        _selectedCard = null;
        _selectedPerson = null;
        DeleteBtn.IsEnabled = false;

        var filtered = !string.IsNullOrWhiteSpace(_activeFilter);
        var list = filtered
            ? SearchEngine.Search(Store.Contacts, _activeFilter).Select(h => h.Person).ToList()
            : Store.Contacts
                .OrderBy(p => p, Comparer<Person>.Create(NameSort.Compare))
                .ToList();

        foreach (var person in list)
        {
            var card = new ContactCard
            {
                Margin = new Thickness(0, 0, 18, 18)
            };
            card.Bind(person);
            card.Selected += (_, _) => SelectCard(card);
            card.Activated += (_, _) => OpenPerson(person);
            CardsPanel.Children.Add(card);
            _cards.Add(card);
        }

        EmptyHint.Text = filtered
            ? "没有找到匹配的联系人"
            : "还没有联系人，点击下方「添加」开始吧";
        EmptyHint.Visibility = list.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
    }

    private void SelectCard(ContactCard card)
    {
        foreach (var c in _cards) c.SetSelected(ReferenceEquals(c, card));
        _selectedCard = card;
        _selectedPerson = card.Person;
        DeleteBtn.IsEnabled = true;
    }

    private void ClearSelection()
    {
        foreach (var c in _cards) c.SetSelected(false);
        _selectedCard = null;
        _selectedPerson = null;
        DeleteBtn.IsEnabled = false;
    }

    private void SelectPerson(Person person, bool scroll = true)
    {
        var card = _cards.FirstOrDefault(c => c.Person.Id == person.Id);
        if (card is null) return;
        SelectCard(card);
        if (scroll) card.BringIntoView();
    }

    private void CardsScroll_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
    {
        var hit = CardsScroll.InputHitTest(e.GetPosition(CardsScroll)) as DependencyObject;
        if (hit is not null && FindAncestor<ContactCard>(hit) is not null) return;
        ClearSelection();
    }

    private static T? FindAncestor<T>(DependencyObject node) where T : DependencyObject
    {
        var current = node;
        while (current is not null)
        {
            if (current is T match) return match;
            current = VisualTreeHelper.GetParent(current);
        }
        return null;
    }

    // ============================ 侧边栏 ============================

    private void BuildSidebar()
    {
        SidebarPanel.Children.Clear();
        SidebarCount.Text = $"{Store.Contacts.Count} 位联系人 · 按姓名排序";

        var sorted = Store.Contacts
            .OrderBy(p => p, Comparer<Person>.Create(NameSort.Compare))
            .ToList();

        char current = '\0';
        foreach (var person in sorted)
        {
            if (person.Initial != current)
            {
                current = person.Initial;
                SidebarPanel.Children.Add(new TextBlock
                {
                    Text = current.ToString(),
                    FontSize = 20,
                    FontWeight = FontWeights.Bold,
                    Foreground = (Brush)FindResource("PrimaryBrush"),
                    Margin = new Thickness(8, 14, 0, 4)
                });
            }

            var phones = person.Phones.Count > 0 ? person.Phones : new List<string> { "" };
            foreach (var phone in phones)
            {
                SidebarPanel.Children.Add(CreateSidebarRow(person, phone));
            }
        }
    }

    private Border CreateSidebarRow(Person person, string phone)
    {
        var stack = new StackPanel();
        stack.Children.Add(new TextBlock
        {
            Text = person.DisplayName,
            FontSize = 13.5,
            FontWeight = FontWeights.SemiBold,
            Foreground = (Brush)FindResource("InkBrush"),
            TextTrimming = TextTrimming.CharacterEllipsis
        });
        stack.Children.Add(new TextBlock
        {
            Text = string.IsNullOrEmpty(phone) ? "无电话号码" : phone,
            FontSize = 11.5,
            Foreground = (Brush)FindResource("MutedBrush"),
            Margin = new Thickness(0, 2, 0, 0),
            TextTrimming = TextTrimming.CharacterEllipsis
        });

        var row = new Border
        {
            Style = (Style)FindResource("SidebarItemStyle"),
            Child = stack,
            Tag = person
        };
        row.MouseLeftButtonDown += (_, e) =>
        {
            if (e.ClickCount == 2)
            {
                OpenPerson(person);
                e.Handled = true;
            }
            else
            {
                SelectPerson(person);
            }
        };
        return row;
    }

    private void ExpandSidebar()
    {
        if (_sidebarExpanded) return;
        _sidebarExpanded = true;

        SidebarGrip.Visibility = Visibility.Collapsed;
        SidebarContent.IsHitTestVisible = true;

        var ease = new CubicEase { EasingMode = EasingMode.EaseOut };
        Sidebar.BeginAnimation(WidthProperty, new DoubleAnimation(Sidebar.ActualWidth, 340,
            TimeSpan.FromMilliseconds(200)) { EasingFunction = ease });
        SidebarContent.BeginAnimation(OpacityProperty, new DoubleAnimation(SidebarContent.Opacity, 1,
            TimeSpan.FromMilliseconds(160)));
    }

    private void CollapseSidebar()
    {
        if (!_sidebarExpanded) return;
        _sidebarExpanded = false;

        var ease = new CubicEase { EasingMode = EasingMode.EaseIn };
        var anim = new DoubleAnimation(Sidebar.ActualWidth, 22, TimeSpan.FromMilliseconds(170))
        {
            EasingFunction = ease
        };
        anim.Completed += (_, _) =>
        {
            if (_sidebarExpanded) return;
            SidebarGrip.Visibility = Visibility.Visible;
            SidebarContent.IsHitTestVisible = false;
        };

        Sidebar.BeginAnimation(WidthProperty, anim);
        SidebarContent.BeginAnimation(OpacityProperty, new DoubleAnimation(SidebarContent.Opacity, 0,
            TimeSpan.FromMilliseconds(130)));
    }

    // ============================ 搜索 ============================

    private void SearchBox_TextChanged(object sender, TextChangedEventArgs e)
    {
        Placeholder.Visibility = string.IsNullOrEmpty(SearchBox.Text)
            ? Visibility.Visible
            : Visibility.Collapsed;

        _searchTimer.Stop();
        if (string.IsNullOrWhiteSpace(SearchBox.Text))
        {
            HideResults();
            return;
        }
        _searchTimer.Start();
    }

    private void SearchBox_KeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key == Key.Enter)
        {
            ApplyFilter();
            e.Handled = true;
        }
        else if (e.Key == Key.Escape)
        {
            HideResults();
            e.Handled = true;
        }
    }

    private void SearchButton_Click(object sender, RoutedEventArgs e) => ApplyFilter();

    private void RefreshButton_Click(object sender, RoutedEventArgs e) => ShowAllContacts();

    private void Window_PreviewKeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key == Key.F5 || (e.Key == Key.R && Keyboard.Modifiers == ModifierKeys.Control))
        {
            ShowAllContacts();
            e.Handled = true;
        }
    }

    private void RunSearch()
    {
        var query = SearchBox.Text;
        if (string.IsNullOrWhiteSpace(query))
        {
            HideResults();
            return;
        }

        var hits = SearchEngine.Search(Store.Contacts, query);
        ResultsPanel.Children.Clear();

        if (hits.Count == 0)
        {
            ResultsPanel.Children.Add(new TextBlock
            {
                Text = "没有找到匹配的联系人",
                Foreground = (Brush)FindResource("MutedBrush"),
                FontSize = 13.5,
                Margin = new Thickness(14, 12, 14, 14)
            });
        }
        else
        {
            foreach (var hit in hits) ResultsPanel.Children.Add(CreateResultItem(hit));
        }

        ResultsPopup.Visibility = Visibility.Visible;
        PositionResults();
    }

    private Border CreateResultItem(SearchHit hit)
    {
        var stack = new StackPanel();
        stack.Children.Add(new TextBlock
        {
            Text = hit.Person.DisplayName,
            FontSize = 14.5,
            FontWeight = FontWeights.SemiBold,
            Foreground = (Brush)FindResource("InkBrush")
        });
        stack.Children.Add(new TextBlock
        {
            Text = hit.Snippet,
            FontSize = 12,
            Foreground = (Brush)FindResource("MutedBrush"),
            Margin = new Thickness(0, 2, 0, 0),
            TextTrimming = TextTrimming.CharacterEllipsis
        });

        var item = new Border
        {
            Style = (Style)FindResource("ResultItemStyle"),
            Child = stack,
            Tag = hit
        };
        item.MouseLeftButtonDown += (_, e) =>
        {
            if (e.ClickCount == 2)
            {
                HideResults();
                OpenPerson(hit.Person);
            }
            else
            {
                SelectPerson(hit.Person);
            }
        };
        return item;
    }

    private void PositionResults()
    {
        if (ResultsPopup.Visibility != Visibility.Visible) return;
        var point = SearchField.TranslatePoint(new Point(0, 0), OverlayCanvas);
        Canvas.SetLeft(ResultsPopup, point.X);
        Canvas.SetTop(ResultsPopup, point.Y + SearchField.ActualHeight + 8);
        ResultsPopup.Width = Math.Max(320, SearchField.ActualWidth);
    }

    private void HideResults()
    {
        ResultsPopup.Visibility = Visibility.Collapsed;
        ResultsPanel.Children.Clear();
    }

    // ============================ 操作 ============================

    /// <summary>把当前搜索词应用到主列表：只显示模糊匹配成功的联系人。</summary>
    private void ApplyFilter()
    {
        _searchTimer.Stop();
        _activeFilter = SearchBox.Text.Trim();
        HideResults();
        ReloadCards();
        CardsScroll.ScrollToTop();
    }

    /// <summary>清空搜索并重新从磁盘加载，显示全部联系人。</summary>
    private void ShowAllContacts()
    {
        _searchTimer.Stop();
        _activeFilter = "";
        SearchBox.Text = "";
        Placeholder.Visibility = Visibility.Visible;
        HideResults();

        Store.Load();
        ReloadCards();
        BuildSidebar();
        ClearSelection();
        CardsScroll.ScrollToTop();
    }

    private void Add_Click(object sender, RoutedEventArgs e)
    {
        var window = new ContactWindow(null) { Owner = this };
        if (window.ShowDialog() == true) AfterChange();
    }

    private void Delete_Click(object sender, RoutedEventArgs e)
    {
        if (_selectedPerson is null) return;

        var result = MessageBox.Show(
            $"确定要删除联系人「{_selectedPerson.DisplayName}」吗？\n删除后无法恢复。",
            "删除联系人",
            MessageBoxButton.YesNo,
            MessageBoxImage.Warning,
            MessageBoxResult.No);

        if (result != MessageBoxResult.Yes) return;

        var target = Store.Contacts.FirstOrDefault(p => p.Id == _selectedPerson.Id);
        if (target is not null)
        {
            Store.Contacts.Remove(target);
            try
            {
                Store.Save();
            }
            catch (Exception ex)
            {
                Store.Load();
                AfterChange();
                MessageBox.Show(this, "删除后保存失败，联系人已恢复：\n" + ex.Message, "保存失败",
                    MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }
        }
        AfterChange();
    }

    private void OpenPerson(Person person)
    {
        var window = new ContactWindow(person) { Owner = this };
        if (window.ShowDialog() == true) AfterChange();
    }

    private void AfterChange()
    {
        Store.Load();
        ReloadCards();
        BuildSidebar();
        ClearSelection();
        HideResults();
    }
}
