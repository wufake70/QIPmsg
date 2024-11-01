#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDir>


MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    isSingleton();
    pMainThread = QThread::currentThread();
    connect(this,SIGNAL(switchTcpServerObjThread(QThread *)),
            this,SLOT(on_switchTcpServerObjThread(QThread *)));

    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowMaximizeButtonHint);
    port = QIPMSG_SERVER_PORT;
    ui->tableWidget->setItem(0,0,new QTableWidgetItem(tr("主机名"))); // table widget init
    ui->tableWidget->setItem(0,1,new QTableWidgetItem(tr("IP地址")));

    localIpList = getLocalHostIps();
    ui->label_2->setText(hostName);
    memberIpList.push_front("127.0.0.1");
    memberNameList.push_front(hostName);
    ui->textEdit->setObjectName(hostName);      // textEdit widget init
    ui->textEdit->setReadOnly(true);
    pteList.push_back(ui->textEdit);
    curShowPteIndex = 0;
    updateMembersTable();

    pUdpSocket= new QUdpSocket();

    connect(ui->tableWidget,
            SIGNAL(itemClicked(QTableWidgetItem *)),this,
            SLOT(clickTableWidgetItem(QTableWidgetItem *)));
    connect(pUdpSocket,SIGNAL(readyRead()),this,SLOT(UdpEvent()));
    startUdpSocket();
    sendUdpSocket(QIPMSG_UPD_PROBE); // 探测其他主机

    initTcpServer();
    connect(this,SIGNAL(sendMsgFileAddOrDel(QString,bool,QString)),
            this,SLOT(on_sendMsgFileAddOrDel(QString,bool,QString)));
    connect(this,SIGNAL(sendMsgFileAddOrDel(QString,bool,QStringList)),
            this,SLOT(on_sendMsgFileAddOrDel(QString,bool,QStringList)));
    connect(this,SIGNAL(sendMsgFileSendAck(FilesDialog*)),this,
            SLOT(on_sendMsgFileSendAck(FilesDialog*)));

    QTimer *ptimer = &timer;
    ptimer->setInterval(7000);
    connect(&timer,&QTimer::timeout,this,
            [this,ptimer]()
    {
        this->sendUdpSocket(QIPMSG_UPD_PROBE);
        this->pUdpSocket->flush();
        ptimer->start();
    });
    timer.start();
}

void MainWidget::isSingleton()
{
    singleton.setKey("QIPmsg");
    if(!singleton.attach())
    {
        singleton.create(1,QSharedMemory::ReadOnly);
    }else{
        singleton.detach(); // avoid memory loss
        exit(1);
    }
}

MainWidget::~MainWidget()
{
    delete ui;
}

QStringList MainWidget::getLocalHostIps()
{
    hostName = QHostInfo::localHostName();
    QStringList result;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    foreach(const QNetworkInterface &interface, interfaces)
    {
        if (!interface.flags().testFlag(QNetworkInterface::IsUp) ||
                interface.flags().testFlag(QNetworkInterface::IsLoopBack))
            continue;
        foreach(const QNetworkAddressEntry &entry, interface.addressEntries())
        {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) continue;
            result << entry.ip().toString();
        }
    }

    return result;

}

void MainWidget::startUdpSocket()
{
    if(pUdpSocket->bind(port))
    {
        ui->textEdit->append("本机"+QString::number(port)+
                             "端口绑定成功");
    }else{
        ui->textEdit->append("本机"+QString::number(port)+
                             "端口绑定失败");
    }

}
void MainWidget::stopUdpSocket()
{
    if(pUdpSocket!=nullptr) pUdpSocket->abort();
}

void MainWidget::on_pushButton_refresh_clicked()
{
    sendUdpSocket(1);
}

void MainWidget::on_pushButton_send_clicked()
{
    if(ui->textEdit_send->toPlainText().startsWith("/file"))
    {
        if(memberNameList.at(curShowPteIndex)!=hostName)
            sendUdpSocket(4);

    }else{
        sendUdpSocket(3);
    }
}

