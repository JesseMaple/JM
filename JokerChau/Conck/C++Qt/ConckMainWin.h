//
// Created by JokerChau on 2026/11/24.
//

#ifndef CONCKCPP_CONCKWIN_H
#define CONCKCPP_CONCKWIN_H

#include <QMainWindow>
#include <QString>

struct ConckRect {
    int x, y, w, h;

    ConckRect();
};

class ConckMainWin : public QMainWindow {
    Q_OBJECT
public:
    ConckMainWin(QWidget *parent = nullptr);

private:
    void search(const QString &text);
};

#endif // CONCKCPP_CONCKWIN_H
