#include "./Widget/widget.h"

#include <QTranslator>
#include <QApplication>
#include <QLocale>

MainWidget* MainWidget::instance =nullptr;

void enableFireWallRuld();

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    QTranslator lang_en;
    if(QLocale::system().uiLanguages()[0]!="zh-CN") //
    {
        lang_en.load(":/lang/QIPmsg_en.qm");
        app.installTranslator(&lang_en);
    }

    MainWidget w;
    w.setWindowTitle(QString("QIPmsg")+" "+QIPMSG_VERSION);
    MainWidget::instance = &w;
    w.show();
    return app.exec();
}
