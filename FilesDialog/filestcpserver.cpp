#include "filestcpserver.h"
#include "FilesDialog/filesdialog.h"
#include "Widget/widget.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

void ServerThread::run()
{
    FilesTcpServer *fts = new FilesTcpServer(shptrServerTcpSocket,this);
    connect((FilesDialog*)parent(),SIGNAL(switchTcpServerThread()),
            fts,SLOT(on_switchTcpServerThread()));
    connect((FilesDialog*)parent(),SIGNAL(sendFileTcpSocket())
            ,fts,SLOT(on_sendFileTcpSocket()));
    connect(this,SIGNAL(sendFileTcpSocket()),fts,SLOT(on_sendFileTcpSocket()));
    exec(); // 进入事件循环，quit函数可以跳出事件循环并执行后面代码
//    emit closeFilesDialoged();

    delete fts;

}

void FilesTcpServer::connectClient()
{

    qDebug().noquote() << "[client connect]: "+shptrServerTcpSocket.get()->peerAddress().toString()+":"+
                          QString::number(shptrServerTcpSocket.get()->peerPort());
}
void FilesTcpServer::disconnectClient()
{
    qDebug().noquote() << "[client socket disconnect]";
    emit ((ServerThread*)parentThread)->closeFilesDialoged();
    parentThread->quit();
}
void FilesTcpServer::readSocketData()
{
//    qDebug().noquote() << "[in]: "+shptrServerTcpSocket.get()->readLine();

}
void FilesTcpServer::on_switchTcpServerThread()
{
    // 切换TcpServer的线程对象
    QThread *curThread = QThread::currentThread();
//    QThread *mainThread = Widget::instance->pMainThread;
    emit Widget::instance->switchTcpServerObjThread(curThread);
}
void FilesTcpServer::on_sendFileTcpSocket()
{
    qDebug().noquote() << "send file";
    QJsonObject jsonObj;
    QJsonDocument jsonDoc;

    QByteArray buffer;


    QFile *file = nullptr;
    qint64 bytesToWriten = 0;
    qint64 fileSize = 0;

    QStringList filePathList = ((FilesDialog *)parentThread->parent())->filePathList;
    foreach(auto fullPath,filePathList)
    {
        if(!QFile::exists(fullPath)) continue;
        file = new QFile(fullPath);
        if(!file->open(QIODevice::ReadOnly)) continue;
        bytesToWriten = 0;

        jsonObj["type"] = QIPMSG_TCP_FILE_BEGIN;
        jsonObj["file_name"] = QString(fullPath).replace(QRegExp(".*/"),"");
        fileSize = file->size();
        jsonObj["file_size"] = fileSize;
        jsonDoc.setObject(jsonObj);
        shptrServerTcpSocket->write(jsonDoc.toJson().toBase64()+"\n");
        shptrServerTcpSocket->flush();
        emit ((FilesDialog*)parentThread->parent())->
                curHandlingFileState(QIPMSG_TCP_FILE_BEGIN,fullPath,0);

        while (!file->atEnd())
        {
            buffer = file->read(1024);

            bytesToWriten +=buffer.size();
            jsonObj["type"] = QIPMSG_TCP_FILE_SCHEDULE;
            jsonObj["section_size"] = buffer.size();
            // json不支持传输二进制，转成Base64(就是字符串且没有\0)
            jsonObj["file_bin"] = ((QString)buffer.toBase64());

            jsonDoc.setObject(jsonObj);
            shptrServerTcpSocket->write(jsonDoc.toJson().toBase64()+"\n"); // 不会立即发送，会先填充缓冲区

            if(shptrServerTcpSocket->bytesToWrite()>=5*1024*1024) // 5Mb
            {
                shptrServerTcpSocket->flush(); // 不会立即执行、异步执行
                shptrServerTcpSocket->waitForBytesWritten(); // 阻塞、等待socket发送
                emit ((FilesDialog*)parentThread->
                        parent())->
                        curHandlingFileState(QIPMSG_TCP_FILE_SCHEDULE,fullPath,
                        ((float)bytesToWriten/fileSize)*100);
            }
        }
        file->close();

        jsonObj["type"] = QIPMSG_TCP_FILE_END;
        jsonDoc.setObject(jsonObj);
        shptrServerTcpSocket->write(jsonDoc.toJson().toBase64()+"\n");
        shptrServerTcpSocket->flush();
        shptrServerTcpSocket->waitForBytesWritten();
        emit ((FilesDialog*)parentThread->
                parent())->
                curHandlingFileState(QIPMSG_TCP_FILE_END,fullPath,
                ((float)bytesToWriten/fileSize)*100);
    }

    // tcpServer对象移到主线程
    Widget::instance->shptrTcpServer.get()->moveToThread(
                Widget::instance->pMainThread);
}

