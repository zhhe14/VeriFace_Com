#include "crc16.h"
#include "hardware.h"
#include "mainwindow.h"
#include "inputnum_widget.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    hardware = new HARDWARE(this);
    connect(hardware, SIGNAL(sig_LightSensor_Data(QString)), this, SLOT(recvHardwareData(QString)));
    connect(hardware, SIGNAL(sig_MinShengCard_Data(QString)), this, SLOT(recvHardwareData(QString)));
    connect(hardware, SIGNAL(sig_IdentityCard_Data(QString)), this, SLOT(recvHardwareData(QString)));
    connect(hardware, SIGNAL(sig_IC_Data(QString)), this, SLOT(recvHardwareData(QString)));
    connect(ui->lineEdit, SIGNAL(clicked()), this, SLOT(HandleLineEditclicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::HandleLineEditclicked()
{
    qDebug() << "MainWindow::HandleLineEditclicked" << endl;
    QString text;

    InputNum_Widget * input = new InputNum_Widget(text, this);
    input->move((this->width()-input->width())/2, (this->height()-input->height())/2 + 20);
    connect(input, SIGNAL(sendData(QString)), this, SLOT(receiveData(QString)));
    input->show();
}

void MainWindow::receiveData(QString data)
{
    qDebug() << "recv:" << data << endl;
    qDebug() << "LineEdit recv:" << data << endl;
    ui->lineEdit->setText(data);
}

//void MainWindow::recvHardwareData(u_int8_t *recv_data, u_int32_t recv_len)
void MainWindow::recvHardwareData(QString str)
{
    qDebug() << "MainWindow recv Hardware data";
    ui->textBrowser->clear();
    ui->textBrowser->insertPlainText(str);
}

void MainWindow::on_SendPWM_Button_clicked()
{
    u_int8_t pwm = ui->lineEdit->text().toUShort();
    qDebug() << "on_SendPWM_Button_clicked pwm: " << pwm;

    this->hardware->setInfraredPWM(pwm);
    //emit sendAppHardwareData(send_data, send_len);
}

void MainWindow::on_OpenDoor_Button_clicked()
{
    this->hardware->setCtrlDoor(DOOR_OPEN);
    qDebug() << "on_OpenDoor_Button_clicked";
    //emit sendAppHardwareData(send_data, send_len);
}

void MainWindow::on_CloseDoor_Button_clicked()
{
    this->hardware->setCtrlDoor(DOOR_CLOSE);
    qDebug() << "on_CloseDoor_Button_clicked";
    //emit sendAppHardwareData(send_data, send_len);
}

void MainWindow::on_OpenLogoLed_Button_clicked()
{
    this->hardware->setLogoLed(LOGOLED_ON);
    qDebug() << "on_OpenLogoLed_Button_clicked";
    //emit sendAppHardwareData(send_data, send_len);
}

void MainWindow::on_CloseLogoLed_Button_clicked()
{
    this->hardware->setLogoLed(LOGOLED_OFF);
    qDebug() << "on_CloseLogoLed_Button_clicked";
    //emit sendAppHardwareData(send_data, send_len);
}

void MainWindow::on_ResetSTM32_Button_clicked()
{
   this->hardware->resetStm32();
    qDebug() << "on_ResetSTM32_Button_clicked";
}
