#include "./Widget/widget.h"

#include <QProcess>
#include <QApplication>

MainWidget* MainWidget::instance =nullptr;

void enableFireWallRuld();

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
//    QIcon appIcon(":/new/prefix1/file-transfer.png");
//    QApplication::setWindowIcon(appIcon);
    MainWidget w;
    w.setWindowTitle(QString("QIpmsg")+" "+QIPMSG_VERSION);
    MainWidget::instance = &w;
    w.show();
    return app.exec();
}
