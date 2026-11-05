import tkinter as tk
from tkinter import messagebox
from FileOperator import get_theme_config, set_saved_difficulty, set_win_count, set_max_steps, set_min_steps
from UIComponents import ThemeManager
from Views import MainGameView, SettingsView


class GuessNumberApp(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)
        self.title("猜数字游戏")
        self.geometry(
            f"{self.winfo_screenwidth() // 2}x{self.winfo_screenheight() // 2}+{self.winfo_screenwidth() // 4}+{self.winfo_screenheight() // 4}"
        )

        self.theme_config = get_theme_config()
        self.colors = ThemeManager.get_theme_colors(self.theme_config)

        self.current_view = None
        self.show_main()
        self.protocol("WM_DELETE_WINDOW", self.handle_window_close)

    def clear_current_view(self):
        if self.current_view:
            self.current_view.destroy()
            self.current_view = None

    def show_main(self):
        self.clear_current_view()
        self.title("猜数字游戏")
        self.configure(bg=self.colors["app_bg"])
        self.current_view = MainGameView(self, self)
        self.current_view.pack(fill="both", expand=True)

    def show_settings(self):
        self.clear_current_view()
        self.title("猜数字游戏 - 设置")
        self.configure(bg=self.colors["app_bg"])
        self.current_view = SettingsView(self, self)
        self.current_view.pack(fill="both", expand=True)

    def refresh_theme(self):
        self.theme_config = get_theme_config()
        self.colors = ThemeManager.get_theme_colors(self.theme_config)
        self.show_settings()

    def handle_window_close(self):
        if messagebox.askyesno("退出", "是否确认退出游戏？\n（系统会自动保存你的游戏进度）"):
            if isinstance(self.current_view, MainGameView):
                set_saved_difficulty(self.current_view.difficulty)
                set_win_count(self.current_view.difficulty, self.current_view.win_count)
                set_max_steps(self.current_view.difficulty, self.current_view.max_steps)
                set_min_steps(self.current_view.difficulty, self.current_view.min_steps)
            self.destroy()


if __name__ == "__main__":
    app = GuessNumberApp()
    app.mainloop()