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
    ui->Icon0_Label->hide();
    ui->Icon1_Label->hide();
    ui->Icon2_Label->hide();
    ui->Icon3_Label->hide();

    connect(hardware, SIGNAL(sig_LightSensor_Data(QString)), this, SLOT(slot_HardwareData(QString)));
    connect(hardware, SIGNAL(sig_MinShengCard_Data(QString)), this, SLOT(slot_HardwareData(QString)));
    connect(hardware, SIGNAL(sig_IdentityCard_Data(QString)), this, SLOT(slot_HardwareData(QString)));
    connect(hardware, SIGNAL(sig_IC_Data(QString)), this, SLOT(slot_HardwareData(QString)));

    connect(hardware, SIGNAL(sig_stm32IsAlive(bool)), this, SLOT(slot_stm32IsAlive(bool)));
    connect(hardware, SIGNAL(sig_Use_Reader_Type(u_int8_t,u_int8_t,u_int8_t)), this, SLOT(slot_Use_Reader_Type(u_int8_t,u_int8_t,u_int8_t)));
    connect(hardware, SIGNAL(sig_MinSheng_Reader_IsAlive(bool)), this, SLOT(slot_MinSheng_Reader_IsAlive(bool)));
    connect(hardware, SIGNAL(sig_Identity_Reader_IsAlive(bool)), this, SLOT(slot_Identity_Reader_IsAlive(bool)));
    connect(hardware, SIGNAL(sig_IC_Reader_IsAlive(bool)), this, SLOT(slot_IC_Reader_IsAlive(bool)));

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
void MainWindow::slot_HardwareData(QString str)
{
    qDebug() << "MainWindow recv Hardware data";
    ui->textBrowser->clear();
    ui->textBrowser->insertPlainText(str);
}

void MainWindow::slot_stm32IsAlive(bool alive) // 单片机是否在线信号
{
    if (alive)
    {
        ui->Icon0_Label->setPixmap(QPixmap(":/icon/image/MCU.png"));
        ui->Icon0_Label->show();
    }
    else
    {
        ui->Icon0_Label->setPixmap(QPixmap(":/icon/image/MCU_R.png"));
        ui->Icon0_Label->show();
    }
}

void MainWindow::slot_Use_Reader_Type(u_int8_t ms, u_int8_t id, u_int8_t ic)
{
    u_int8_t cnt = 0;
    QPixmap first_pix;
    QPixmap second_pix;

    if (hardware->hw_info.USE_MINSHENG_READER)
    {
        cnt++;

        if (ms)
        {
            first_pix = QPixmap(":/icon/image/MS.png");
        }
        else
        {
            first_pix = QPixmap(":/icon/image/MS_R.png");
        }
    }

    if (hardware->hw_info.USE_IDENTITY_READER)
    {
        cnt++;
        if (id)
        {
            if (cnt == 1)
                first_pix = QPixmap(":/icon/image/ID.png");
            else if (cnt == 2)
                second_pix = QPixmap(":/icon/image/ID.png");
        }
        else
        {
            if (cnt == 1)
                first_pix = QPixmap(":/icon/image/ID_R.png");
            else if (cnt == 2)
                second_pix = QPixmap(":/icon/image/ID_R.png");
        }
    }

    if (hardware->hw_info.USE_IC_READER)
    {
        cnt++;

        if (ic)
        {
            if (cnt == 1)
                first_pix = QPixmap(":/icon/image/IC.png");
            else if (cnt == 2)
                second_pix = QPixmap(":/icon/image/IC.png");
        }
        else
        {
            if (cnt == 1)
                first_pix = QPixmap(":/icon/image/IC_R.png");
            else if (cnt == 2)
                second_pix = QPixmap(":/icon/image/IC_R.png");
        }
    }

    if (cnt == 1)
    {
        ui->Icon1_Label->setPixmap(first_pix);
        ui->Icon1_Label->show();
    }
    else if (cnt == 2)
    {
        ui->Icon1_Label->setPixmap(first_pix);
        ui->Icon1_Label->show();
        ui->Icon2_Label->setPixmap(second_pix);
        ui->Icon2_Label->show();
    }
}

void MainWindow::slot_MinSheng_Reader_IsAlive(bool alive)
{
    if (alive)
    {
    }
    else
    {
    }
}

void MainWindow::slot_Identity_Reader_IsAlive(bool alive)
{
    if (alive)
    {
    }
    else
    {
    }
}

void MainWindow::slot_IC_Reader_IsAlive(bool alive)
{
    if (alive)
    {
    }
    else
    {
    }
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
