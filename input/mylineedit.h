#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QLineEdit>
#include <QMouseEvent>
#include <QWidget>

class MyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEdit(QWidget *parent = 0);

protected:
    virtual void mousePressEvent(QMouseEvent * event);
#if 0
    virtual void focusInEvent(QFocusEvent * event);
    virtual void focusOutEvent(QFocusEvent * event);
#endif

signals:
    void clicked();

public slots:

};

#endif // MYLINEEDIT_H
