#ifndef FILESTCPSERVER_H
#define FILESTCPSERVER_H

#include <QObject>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>

class ServerThread: public QThread
{
    Q_OBJECT
    QSharedPointer<QTcpSocket> shptrServerTcpSocket;
signals:
    void closeFilesDialoged();
    void sendFileTcpSocket();
public:
    ServerThread(QSharedPointer<QTcpSocket> shptr,QObject *parent)
    {
        shptrServerTcpSocket =shptr;
        setParent(parent);
    }
    ~ServerThread()
    {
        this->quit();
        this->wait();
    }

protected:
    void run() override;
};





class FilesTcpServer: public QObject
{
    Q_OBJECT

private slots:
    void connectClient();
    void disconnectClient();
    void readSocketData();
    void on_sendFileTcpSocket();
    void on_switchTcpServerThread();

public:
    QThread *parentThread;

    QSharedPointer<QTcpSocket> shptrServerTcpSocket;

    FilesTcpServer(QSharedPointer<QTcpSocket> shptr,QObject *parent){
        shptrServerTcpSocket =shptr;
        parentThread = (QThread*)parent;
//        setParent(parent); //  Cannot set parent, new parent is in a different thread

        connect(shptrServerTcpSocket.get(),SIGNAL(connected()),this,SLOT(connectClient()));
        connectClient();
        connect(shptrServerTcpSocket.get(),SIGNAL(readyRead()),this,SLOT(readSocketData()));
        connect(shptrServerTcpSocket.get(),SIGNAL(disconnected()),this,SLOT(disconnectClient()));
        connect(shptrServerTcpSocket.get(),SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                this,SLOT(OnSocketStateChanged(QAbstractSocket::SocketState)));
    }
    ~FilesTcpServer()
    {

//        emit shptrServerTcpSocket->disconnected();
//        parentThread->deleteLater();
    }

};

#endif // FILESTCPSERVER_H
