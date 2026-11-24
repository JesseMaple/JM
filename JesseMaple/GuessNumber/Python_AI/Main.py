import sys
from PySide6.QtWidgets import QApplication, QMainWindow, QStackedWidget, QMessageBox
from FileOperator import get_theme_config, set_saved_difficulty, set_win_count, set_max_steps, set_min_steps
from QtThemeManager import get_theme_qss
from QtViews import GameView, SettingsView


class GuessNumberApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("猜数字游戏 - PySide6版")
        self.resize(800, 500)

        # 读取主题并应用
        self.theme_config = get_theme_config()
        self.apply_theme()

        # 使用 QStackedWidget 管理页面（0为主页，1为设置页）
        self.stacked_widget = QStackedWidget()
        self.setCentralWidget(self.stacked_widget)

        self.game_view = GameView(self)
        self.settings_view = SettingsView(self)

        self.stacked_widget.addWidget(self.game_view)
        self.stacked_widget.addWidget(self.settings_view)

    def apply_theme(self):
        """重新生成 QSS 并应用到整个窗口"""
        qss = get_theme_qss(self.theme_config)
        self.setStyleSheet(qss)

    def show_main(self):
        self.setWindowTitle("猜数字游戏")
        self.stacked_widget.setCurrentIndex(0)

    def show_settings(self):
        self.setWindowTitle("猜数字游戏 - 设置")
        self.settings_view.refresh_ui()
        self.stacked_widget.setCurrentIndex(1)

    def refresh_theme(self):
        self.theme_config = get_theme_config()
        self.apply_theme()
        self.settings_view.refresh_ui()

    def closeEvent(self, event):
        """重写关闭事件，触发确认弹窗"""
        reply = QMessageBox.question(self, '退出', '是否确认退出游戏？\n（系统会自动保存你的游戏进度）',
                                     QMessageBox.Yes | QMessageBox.No, QMessageBox.No)

        if reply == QMessageBox.Yes:
            if self.stacked_widget.currentIndex() == 0:  # 如果在主页
                set_saved_difficulty(self.game_view.difficulty)
                set_win_count(self.game_view.difficulty, self.game_view.win_count)
                set_max_steps(self.game_view.difficulty, self.game_view.max_steps)
                set_min_steps(self.game_view.difficulty, self.game_view.min_steps)
            event.accept()
        else:
            event.ignore()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = GuessNumberApp()
    window.show()
    sys.exit(app.exec())