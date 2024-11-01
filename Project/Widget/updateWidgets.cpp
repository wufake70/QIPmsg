#include "widget.h"
#include "ui_widget.h"



void MainWidget::updateMembersTable()
{
    ui->tableWidget->clear();
    ui->tableWidget->setItem(0,0,new QTableWidgetItem(tr("主机名")));
    ui->tableWidget->setItem(0,1,new QTableWidgetItem(tr("IP地址")));
    for(int i=0;i<memberNameList.size();i++)
    {
        ui->tableWidget->setItem(i+1,0,new QTableWidgetItem(memberNameList.at(i)));
        ui->tableWidget->setItem(i+1,1,new QTableWidgetItem(memberIpList.at(i)));
    }
    ui->label->setText("Member: "+QString::number(memberIpList.size()));
}


void MainWidget::updateTextEdit()
{
    if(0<=oldShowPteIndex&&oldShowPteIndex<pteList.size()&&
            0<=curShowPteIndex&&curShowPteIndex<pteList.size())
    {
        pteList.at(oldShowPteIndex)->hide();
        pteList.at(curShowPteIndex)->show();
        ui->label_2->setText(memberNameList.at(curShowPteIndex));
    }
}

void MainWidget::updateTextEdit(QString content)
{
    pteList.at(curShowPteIndex)->append(content);
}
void MainWidget::updateTextEdit(int index,QString content)
{
    oldShowPteIndex = curShowPteIndex;
    curShowPteIndex = index;
    updateTextEdit();
    updateTextEdit(content);
}

