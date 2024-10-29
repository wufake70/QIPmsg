#include "./Widget/widget.h"

#include <QProcess>
#include <QApplication>

Widget* Widget::instance =nullptr;

void enableFireWallRuld();

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
//    QIcon appIcon(":/new/prefix1/file-transfer.png");
//    QApplication::setWindowIcon(appIcon);
    Widget w;
    w.setWindowTitle(QString("QIpmsg")+" "+QIPMSG_VERSION);
    Widget::instance = &w;
    w.show();
    return app.exec();
}
