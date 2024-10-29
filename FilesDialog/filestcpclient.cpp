#include "filestcpclient.h"
#include <QCoreApplication>
#include <QEvent>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include "./Widget/Qipmsg.h"
#include "filesdialog.h"
#include <QStandardPaths>
#include <QJsonArray>
#include <QDir>
#include <QNetworkProxy>
#include <QThreadStorage>

QThreadStorage<QString> tlsHomePath;
QThreadStorage<QFile*> tlsPFile;
QThreadStorage<qint64> tlsBytesToWriten;
QThreadStorage<qint64> tlsFileSize;

void ClientThread::run()
{

    FilesTcpClient *ptc = new FilesTcpClient(ip,this);

    // 连接信号和槽
    connect(ptc->pTcpSocket, SIGNAL(connected()), ptc, SLOT(connectServer()));
    connect(ptc->pTcpSocket, SIGNAL(disconnected()), ptc, SLOT(disconnectServer()));
    connect(ptc->pTcpSocket, SIGNAL(readyRead()), ptc, SLOT(readSocketData()));
    ptc->pTcpSocket->setProxy(QNetworkProxy::NoProxy);
    ptc->pTcpSocket->connectToHost(QHostAddress(ip),4455);
    exec(); // 开始事件循环
    delete ptc;

}

FilesTcpClient::FilesTcpClient(QString serverIpArg,QObject *parent)
{
    parentThread = (QThread*)parent;
    serverIp=serverIpArg;
    pTcpSocket = new QTcpSocket();
}


void FilesTcpClient::connectServer()
{
    qDebug().noquote() << "[server address]: "+
                          pTcpSocket->peerAddress().toString()+":"+
                          QString::number(pTcpSocket->peerPort());

}
void FilesTcpClient::disconnectServer()
{
    qDebug().noquote() << "[disconnect server]";
    if(parentThread->isRunning()) parentThread->quit();
    emit ((ClientThread*)parentThread)->closeFilesDialoged();

}
void FilesTcpClient::readSocketData()
{
//    qDebug().noquote() << "[in]: "+pTcpSocket->readLine();
    QJsonDocument jsonDoc;
    QJsonObject jsonObj;
    while (pTcpSocket->canReadLine()) { //tcp粘包，判断下一行是否存在

        jsonDoc = QJsonDocument::fromJson(QByteArray::fromBase64(pTcpSocket->readLine()));
        jsonObj = jsonDoc.object();
        QString homePath;
        QDir homeDir;
        QFile *pFile;

        qint64 bytesToWriten=0;
        qint64 fileSize = 0;
        switch (jsonObj["type"].toInt()) {
        case QIPMSG_TCP_FILE_BEGIN:
            tlsBytesToWriten.setLocalData(0);
            tlsFileSize.setLocalData((qint64)jsonObj["file_size"].toDouble());

            tlsHomePath.setLocalData(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/QIpmsg");
            homePath = tlsHomePath.localData();
            homeDir.setPath(homePath);

            if(!homeDir.exists(homePath)) homeDir.mkpath(homePath);
            pFile = new QFile(homePath+"/"+jsonObj["file_name"].toString());
            tlsPFile.setLocalData(pFile);
            // 不存在创建/存在覆写
            if(!pFile->open(QIODevice::WriteOnly)) return; // 删除套接字

            emit ((FilesDialog*)parentThread->parent())->
                    curHandlingFileState(QIPMSG_TCP_FILE_BEGIN,
                    jsonObj["file_name"].toString());
            break;
        case QIPMSG_TCP_FILE_SCHEDULE:
            pFile = tlsPFile.localData();
            if(pFile==nullptr) return;
            fileSize = (qint64)jsonObj["file_size"].toDouble();
            pFile->write(QByteArray::fromBase64(jsonObj["file_bin"].toString().toUtf8()));
            bytesToWriten = tlsBytesToWriten.localData();
            bytesToWriten += jsonObj["section_size"].toInt();
            tlsBytesToWriten.setLocalData(bytesToWriten);
            if(bytesToWriten %(5*1024*1024) == 0)
            {
                pFile->flush(); // 将缓冲区数据写入 文件
                pFile->waitForBytesWritten(2000); // 等待 flush执行
                emit ((FilesDialog*)parentThread->parent())->
                        curHandlingFileState(QIPMSG_TCP_FILE_SCHEDULE,
                        jsonObj["file_name"].toString(),
                        ((float)bytesToWriten/fileSize)*100);
            }
            break;

        case QIPMSG_TCP_FILE_END:
            pFile = tlsPFile.localData();
            if(pFile==nullptr) return;
            emit ((FilesDialog*)parentThread->parent())->
                    curHandlingFileState(QIPMSG_TCP_FILE_END,
                    jsonObj["file_name"].toString(),100);

            bytesToWriten = 0;
            tlsBytesToWriten.setLocalData(0);
            fileSize = 0;
            tlsFileSize.setLocalData(0);
            pFile->close();
            pFile->deleteLater();
            tlsPFile.setLocalData(nullptr);
            break;
        }
    }

}

