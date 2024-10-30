﻿#include "filesdialog.h"
#include "ui_filesdialog.h"
#include "../Widget/widget.h"
#include <QToolTip>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QFileDialog>

void FilesDialog::init(QString ip_arg,
                       QString host_arg,
                       bool mode_arg)
{
    ui->setupUi(this);
    pServerThread = nullptr;
    pClientThread = nullptr;
    isServer = mode_arg;
    host = host_arg;
    ip = ip_arg;
    pShareMemory= new QSharedMemory();
    ui->listWidget->setDragEnabled(false);

    connect(this,&FilesDialog::curHandlingFileState,
            this,&FilesDialog::on_curHandlingFileState);
    if(singleton()){ // 单例模式
        if(isServer)
        {
            setWindowTitle("发送到 "+host_arg);
            ui->label->setText("发送到 "+ip_arg);
            ui->listWidget->setToolTip("支持拖动文件操作;\n双击指定列表项即可删除;");
            connect(ui->listWidget,
                    SIGNAL(itemDoubleClicked(QListWidgetItem *)),
                    this,SLOT(itemDoubleClick(QListWidgetItem *)));
            pServerThread = new ServerThread(shptrServerTcpSocket,this);
            pServerThread->start();
            connect(pServerThread,SIGNAL(closeFilesDialoged()),
                    this,SLOT(onCloseFilesDialog()));
            connect(this,SIGNAL(startSendFiles()),
                    this,SLOT(on_startSendFiles()));
        }else {
            setWindowTitle("来自于 "+host_arg);
            ui->label->setText("来自于 "+ip_arg);
            ui->pushButton_selectfile->setEnabled(false);
            ui->pushButton_selectfile->hide();
            ui->pushButton_sendfile->setText("查看目录");
            ui->listWidget->setToolTip("");
            connect(ui->pushButton_sendfile,&QPushButton::clicked,
                    ui->pushButton_sendfile,[](){

                QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
                homePath += "/QIpmsg";
                QUrl url = QUrl::fromLocalFile(homePath);
                QDesktopServices::openUrl(url);
            });
            ui->pushButton_sendfile->disconnect(SIGNAL(clicked()), this, SLOT(on_pushButton_sendfile_clicked()));

            ui->pushButton_sendfile->setEnabled(true);
            setAcceptDrops(false);
            pClientThread = new ClientThread(ip,this);
            pClientThread->start();
            connect(pClientThread,SIGNAL(closeFilesDialoged()),
                    this,SLOT(onCloseFilesDialog()));
        }
        show();
        updateListWidget();
    }else {
        deleteLater();
    }
}

FilesDialog::FilesDialog(QString ip_arg,
                         QString host_arg,
                         bool mode_arg,
                         QTcpSocket *pTcpSocket,
                         QWidget *parent)
    :QWidget(parent),
     ui(new Ui::filesDialog)
{
    QSharedPointer<QTcpSocket> t(pTcpSocket);
    shptrServerTcpSocket = t;
    init(ip_arg,host_arg,mode_arg);
}

FilesDialog::FilesDialog(QString ip_arg,
                         QString host_arg,
                         QString fileName,
                         bool mode_arg,
                         QTcpSocket *pTcpSocket,
                         QWidget *parent)
    :QWidget(parent),
     ui(new Ui::filesDialog)
{
    QSharedPointer<QTcpSocket> t(pTcpSocket);
    shptrServerTcpSocket = t;
    filePathList << fileName;
    init(ip_arg,host_arg,mode_arg);

}

FilesDialog::FilesDialog(QString ip_arg,
                         QString host_arg,
                         QStringList fileNameList,
                         bool mode_arg,
                         QTcpSocket *pTcpSocket,
                         QWidget *parent)
    :QWidget(parent),
     ui(new Ui::filesDialog)
{
    QSharedPointer<QTcpSocket> t(pTcpSocket);
    shptrServerTcpSocket = t;
    filePathList = fileNameList;
    init(ip_arg,host_arg,mode_arg);
}

void FilesDialog::itemDoubleClick(QListWidgetItem *item)
{
    filePathList.removeOne(item->text());
    emit MainWidget::instance->sendMsgFileAddOrDel(ip,false,filePathList);
    updateListWidget();
}

void FilesDialog::onCloseFilesDialog()
{
    close();
}

FilesDialog::~FilesDialog()
{
    delete ui;
}

