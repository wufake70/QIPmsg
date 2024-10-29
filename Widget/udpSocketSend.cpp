#include "widget.h"
#include "ui_widget.h"
#include <QJsonArray>

void Widget::sendUdpSocket(int type,QString targetIp)
{
    QJsonObject jsonObj;
    jsonObj["host_name"] = hostName;

    switch (type) {
    case 1:
        jsonObj["type"] = QIPMSG_UPD_PROBE;
        probeHost(jsonObj);
        break;
    case 2:
        jsonObj["type"] = QIPMSG_UPD_EXIT;
        probeHost(jsonObj);
        break;
    case 3:
        jsonObj["type"] = QIPMSG_UPD_TXT;
        sendMsg(jsonObj);
        break;
    case 4:
        jsonObj["type"] = QIPMSG_UPD_TCPSERVER;
        sendMsgBuildTcp(jsonObj);
        break;
    case 6:
        jsonObj["type"] = QIPMSG_UPD_FILE_ADD;
        sendMsgFileAdd(jsonObj);
        break;
    case 7:
        jsonObj["type"] = QIPMSG_UPD_FILE_DEL;
        sendMsgFileDel(jsonObj);
        break;
    case 8:
        jsonObj["type"] = QIPMSG_UPD_FILE_SENDACK;
        sendMsgFileSendAck(jsonObj,targetIp);
        break;
    case 9:
        jsonObj["type"] = QIPMSG_UPD_FILE_RECEIVEACK;
        sendMsgFileReceiveAck(jsonObj,targetIp);
        break;
    default:
        jsonObj["type"] = QIPMSG_UPD_NOP;
        probeHost(jsonObj);
    }
    pUdpSocket->flush(); // force flush udpsocket buffer(asyn)
    pUdpSocket->waitForBytesWritten(); // wait udpsocket flush

}

// udp探测其他主机
void Widget::probeHost(QJsonObject& jsonObj)
{
    QJsonDocument jsonDoc;
    QString broadcast;

    foreach(auto ip,localIpList)
    {
        broadcast = ip;
//        jsonObj["host_ip"] = ip;
        broadcast.replace(QRegExp("\\.\\d+$"),".255");
        jsonDoc.setObject(jsonObj);
        pUdpSocket->writeDatagram(jsonDoc.toJson(),QHostAddress(broadcast),port);
    }
}

void Widget::sendMsg(QJsonObject &jsonObj)
{
    QJsonDocument jsonDoc;
    if(!ui->textEdit_send->toPlainText().isEmpty()||
            jsonObj["type"].toInt()==QIPMSG_UPD_TCPSERVER){
        QJsonDocument jsonDoc;
        QString broadcast;
        QHostAddress addr(memberIpList.at(curShowPteIndex));
        jsonObj["txt_msg"]=ui->textEdit_send->toPlainText();
        jsonObj["time_stamp"]=QDateTime::currentSecsSinceEpoch();
        jsonDoc.setObject(jsonObj);
        pUdpSocket->writeDatagram(jsonDoc.toJson(),addr,port);
        updateTextEdit(tr("[%1 out] ").arg(
                           QDateTime::currentDateTime().toString("hh:mm:ss"))+
                       jsonObj["txt_msg"].toString());
    }
}
//
void Widget::sendMsgBuildTcp(QJsonObject &jsonObj)
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArr;
    if(jsonObj["type"].toInt()==QIPMSG_UPD_TCPSERVER){
        QJsonDocument jsonDoc;
        QString broadcast;
        QHostAddress addr(memberIpList.at(curShowPteIndex));
        jsonObj["txt_msg"]=ui->textEdit_send->toPlainText();
        jsonObj["time_stamp"]=QDateTime::currentSecsSinceEpoch();
        foreach(auto i,filePathList)
        {

            jsonArr.append(i.replace(QRegExp(".*/"),""));
        }
        jsonObj["file_list"] = jsonArr;
        jsonDoc.setObject(jsonObj);
        pUdpSocket->writeDatagram(jsonDoc.toJson(),addr,port);
    }
}

