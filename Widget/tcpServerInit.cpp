#include "widget.h"
#include <QNetworkProxy>


void Widget::initTcpServer()
{
    QSharedPointer<QTcpServer> shptrTcpServer_(new QTcpServer());
    shptrTcpServer = shptrTcpServer_;
    shptrTcpServer->setProxy(QNetworkProxy::NoProxy);
    shptrTcpServer->listen(QHostAddress::Any,port);
    connect(shptrTcpServer.get(),
            SIGNAL(newConnection()),this,
            SLOT(on_newTcpSocketConnect()));
}

void Widget::on_newTcpSocketConnect()
{
    QTcpSocket *pTcpSocket = shptrTcpServer->nextPendingConnection();
    connect(pTcpSocket,SIGNAL(disconnected),
            this,SLOT([pTcpSocket](){
        pTcpSocket->close();
    }));
    pTcpSocket->setProxy(QNetworkProxy::NoProxy);
    hostTcpSocketMap[ipv4ToString(pTcpSocket->peerAddress().toIPv4Address())] = pTcpSocket;

    FilesDialog *pfdlg = new FilesDialog(memberIpList.at(curShowPteIndex),
                                       memberNameList.at(curShowPteIndex),
                                       filePathList,
                                       true,pTcpSocket);
    fdlgMap[ipv4ToString(pTcpSocket->peerAddress().toIPv4Address())] = pfdlg;
    // FilesDialog销毁时,对应移除 hostSocketMap数据
    connect(pfdlg,&FilesDialog::destroyed,
            this,&Widget::on_destroyFilesDialog);
    filePathList.clear();

}

void Widget::on_destroyFilesDialog(QObject *obj)
{
    if(obj==nullptr) return;
    FilesDialog* pfdg = (FilesDialog*)obj;
    QString ip = pfdg->ip;
    if(hostTcpSocketMap.contains(ip))
    {
//        hostTcpSocketMap[ip]->close(); // tcpserver不属于当前线程时会报错，发消息处理
        emit hostTcpSocketMap[ip]->disconnected();
//        hostSocketMap[ip]->deleteLater(); // 使用智能指针管理，无需手动销毁
        hostTcpSocketMap.remove(ip);
    }
    obj->deleteLater();
    fdlgMap.remove(ip);
    if(sendAckMsgBoxMap.contains(ip))
    {
        sendAckMsgBoxMap[ip]->deleteLater();
        sendAckMsgBoxMap.remove(ip);
    }
}

