#include "inputnum_widget.h"
#include "ui_inputnum_widget.h"
#include <QDebug>

InputNum_Widget::InputNum_Widget(QString inittext, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InputNum_Widget)
{
    ui->setupUi(this);
    setAttribute (Qt::WA_DeleteOnClose);
    waitingForOperand = false;
    //if (inittext == "")
     //   inittext = "0";
    ui->Input_LineEdit->setText(inittext);
    ui->Input_LineEdit->setReadOnly(true);
    ui->Input_LineEdit->setMaxLength(12);
    ui->Input_LineEdit->setAlignment(Qt::AlignRight);
    connect(ui->Button_0, SIGNAL(clicked()), this, SLOT(digitClicked()));
    connect(ui->Button_1, SIGNAL(clicked()), this, SLOT(digitClicked()));
    connect(ui->Button_2, SIGNAL(clicked()), this, SLOT(digitClicked()));
    connect(ui->Button_3, SIGNAL(clicked()), this, SLOT(digitClicked()));
    connect(ui->Button_4, SIGNAL(clicked()), this, SLOT(digitClicked()));
    connect(ui->Button_5, SIGNAL(clicked()), this, SLOT(digitClicked()));
    connect(ui->Button_6, SIGNAL(clicked()), this, SLOT(digitClicked()));
    connect(ui->Button_7, SIGNAL(clicked()), this, SLOT(digitClicked()));
    connect(ui->Button_8, SIGNAL(clicked()), this, SLOT(digitClicked()));
    connect(ui->Button_9, SIGNAL(clicked()), this, SLOT(digitClicked()));
}

InputNum_Widget::~InputNum_Widget()
{
    delete ui;
}

void InputNum_Widget::digitClicked()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    int digitValue = clickedButton->text().toInt();
    QString text("aplay /root/my_musics/piano/b");
    QString text_end(".wav &");

    if (ui->Input_LineEdit->text().length() == ui->Input_LineEdit->maxLength())
    {
        system("aplay /root/my_musics/error.wav &");
        //QSound::play("E:/sunward/A83T/qt_code/my_musics/error.wav");
        return;
    }

    if (digitValue == 0)
    {
        if (ui->Input_LineEdit->text() == "0" || (ui->Input_LineEdit->text() == ""))
        {
            //QSound::play("E:/sunward/A83T/qt_code/my_musics/piano/b10.wav");
            //system("aplay /root/my_musics/piano/b0.wav &");
            qDebug() << "0" << endl;
            waitingForOperand = true;
            return;
        }
    }

    text += QString::number(digitValue) + text_end;
    //text = "E:/sunward/A83T/qt_code/my_musics/piano/b";
    //text += QString::number(digitValue) + ".wav";
    QByteArray cpath = text.toLocal8Bit();
    char *str = cpath.data();
    qDebug() << str << endl;
    system(str);
    //QSound::play(str);

    if (waitingForOperand) {
        qDebug() << "clear" << endl;
        ui->Input_LineEdit->clear();
        waitingForOperand = false;
    }
    ui->Input_LineEdit->setText(ui->Input_LineEdit->text() + QString::number(digitValue));
}

void InputNum_Widget::on_Enter_Button_clicked()
{
    //QSound::play("E:/sunward/A83T/qt_code/my_musics/piano/b11.wav");
    system("aplay /root/my_musics/piano/b11.wav &");
    if (ui->Input_LineEdit->text() != "0")
        emit sendData(ui->Input_LineEdit->text());
    else
        emit sendData("");
    //this->accept();
    this->close();
}

void InputNum_Widget::on_Del_Button_clicked()
{
    QString text = ui->Input_LineEdit->text();

    text.chop(1);
    if (text.isEmpty()) {
        //text = "0";
        //QSound::play("E:/sunward/A83T/qt_code/my_musics/error.wav");
        system("aplay /root/my_musics/error.wav &");
        waitingForOperand = true;
        emit sendData("");
        //this->accept();
        this->close();
    }
    else
    {
        //QSound::play("E:/sunward/A83T/qt_code/my_musics/piano/b10.wav");
        system("aplay /root/my_musics/piano/b10.wav &");
    }
    ui->Input_LineEdit->setText(text);
}