void Widget::sendMsgFileAdd(QJsonObject &jsonObj)
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArr;
    if(jsonObj["type"].toInt()==QIPMSG_UPD_FILE_ADD){
        QJsonDocument jsonDoc;
        QString broadcast;
        QHostAddress addr(memberIpList.at(curShowPteIndex));
        jsonObj["txt_msg"]=ui->textEdit_send->toPlainText();
        jsonObj["time_stamp"]=QDateTime::currentSecsSinceEpoch();
        //
        jsonObj["is_file_add"] = true;
        //
        foreach(auto file,filePathList)
        {
            jsonArr.append(file.replace(QRegExp(".*/"),""));
        }
        jsonObj["file_list"] = jsonArr;
        jsonDoc.setObject(jsonObj);
        pUdpSocket->writeDatagram(jsonDoc.toJson(),addr,port);
    }
}

void Widget::sendMsgFileDel(QJsonObject &jsonObj)
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArr;
    if(jsonObj["type"].toInt()==QIPMSG_UPD_FILE_DEL){
        QJsonDocument jsonDoc;
        QString broadcast;
        QHostAddress addr(memberIpList.at(curShowPteIndex));
        jsonObj["txt_msg"]=ui->textEdit_send->toPlainText();
        jsonObj["time_stamp"]=QDateTime::currentSecsSinceEpoch();
        //
        jsonObj["is_file_add"] = false;
        //
        foreach(auto file,filePathList)
        {
            jsonArr.append(file.replace(QRegExp(".*/"),""));
        }
        jsonObj["file_list"] = jsonArr;
        jsonDoc.setObject(jsonObj);
        pUdpSocket->writeDatagram(jsonDoc.toJson(),addr,port);
    }
}

void Widget::sendMsgFileSendAck(QJsonObject &jsonObj,QString targetIp)
{
    QJsonDocument jsonDoc;
    if(jsonObj["type"].toInt()==QIPMSG_UPD_FILE_SENDACK){
        QJsonDocument jsonDoc;
        QString broadcast;
        QHostAddress addr(targetIp);
        jsonObj["txt_msg"]=ui->textEdit_send->toPlainText();
        jsonObj["time_stamp"]=QDateTime::currentSecsSinceEpoch();
        //
        jsonObj["send_ack"] = true;
        //
        jsonDoc.setObject(jsonObj);
        pUdpSocket->writeDatagram(jsonDoc.toJson(),addr,port);
    }
}
void Widget::sendMsgFileReceiveAck(QJsonObject &jsonObj,QString tardgetIp)
{
    QJsonDocument jsonDoc;
    if(jsonObj["type"].toInt()==QIPMSG_UPD_FILE_RECEIVEACK){
        QJsonDocument jsonDoc;
        QString broadcast;
        QHostAddress addr(tardgetIp);
        jsonObj["txt_msg"]=ui->textEdit_send->toPlainText();
        jsonObj["time_stamp"]=QDateTime::currentSecsSinceEpoch();
        //
        jsonObj["receive_ack"] = true;
        //
        jsonDoc.setObject(jsonObj);
        pUdpSocket->writeDatagram(jsonDoc.toJson(),addr,port);
    }
}

void Widget::on_sendMsgFileAddOrDel(QString ip,bool isAdd,QString file)
{
    filePathList << file;
    oldShowPteIndex = curShowPteIndex;
    curShowPteIndex = memberIpList.indexOf(ip);
    updateTextEdit();
    if(isAdd)
    {
        sendUdpSocket(QIPMSG_UPD_FILE_ADD);
    }else{
        sendUdpSocket(QIPMSG_UPD_FILE_DEL);
    }
    filePathList.clear();
}

void Widget::on_sendMsgFileAddOrDel(QString ip,bool isAdd,QStringList fileList)
{
    filePathList = fileList;
    oldShowPteIndex = curShowPteIndex;
    curShowPteIndex = memberIpList.indexOf(ip);
    updateTextEdit();
    if(isAdd)
    {
        sendUdpSocket(QIPMSG_UPD_FILE_ADD);
    }else{
        sendUdpSocket(QIPMSG_UPD_FILE_DEL);
    }
    filePathList.clear();
}

void Widget::on_sendMsgFileSendAck(FilesDialog *pfdlg)
{
    sendUdpSocket(QIPMSG_UPD_FILE_SENDACK,pfdlg->ip);
}
