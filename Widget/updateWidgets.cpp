#include "widget.h"
#include "ui_widget.h"



void Widget::updateMembersTable()
{
    ui->tableWidget->clear();
    ui->tableWidget->setItem(0,0,new QTableWidgetItem("主机名"));
    ui->tableWidget->setItem(0,1,new QTableWidgetItem("IP地址"));
    for(int i=0;i<memberNameList.size();i++)
    {
        ui->tableWidget->setItem(i+1,0,new QTableWidgetItem(memberNameList.at(i)));
        ui->tableWidget->setItem(i+1,1,new QTableWidgetItem(memberIpList.at(i)));
    }
    ui->label->setText("Member: "+QString::number(memberIpList.size()));
}


void Widget::updateTextEdit()
{
    if(oldShowPteIndex<pteList.size() && pteList.size()>curShowPteIndex)
    {
        pteList.at(oldShowPteIndex)->hide();
        pteList.at(curShowPteIndex)->show();
        ui->label_2->setText(memberNameList.at(curShowPteIndex));
    }
}

void Widget::updateTextEdit(QString content)
{
    pteList.at(curShowPteIndex)->append(content);
}
void Widget::updateTextEdit(int index,QString content)
{
    oldShowPteIndex = curShowPteIndex;
    curShowPteIndex = index;
    updateTextEdit();
    updateTextEdit(content);
}

