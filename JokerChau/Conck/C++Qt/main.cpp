#include <QApplication>
#include "ConckMainWin.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ConckMainWin mainWin;
    mainWin.show();
    return QApplication::exec();
}
