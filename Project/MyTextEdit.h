#ifndef MYTEXTEDIT_H
#define MYTEXTEDIT_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QPushButton>
#include <QEvent>
#include <QDebug>

class MyTextEdit: public QTextEdit
{
    Q_OBJECT

public:
    MyTextEdit(QWidget *parent = nullptr)
    {
        setParent(parent);
    }
    ~MyTextEdit()
    {

    }
protected:
    void keyPressEvent(QKeyEvent *event)
    {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Return && !keyEvent->modifiers()) {
//                qDebug() << "Enter key will be pressed";
                ((QWidget*)parent())->findChild<QPushButton*>("pushButton_send")->click();
                return ;
            } else if (keyEvent->key() == Qt::Key_Return && keyEvent->modifiers() == Qt::ShiftModifier) {
//                qDebug() << "Shift+Enter key will be pressed";
                QTextEdit::keyPressEvent(new QKeyEvent(QEvent::KeyPress,
                                                       Qt::Key_Return,
                                                       Qt::NoModifier));
                return ;
            }
        }
        QTextEdit::keyPressEvent(event);
    }

};

#endif // MYTEXTEDIT_H
