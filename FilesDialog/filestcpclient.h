#ifndef FILESTCPSOCKET_H
#define FILESTCPSOCKET_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QDebug>

class ClientThread: public QThread
{
    Q_OBJECT

    QString ip;
public:
    bool isReceiving;
    QObject *parentObj;
    ClientThread(QString ip_arg,QObject *parent)
    {
        isReceiving = false;
        parentObj = parent;
        setParent(parent);
        ip = ip_arg;
    }

signals:
    void closeFilesDialoged();
    void threadSendAckMsgBox();

protected:
    void run() override;
};



class FilesTcpClient:public QObject
{
    Q_OBJECT
public:
    QThread *parentThread;
    QString serverIp;
    QTcpSocket *pTcpSocket;

private slots:
    void connectServer();
    void disconnectServer();
    void readSocketData();
//public slots:
//    void on_threadSendAckMsgBox();

public:
    FilesTcpClient(QString serverIpArg,QObject *parent);
    ~FilesTcpClient()
    {
        pTcpSocket->deleteLater();
    }

};

#endif // FILESTCPSOCKET_H