void MainWidget::clickTableWidgetItem(QTableWidgetItem *item)
{
    int index = item->row()-1;
    if(index<0||curShowPteIndex==index) return;
    oldShowPteIndex = curShowPteIndex;
    curShowPteIndex= index;
    updateTextEdit();
}

void MainWidget::on_switchTcpServerObjThread(QThread *target)
{
    shptrTcpServer.get()->moveToThread(target);
    emit ((ServerThread*)target)->sendFileTcpSocket();
}


// QIPMSG_UPD_EXIT
void MainWidget::closeEvent(QCloseEvent *event)
{
    singleton.detach(); // avoid memory loss(ubuntu)
    sendUdpSocket(QIPMSG_UPD_EXIT);
    pUdpSocket->flush();
    pUdpSocket->waitForBytesWritten();
    stopUdpSocket();
    event->accept();
    exit(0);
}


void MainWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void MainWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    filePathList.clear();

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
                filePathList << filePath;
            }
        }
    }

    // 通知tcp客户端
    if(memberNameList.at(curShowPteIndex)!=hostName)
        sendUdpSocket(4);

}

void MainWidget::on_pushButton_checkDir_clicked()
{
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    homePath += "/QIpmsg";

    // 检查路径是否存在，如果不存在则创建
    QDir dir(homePath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QUrl url = QUrl::fromLocalFile(homePath);
    QDesktopServices::openUrl(url);
}

void MainWidget::on_pushButton_help_about_clicked()
{
    static qint8 showflag = 0;
    if(showflag){
        return;
    }else {
        showflag =1;
    }
    QDialog *pMsgbox = new QDialog(this);
    pMsgbox->setModal(false);
    pMsgbox->setWindowTitle(tr("帮助/关于"));

    QVBoxLayout *pLayout = new QVBoxLayout(pMsgbox);

    // 创建 QLabel 对象
    QLabel *pLabel = new QLabel(tr("<h3>关于该软件</h3>"
                              "本程序基于Qt5实现，支持局域网下主机探测、即时消息(udp实现)、<br>"
                              "一对一发送文件(速率上线取决于当前局域网设备，tcp实现)、<br>"
                              "多对一发送文件(多线程+tcp实现)。"
                              "<h3>使用说明：</h3>"
                              "<ul>"
                              "<li>选中目标主机，在文本框键内容，最后Enter"
                              "(Shift+Enter换行)<br>即可发送消息。</li>"
                              "<li>选中主机，拖动文件到主程序即可弹出文件发送框，<br>"
                              "文件发送框支持拖动文件增加列表，双击列表框某一项即可删除，<br>"
                              "点击发送，等待对方确认即可。</li>"
                              "<li>注意设置防火墙允许本程序在专有网络(私有)下进行通信，<br>"
                              "可以手动设置防火墙入站规则。</li>"
                              "</ul>"
                              "<a href='https://github.com/wufake70/QIPmsg'>本项目GitHub仓库地址，给个star支持一下^v^</a>"));

    connect(pLabel, &QLabel::linkActivated, [](const QString& link)
    {
            QDesktopServices::openUrl(QUrl(link));
    });

    // 设置字体大小和样式
    QFont font("Adobe Devanagari", 12);  // 设置字体为 Arial, 大小为 12
    pLabel->setFont(font);

    // 设置文本对齐方式
    pLabel->setAlignment(Qt::AlignLeft);

    // 设置样式表
    pLabel->setStyleSheet("QLabel { color : black; }");
    pLayout->addWidget(pLabel);
    pMsgbox->setLayout(pLayout);
    connect(pMsgbox,&QMessageBox::finished,pMsgbox,
            [pMsgbox,pLabel,pLayout](int){
            pMsgbox->deleteLater();
            pLabel->deleteLater();
            pLayout->deleteLater();
            showflag = 0;
    });
    pMsgbox->show();

}