void FilesDialog::updateListWidget()
{
    ui->listWidget->clear();
    QListWidgetItem *plwdtit = nullptr;
    foreach(auto fpath,filePathList)
    {
        if((!isServer)||QFile::exists(fpath))
        {
            plwdtit = new QListWidgetItem(fpath);
            QPixmap pix(":/new/prefix1/file.png");
            plwdtit->setIcon(pix);
            ui->listWidget->addItem(plwdtit);
        }
    }
}

void FilesDialog::closeEvent(QCloseEvent *event)
{
    if(!isServer&&pClientThread->isReceiving)
    {
        QMessageBox *pMsgBox = new QMessageBox(this);
        pMsgBox->setWindowTitle(this->windowTitle());
        pMsgBox->setText("正在接收文件\n无法进行相关操作");
        pMsgBox->setModal(false);
        pMsgBox->show();
        connect(pMsgBox,&QObject::destroyed,
                this, [pMsgBox](){

            pMsgBox->deleteLater();
        });
        event->ignore();
        return;
    }
    // tcpserver 不在主线程内，需要特殊处理
    if(shptrServerTcpSocket&&shptrServerTcpSocket->thread()!=thread())
    {
        QMessageBox *pMsgBox = new QMessageBox(this);
        pMsgBox->setWindowTitle(this->windowTitle());
        pMsgBox->setText("文件正在发送\n无法进行相关操作");
        pMsgBox->setModal(false);
        pMsgBox->show();
        connect(pMsgBox,&QObject::destroyed,
                this, [pMsgBox](){

            pMsgBox->deleteLater();
        });
        event->ignore();
        return;
    }
    if(isServer)
    {
        if(pServerThread&&pServerThread->isRunning()){
            pServerThread->quit();
            pServerThread->wait();
            pServerThread->deleteLater();
            pServerThread=nullptr;

        }
        emit destroyed(this);
    }else {
        if(pClientThread&&pClientThread->isRunning()){
            pClientThread->quit();
            pClientThread->wait();
            pClientThread->deleteLater();
            pClientThread = nullptr;
        }
            emit destroyed(this);
    }

    if(pShareMemory!=nullptr) {
      delete pShareMemory;
        pShareMemory=nullptr;
    }
    event->accept();
    QWidget::closeEvent(event);
}

void FilesDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void FilesDialog::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList)
        {
            QString filePath = url.toLocalFile();
            QFileInfo fileInfo(filePath);
            if (fileInfo.isFile())
            {
//                qDebug() << "Dropped file path: " << filePath;
                // 在这里可以处理文件路径
                if(!filePathList.contains(filePath))
                    filePathList << filePath;
            }
        }
    }
    updateListWidget();
    emit MainWidget::instance->sendMsgFileAddOrDel(ip,true,filePathList);
}

bool FilesDialog::singleton()
{
    QString temp = ip;

    pShareMemory->setKey(temp);
    if(!pShareMemory->attach())
    {
        pShareMemory->create(1,QSharedMemory::ReadOnly);
        return true;
    }
    delete pShareMemory; // 存在引用计数器
    return false;
}

void FilesDialog::on_pushButton_sendfile_clicked()
{
    if(filePathList.size()<=0)
    {
        ui->pushButton_sendfile->setText("发送");
        ui->pushButton_sendfile->setToolTip("");
        ui->pushButton_sendfile->setEnabled(true);
        QMessageBox *pMsgBox = new QMessageBox(this);
        pMsgBox->setWindowTitle(this->windowTitle());
        pMsgBox->setText("当前文件传输队列为空。");
        pMsgBox->setModal(false);
        pMsgBox->show();
        connect(pMsgBox,&QObject::destroyed,
                this, [pMsgBox](){

            pMsgBox->deleteLater();
        });
        return;
    }
    if(thread()!=MainWidget::instance->shptrTcpServer->thread())
    {
        QMessageBox *pMsgBox = new QMessageBox(this);
        pMsgBox->setWindowTitle(this->windowTitle());
        pMsgBox->setText("当前已有文件在发送\n无法进行相关操作");
        pMsgBox->setModal(true);
        pMsgBox->show();
        connect(pMsgBox,&QObject::destroyed,
                this, [pMsgBox](){

            pMsgBox->deleteLater();
        });
        return;
    }

    emit MainWidget::instance->sendMsgFileSendAck(this);
    ui->pushButton_sendfile->setText("等待中...");
    ui->pushButton_sendfile->setToolTip("等待对方确认中...");
    ui->pushButton_sendfile->setToolTipDuration(3000);
    ui->pushButton_sendfile->setEnabled(false);
    ui->pushButton_sendfile->toolTip();
    QToolTip::showText(ui->pushButton_sendfile->mapToGlobal(QPoint(0,0)),
                       "等待对方确认中...",this,this->rect(),4500);
}

