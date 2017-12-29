#ifndef INPUT_WIDGET_H
#define INPUT_WIDGET_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class InputNum_Widget;
}

class InputNum_Widget : public QWidget
{
    Q_OBJECT

public:
    explicit InputNum_Widget(QString inittext = "0", QWidget *parent = 0);
    ~InputNum_Widget();

signals:
    void sendData(QString);

private slots:
    void digitClicked();

    void on_Enter_Button_clicked();

    void on_Del_Button_clicked();

private:
    Ui::InputNum_Widget *ui;
    bool waitingForOperand;
};

#endif // INPUT_WIDGET_H
