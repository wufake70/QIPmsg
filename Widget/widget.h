#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QCloseEvent>
#include <QTextEdit>
#include <QTableWidgetItem>
#include <QSharedMemory>
#include <QDateTime>
#include <QTime>
#include <QApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDebug>
#include <QUrl>
#include <QSharedPointer>
#include <QMap>
#include <QMessageBox>
#include <QTimer>
#include "./FilesDialog/filesdialog.h"
#include "Qipmsg.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

    QSharedMemory singleton;
    int curShowPteIndex;
    int oldShowPteIndex;
    quint16 port;
    QString hostName;
    QList<QString> memberNameList;
    QList<QString> memberIpList;
    QList<QTextEdit*> pteList;

    QStringList localIpList;
    QUdpSocket *pUdpSocket;
    QTcpServer *pTcpServer;
    QMap<QString,QTcpSocket*> hostTcpSocketMap;
    QMap<QString,FilesDialog*> fdlgMap;
    QMap<QString,QMessageBox*> sendAckMsgBoxMap;

    QStringList filePathList;

    QTimer timer;

    void isSingleton();
    QStringList getLocalHostIps();
    inline QString ipv4ToString(int ipv4);

    void startUdpSocket();
    void stopUdpSocket();
    void sendUdpSocket(int type,QString ip="");
    void probeHost(QJsonObject&);
    void sendMsg(QJsonObject&);
    void sendMsgBuildTcp(QJsonObject&);
    void sendMsgFileAdd(QJsonObject&);
    void sendMsgFileDel(QJsonObject&);
    void sendMsgFileSendAck(QJsonObject&,QString ip="");
    void sendMsgFileReceiveAck(QJsonObject&,QString ip="");

    void initTcpServer();

signals:
    void closeTcpServerSocket(QString ip);
    void sendMsgFileAddOrDel(QString ip,bool isAdd,QString file);
    void sendMsgFileAddOrDel(QString ip,bool isAdd,QStringList fileList);
    void sendMsgFileSendAck(FilesDialog*);
    void switchTcpServerObjThread(QThread *target);

private slots:
    void UdpEvent();
    void updateMembersTable();
    void updateTextEdit();
    void updateTextEdit(QString);
    void updateTextEdit(int,QString);
    void clickTableWidgetItem(QTableWidgetItem*);
    void on_switchTcpServerObjThread(QThread *target);

    void on_pushButton_refresh_clicked();
    void on_pushButton_send_clicked();
    void on_newTcpSocketConnect();
    void on_destroyFilesDialog(QObject *obj = nullptr);
    void on_sendMsgFileAddOrDel(QString ip,bool isAdd,QString file);
    void on_sendMsgFileAddOrDel(QString ip,bool isAdd,QStringList fileList);
    void on_sendMsgFileSendAck(FilesDialog*);

    void on_pushButton_checkDir_clicked();

    void on_pushButton_help_about_clicked();

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;


public:
    QSharedPointer<QTcpServer> shptrTcpServer;
    QThread *pMainThread;
    static Widget* instance;
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
};

inline QString Widget::ipv4ToString(int ipv4)
{
    QString tempStr;
    tempStr.sprintf("%u.%u.%u.%u",
            (ipv4&0xff000000)>>24,
            (ipv4&0x00ff0000)>>16,
            (ipv4&0x0000ff00)>>8,
            (ipv4&0x000000ff)>>0);
    return tempStr;
}


#endif // WIDGET_H
