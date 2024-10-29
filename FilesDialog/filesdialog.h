#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QWidget>
#include <QSharedMemory>
#include <QCloseEvent>
#include <QListWidgetItem>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDebug>
#include <QUrl>
#include <QSharedPointer>
#include <QTcpServer>
#include "filestcpserver.h"
#include "filestcpclient.h"


namespace Ui {
class filesDialog;
}

class FilesDialog : public QWidget
{
    Q_OBJECT
public:
    QString ip;
    QList<QString> filePathList;  // 文件队列
    QList<QString> handledFilePathList;
    void updateListWidget();
private:
    bool isServer;
    QString host;
    QSharedMemory *pShareMemory; // 保证单个实例
    QSharedPointer<QTcpServer*> pTcpServer_sptr;


    ServerThread *pServerThread;
    ClientThread *pClientThread;

    QSharedPointer<QTcpSocket> shptrServerTcpSocket;

signals:
    void startSendFiles();
    void switchTcpServerThread();
    void sendFileTcpSocket();
    void curHandlingFileState(int,QString,qint8=0);
//    void clientSendAckMsgBox();

private slots:
    void itemDoubleClick(QListWidgetItem *);
    void onCloseFilesDialog();
    void on_pushButton_2_clicked();
    void on_startSendFiles();
    void on_curHandlingFileState(int,QString,qint8=0);
//    void on_clientSendAckMsgBox();

public:
    bool singleton();
    void init(QString,QString,bool);
    FilesDialog(QString,QString,bool,
                QTcpSocket* pTcpServerSocket=nullptr,
                QWidget *parent=nullptr);
    FilesDialog(QString,QString,QString,bool,
                QTcpSocket*pTcpServerSocket=nullptr,
                QWidget *parent=nullptr);
    FilesDialog(QString,QString,QStringList,bool,
                QTcpSocket*pTcpServerSocket=nullptr,
                QWidget *parent=nullptr);

    ~FilesDialog();

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Ui::filesDialog *ui;
};

#endif // FILEDIALOG_H
