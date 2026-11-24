//
// Created by JokerChau on 2026/11/24.
//

#include "ConckMainWin.h"
#include <QDebug>
#include <QGuiApplication>
#include <QLineEdit>
#include <QScreen>
#include <QVBoxLayout>
#include <algorithm>

ConckMainWin::ConckMainWin(QWidget* parent)
    : QMainWindow(parent)          // 基类构造
{
    auto* central = new QWidget(this);     // this = 父对象,归 Qt 管
    setCentralWidget(central);
    setWindowTitle("Conck");

    auto* layout = new QVBoxLayout(central);

    auto* searchLine = new QLineEdit(central);
    searchLine->setPlaceholderText("请输入联系人信息……");
    layout->addWidget(searchLine);

    connect(searchLine, &QLineEdit::returnPressed, this, [this, searchLine] {
        search(searchLine->text());
    });

    ConckRect conckRect;
    resize(conckRect.w, conckRect.h);
    move(conckRect.x, conckRect.y);
}

void ConckMainWin::search(const QString &text) {
    qDebug() << "search:" << text;
}

ConckRect::ConckRect() {
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect area = screen->availableGeometry();

    int width = area.width();
    int height = area.height();
    int halfWidth = width / 2;
    int halfHeight = height / 2;

    int usableWidth = std::min(width, std::max(halfWidth, 800));
    int usableHeight = std::min(height, std::max(halfHeight, 600));

    this->x = area.x() + (area.width()  - usableWidth)  / 2;
    this->y = area.y() + (area.height() - usableHeight) / 2;
    this->w = usableWidth;
    this->h = usableHeight;
}
