#include "mylineedit.h"
#include <QDebug>

MyLineEdit::MyLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    setFocusPolicy(Qt::ClickFocus);
}

void MyLineEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        qDebug() << "MyLineEdit::mousePressEvent" << endl;
        emit clicked();
    }

    QLineEdit::mousePressEvent(event);
}

#if 0
void MyLineEdit::focusInEvent(QFocusEvent * event)
{
    qDebug() << "MyLineEdit::focusInEvent" << endl;
    //emit clicked();
    //this->clearFocus();
    QLineEdit::focusInEvent(event);
}

void MyLineEdit::focusOutEvent(QFocusEvent * event)
{
    qDebug() << "MyLineEdit::focusOutEvent" << endl;
    //this->clearFocus();
    QLineEdit::focusOutEvent(event);
}
#endif

