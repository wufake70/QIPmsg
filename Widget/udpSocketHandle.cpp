#include "widget.h"
#include "ui_widget.h"
#include <QJsonArray>
#include <QMessageBox>
#include <QTimer>


void Widget::UdpEvent()
{
    while(pUdpSocket->hasPendingDatagrams())
    {
        QHostAddress address;
        QString tempIpStr;
        quint32 tempInt;
        quint64 time;
        QByteArray data;
        QJsonArray jsonArr;
        FilesDialog *pfdlg;
        QMessageBox *pMsgBox;

        data.resize(pUdpSocket->pendingDatagramSize());
        pUdpSocket->readDatagram(data.data(),data.size(),&address);
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        QJsonObject jsonObj=jsonDoc.object();

        tempInt = address.toIPv4Address();
        tempIpStr = ipv4ToString(tempInt);
        time = (qint64)jsonObj["time_stamp"].toDouble();
        switch(jsonObj["type"].toInt())
        {
            case QIPMSG_UPD_PROBE:
            if(localIpList.contains(tempIpStr)||
                    memberIpList.contains(tempIpStr))
                break;
            // 新成员
            pteList.push_back(new QTextEdit(this));
            pteList.back()->setReadOnly(true);
            pteList.back()->setMinimumSize(QSize(0,150));
            pteList.back()->setObjectName(jsonObj["host_name"].toString());
            ui->verticalLayout_5->insertWidget(1,pteList.back());
            pteList.back()->hide();
            memberIpList.push_back(tempIpStr);
            memberNameList.push_back(jsonObj["host_name"].toString());
            updateMembersTable();
            sendUdpSocket(1); // 反馈
            break;

        case QIPMSG_UPD_EXIT:
            if(memberIpList.contains(tempIpStr))
            {
                pteList.at(memberIpList.indexOf(tempIpStr))->deleteLater();
                if(curShowPteIndex==memberIpList.indexOf(tempIpStr))
                {
                    curShowPteIndex=0;
                    oldShowPteIndex=0;
                }
                pteList.removeAt(memberIpList.indexOf(tempIpStr));
                memberIpList.removeOne(tempIpStr);
                memberNameList.removeOne(jsonObj["host_name"].toString());
                updateTextEdit();

            }
            updateMembersTable();
            break;

        case QIPMSG_UPD_TXT:
            updateTextEdit(memberIpList.indexOf(tempIpStr),
                           tr("[%1 in ] ").arg(
                               QDateTime::fromSecsSinceEpoch(time).toString("hh:mm:ss"))+
                           jsonObj["txt_msg"].toString());
            break;

        case QIPMSG_UPD_TCPSERVER:
            jsonArr= jsonObj["file_list"].toArray();
            foreach(auto j,jsonArr)
            {
                filePathList << j.toString();
            }
            pfdlg = new FilesDialog(tempIpStr,
                                   jsonObj["host_name"].toString(),
                                   filePathList,false);
            connect(pfdlg,SIGNAL(destroyed(QObject*)),
                    this,SLOT(on_destroyFilesDialog(QObject*)));
            fdlgMap[tempIpStr] = pfdlg;
            filePathList.clear();
            break;

        case QIPMSG_UPD_FILE_ADD:
            pfdlg = fdlgMap[tempIpStr];
            pfdlg->filePathList.clear();
            jsonArr= jsonObj["file_list"].toArray();
            foreach(auto file,jsonArr)
            {
                pfdlg->filePathList << file.toString();
            }
            pfdlg->updateListWidget();
            break;

        case QIPMSG_UPD_FILE_DEL:
            pfdlg = fdlgMap[tempIpStr];
            pfdlg->filePathList.clear();
            jsonArr= jsonObj["file_list"].toArray();
            foreach(auto file,jsonArr)
            {
                pfdlg->filePathList << file.toString();
            }
            pfdlg->updateListWidget();
            break;
        case QIPMSG_UPD_FILE_SENDACK:
            pfdlg = fdlgMap[tempIpStr];
            if(!sendAckMsgBoxMap.contains(tempIpStr)) sendAckMsgBoxMap[tempIpStr] = nullptr;
            if(sendAckMsgBoxMap[tempIpStr]==nullptr)
            {
                pMsgBox = new QMessageBox(pfdlg);
                sendAckMsgBoxMap[tempIpStr] = pMsgBox;
                pMsgBox->setWindowTitle("消息确认");
                pMsgBox->setText(u8"是否接收文件?");
                pMsgBox->setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
                pMsgBox->setModal(false); // 非阻塞
                pMsgBox->show();
                connect(pMsgBox,&QMessageBox::finished,this,
                        [this,tempIpStr,jsonObj](int result){

                    if(result==QMessageBox::Ok)
                    {
                        oldShowPteIndex = curShowPteIndex;
                        curShowPteIndex = memberIpList.indexOf(tempIpStr);
                        if(curShowPteIndex<0)
                        {
                            pteList.push_back(new QTextEdit(this));
                            pteList.back()->setReadOnly(true);
                            pteList.back()->setObjectName(jsonObj["host_name"].toString());
                            ui->verticalLayout_5->insertWidget(1,pteList.back());
                            pteList.back()->hide();
                            memberIpList.push_back(tempIpStr);
                            memberNameList.push_back(jsonObj["host_name"].toString());
                            curShowPteIndex = memberIpList.size()-1;
                            updateMembersTable();
                        }
                        updateTextEdit();
                        sendUdpSocket(QIPMSG_UPD_FILE_RECEIVEACK,tempIpStr);
                    }
                    this->sendAckMsgBoxMap[tempIpStr]->deleteLater();
                    this->sendAckMsgBoxMap[tempIpStr] = nullptr;

                });
            }

            break;
        case QIPMSG_UPD_FILE_RECEIVEACK:
            //TODO 开始发送文件
//            qDebug() << "";
            pfdlg = fdlgMap[tempIpStr];

            emit pfdlg->startSendFiles();
            break;
        }

        pUdpSocket->flush(); // force flush udpsocket buffer(asyn)
        pUdpSocket->waitForBytesWritten(); // wait udpsocket flush
    }
}