void FilesDialog::on_startSendFiles()
{
    if(filePathList.size()<=0)
    {
        ui->pushButton_sendfile->setText("发送");
        ui->pushButton_sendfile->setToolTip("");
        ui->pushButton_sendfile->setEnabled(true);
        QMessageBox *pMsgBox = new QMessageBox(this);
        pMsgBox->setWindowTitle(this->windowTitle());
        pMsgBox->setText("当前文件传输队列为空。");
        pMsgBox->setModal(false);
        pMsgBox->show();
        connect(pMsgBox,&QObject::destroyed,
                this, [pMsgBox](){

            pMsgBox->deleteLater();
        });
        return;
    }
    if(thread()!=MainWidget::instance->shptrTcpServer->thread())
    {
        ui->pushButton_sendfile->setText("发送");
        ui->pushButton_sendfile->setToolTip("");
        ui->pushButton_sendfile->setEnabled(true);
        QMessageBox *pMsgBox = new QMessageBox(this);
        pMsgBox->setWindowTitle(this->windowTitle());
        pMsgBox->setText("当前已有文件在发送\n无法进行相关操作");
        pMsgBox->setModal(false);
        pMsgBox->show();
        connect(pMsgBox,&QObject::destroyed,
                this, [pMsgBox](){

            pMsgBox->deleteLater();
        });
        return;
    }
    ui->pushButton_sendfile->setText("发送中...");
    ui->pushButton_sendfile->setToolTip("");
    ui->pushButton_sendfile->setEnabled(false);
    emit switchTcpServerThread();
}

void FilesDialog::on_curHandlingFileState(int type,QString fullPath,qint8 rate)
{
    QString fileName = QString(fullPath).replace(QRegExp(".*/"),"");

    switch(type)
    {
    case QIPMSG_TCP_FILE_BEGIN:
        ui->label_processbar->setText(fileName+" :");
        ui->label_processbar->setToolTip(fileName);
        ui->progressBar->setValue(0);

        ui->listWidget->setCurrentItem(
                    ui->listWidget->findItems(fullPath,Qt::MatchExactly)[0]);
        if(!isServer) {
            this->pClientThread->isReceiving = true;
            return;
        }
        disconnect(ui->listWidget,SIGNAL(itemDoubleClicked(QListWidgetItem *)),
                   this,SLOT(itemDoubleClick(QListWidgetItem *)));
        ui->listWidget->setToolTip("");
        break;
    case QIPMSG_TCP_FILE_SCHEDULE:
        ui->progressBar->setValue(rate);
        break;
    case QIPMSG_TCP_FILE_END:
        ui->progressBar->setValue(100);

        if(!isServer) {
            this->pClientThread->isReceiving = false;
            return;
        }
        ui->pushButton_sendfile->setText("发送");
        ui->pushButton_sendfile->setToolTip("");
        ui->pushButton_sendfile->setEnabled(true);
        ui->label_processbar->setToolTip("");
        ui->listWidget->setToolTip("双击即可删除指定项");
        connect(ui->listWidget,SIGNAL(itemDoubleClicked(QListWidgetItem *)),
                   this,SLOT(itemDoubleClick(QListWidgetItem *)));
        break;

    }
}


void FilesDialog::on_pushButton_selectfile_clicked()
{
    // 创建一个文件对话框
     QFileDialog *pDlg = new QFileDialog(this);
     pDlg->setFileMode(QFileDialog::ExistingFiles); // 设置对话框可以选择多个文件
     pDlg->setAcceptMode(QFileDialog::AcceptOpen); // 设置对话框为打开文件模式

     pDlg->setModal(false); // 非阻塞
     ui->pushButton_selectfile->setEnabled(false);
     ui->pushButton_sendfile->setEnabled(false);
     // 显示文件对话框
     pDlg->show();

     QStringList filePathList;
     connect(pDlg,&QFileDialog::finished,
             pDlg,[pDlg,this](int result){
        if(result==QDialog::Accepted)
         {
            foreach(auto path,pDlg->selectedFiles())
            {
                if(!this->filePathList.contains(path))
                    this->filePathList.append(path);
            }
            this->updateListWidget();
            emit MainWidget::instance->sendMsgFileAddOrDel(this->ip,
                                                           true,
                                                           this->filePathList);
        }

         ui->pushButton_selectfile->setEnabled(true);
         ui->pushButton_sendfile->setEnabled(true);
         pDlg->deleteLater();
     });

}


