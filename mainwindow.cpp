#include "crc16.h"
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
    //this->setWindowFlags(Qt::FramelessWindowHint);      //加这句可以可以让对话框的控制台不显示。
    // struct PortSettings myComSetting={BAUD115200,DATA_8,PAR_NONE,STOP_1,FLOW_OFF,500};
    //declear a struct for cortain the member of mycom;
    myCom = new Posix_QextSerialPort("/dev/ttyUSB0",QextSerialBase::Polling);
    //myCom = new Posix_QextSerialPort("/dev/ttyS1",QextSerialBase::Polling);
    //define the member of serial ,and sen the number ,declear for it in the constructor
    myCom->setBaudRate(BAUD115200); 	//波特率设置，我们设置为115200
    myCom->setDataBits(DATA_8);  		//数据位设置，我们设置为8位数据位
    myCom->setParity(PAR_NONE);  		//奇偶校验设置，我们设置为无校验
    myCom->setStopBits(STOP_1);  		//停止位设置，我们设置为1位停止位
    myCom->setFlowControl(FLOW_OFF); 	//数据流控制设置，我们设置为无数据流控制
    myCom->open(QIODevice::ReadWrite);  //open the serial with the style of ReadWrite.
    //connect(myCom,SIGNAL(readyRead()),this,SLOT(readMyCom()));   //这个为中断方式，但linux下只能用查询方式
    //conect the SLOT and SIGNAL,when there are data in the serial buffer,it will read the serial

    myCom->setTimeout(100);
    QTimer *readTimer = new QTimer(this);
    readTimer->start(100);
    connect(readTimer,SIGNAL(timeout()),this,SLOT(readMyCom()));
    connect(ui->lineEdit, SIGNAL(clicked()), this, SLOT(HandleLineEditclicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::HandleLineEditclicked()
{
    //ui->UserName_LineEdit->clearFocus();
    //ui->UserID_LineEdit->clearFocus();
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


char ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30;
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10;
    else return (-1);
}

void MainWindow::readMyCom()  //The function for read
{
    static int cnt = 0;
    u_int recv_len = 0;
    QByteArray recv_data = myCom->readAll();
    //read all data int the buffer ,and send it to the recv_dataorary variable "recv_data"
    if (!recv_data.isEmpty())
    {
        qDebug() << "cnt: " << cnt++ << endl;
        ui->textBrowser->clear();
        recv_data=recv_data.toHex();
        ui->textBrowser->insertPlainText(recv_data);

        recv_len = recv_data.length() / 2;
        unsigned char recv_data_ascii[recv_len];
        //转成ASCII形式字符串
        for (u_int i = 0; i < recv_len; i++)
        {
            recv_data_ascii[i] = ((ConvertHexChar(*(recv_data.data() + i * 2)) << 4)
                                  | ConvertHexChar(*(recv_data.data() + i * 2 + 1)));
            //qDebug("%02x ", recv_data_ascii[i]);
        }
        this->Seaial_RecvData_Deal(recv_data_ascii, recv_len);
    }
}

u_int8_t MainWindow::Seaial_RecvData_Deal(u_int8_t *pbuff, u_int plen)
{
    unsigned char cmd = 0xff;
    qDebug("Seaial_RecvData_Deal\r\n");

    cmd = pbuff[2];

    if (plen == ((pbuff[0] << 8) + pbuff[1]))
    {
        if (cmd != FUNCTION_CODE_A83T_IDENTITY_CARD)
        {
            switch (cmd)
            {
            case FUNCTION_CODE_A83T_LOGO:						//A83T控制LOGO灯
            {
                qDebug("FUNCTION_CODE_A83T_LOGO\r\n");
            }
                break;

            case FUNCTION_CODE_A83T_INFRARED:					//A83T控制红外灯板
            {
                qDebug("FUNCTION_CODE_A83T_INFRARED\r\n");
            }
                break;

            case FUNCTION_CODE_A83T_LOCK:						//A83T控制锁
            {
                qDebug("FUNCTION_CODE_A83T_LOCK\r\n");
            }
                break;

            case FUNCTION_CODE_A83T_LIGHT_SENSOR:   			//STM32上传光敏状态
            {
                qDebug("FUNCTION_CODE_A83T_LIGHT_SENSOR\r\n");
                if (CRC16((unsigned char *)pbuff, plen) == 0)//CRC校验
                {
                    qDebug() << "crc16 OK" << endl;
                }
                else
                {
                    qDebug() << "crc16 Fail" << endl;
                }
            }
                break;

            case FUNCTION_CODE_A83T_MINSHENG_CARD:				//STM32上传民生卡信息
            {
                qDebug("FUNCTION_CODE_A83T_MINSHENG_CARD\r\n");
                if (CRC16((unsigned char *)pbuff, plen) == 0)//CRC校验
                {
                    qDebug() << "crc16 OK" << endl;
                    this->Minsheng_Card_Deal(&pbuff[3], plen - 5);
                }
                else
                {
                    qDebug() << "crc16 Fail" << endl;
                }
            }
                break;

            case FUNCTION_CODE_A83T_IC_CARD: 					//STM32上传IC 卡号
            {
                qDebug("FUNCTION_CODE_A83T_IC_CARD\r\n");
                this->IC_Card_Deal(pbuff, plen);
            }
                break;

            default:
                break;
            }
        }
        else if (cmd == FUNCTION_CODE_A83T_IDENTITY_CARD)		//STM32上传身份证信息
        {
            //注: 因这里的数据过长，CRC16不能校验过长的数据，这里的CRC16是”总长度+功能码+身份证的chk_sum”的校验码
            unsigned char crc_buf[6];
            memcpy(&crc_buf[0], &pbuff[0], 3);
            memcpy(&crc_buf[3], &pbuff[plen - 3], 3);
            qDebug("crc_buf: %02x %02x %02x %02x %02x %02x\n", crc_buf[0], crc_buf[1], crc_buf[2], crc_buf[3], crc_buf[4], crc_buf[5]);
            qDebug("FUNCTION_CODE_A83T_IDENTITY_CARD\r\n");
            if (CRC16((unsigned char *)crc_buf, sizeof(crc_buf)) == 0)//CRC校验
            {
                qDebug() << "crc16 OK" << endl;
                this->Identity_Card_Deal(pbuff + 3, plen - 5);
            }
            else
            {
                qDebug() << "crc16 Fail" << endl;
            }
        }
    }

    return 0;
}

u_int8_t MainWindow::Minsheng_Card_Deal(u_int8_t *pbuff, u_int plen)
{
    qDebug("Minsheng_Card_Deal\r\n");
    //u_int8_t *MinSheng_Data = (u_int8_t *)malloc(plen * sizeof(u_int8_t));
    MINSHENG_Info_PACKET * pstrucProc = (MINSHENG_Info_PACKET *)pbuff;

    //if (MinSheng_Data == NULL)
    //    return 1;

    //memcpy(MinSheng_Data, pbuff, plen);
    qDebug() << "pbuff: " << pbuff[plen - 2] << pbuff[plen - 1] << endl;
    qDebug() << "pbuff len: " << plen;
    if (minsheng_packet_valid(pbuff) != NULL)
    {
        qDebug() << "MinSheng_Data valid OK" << endl;
    }

    //free((void *)MinSheng_Data);
    //MinSheng_Data = NULL;

    return 0;
}

u_int8_t MainWindow::Identity_Card_Deal(u_int8_t *pbuff, u_int plen)
{
    qDebug("Identity_Card_Deal\r\n");
    u_int8_t *Identity_Data = (u_int8_t *)malloc(plen * sizeof(u_int8_t));
    if (Identity_Data == NULL)
        return 1;

    memcpy(Identity_Data, pbuff, plen);
    qDebug() << "packlen: " << sizeof(u_int8_t);
    qDebug() << "packlen: " << sizeof(u_int16_t);
    qDebug() << "packlen: " << sizeof(u_int32_t);
    qDebug() << "packlen: " << sizeof(IDENTITY_Textinfo_PACKET);
    qDebug() << "packlen: " << sizeof(IDENTITY_PACKET);
    qDebug() << "packlen: " << sizeof(IDENTITY_Textinfo_PACKET);
    IDENTITY_PACKET * id = (IDENTITY_PACKET *)pbuff;
    qDebug("chksum: %02x\n", id->chk_sum);
    qDebug("name: %02x %02x\n", id->text_info.name[0], id->text_info.name[1]);
    qDebug("chksum: %d\n", id->data_len);
    free((void *)Identity_Data);
    Identity_Data = NULL;

    return 0;
}

u_int8_t MainWindow::IC_Card_Deal(u_int8_t *pbuff, u_int plen)
{
    qDebug("IC_Card_Deal\r\n");
    u_int8_t *IC_Data= (u_int8_t *)malloc(plen * sizeof(u_int8_t));
    if (IC_Data == NULL)
        return 1;

    memcpy(IC_Data, pbuff, plen);

    free((void *)IC_Data);
    IC_Data = NULL;

    return 0;
}

/* Determine whether the data is correct */
u_int8_t * MainWindow::minsheng_packet_valid(u_int8_t * pbySrc)  // 校验民生卡数据
{
    MINSHENG_Info_PACKET * pstrucProc = (MINSHENG_Info_PACKET *)pbySrc;

    qDebug() << "packlen: " << sizeof(MINSHENG_Info_PACKET);
    qDebug() << "packlen: " << pbySrc[98] << ", " << pbySrc[99];

    //if ((MINSHENG_LAST_DATA_2 == pstrucProc->res[4])
     //   && (MINSHENG_LAST_DATA_1 == pstrucProc->res[5]))
    if ((MINSHENG_LAST_DATA_2 == pbySrc[93])
        && (MINSHENG_LAST_DATA_1 == pbySrc[94]))
    {
        qDebug("return true\r\n");
        return (u_int8_t *)(1);
    }

    qDebug("return null\r\n");

    return NULL;
}

 /* 数据逐字节按位异或*/
u_int8_t MainWindow::chk_xor8(u_int8_t *pbySrc, u_int32_t dwDataLen)
{
    u_int8_t   byChecksum = pbySrc[0];
    u_int32_t  i = 0;

    for (i = 1;i < dwDataLen;i++)
    {
        byChecksum ^= pbySrc[i];
        if (i % 32 == 0)
            QThread::msleep(1);
    }
    return byChecksum;
}

void MainWindow::on_SendPWM_Button_clicked()
{
    unsigned char send_data[20] = {0x00, 0x07, 0x21};
    unsigned int send_len = 7;
    unsigned short CrcValue = 0;

    send_data[3] = ui->lineEdit->text().toShort() & 0xff00;
    send_data[4] = ui->lineEdit->text().toShort() & 0x00ff;
    //qDebug("pwm %02x %02x\n", send_data[3], send_data[4]);
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    //myCom->write(ui->lineEdit->text().toLatin1().data());
    myCom->write((char *)send_data, send_len);
}

void MainWindow::on_OpenDoor_Button_clicked()
{
    unsigned char send_data[7] = {0x00, 0x07};
    unsigned int send_len = 7;
    unsigned short CrcValue = 0;

    qDebug() << "on_OpenDoor_Button_clicked" << endl;
    send_data[2] = FUNCTION_CODE_A83T_LOCK;
    send_data[3] = 0x00;
    send_data[4] = 0x01;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    //myCom->write(ui->lineEdit->text().toLatin1().data());
    myCom->write((char *)send_data, send_len);
}

void MainWindow::on_CloseDoor_Button_clicked()
{
    unsigned char send_data[7] = {0x00, 0x07};
    unsigned int send_len = 7;
    unsigned short CrcValue = 0;

    send_data[2] = FUNCTION_CODE_A83T_LOCK;
    send_data[3] = 0x00;
    send_data[4] = 0x00;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    //myCom->write(ui->lineEdit->text().toLatin1().data());
    myCom->write((char *)send_data, send_len);
}

void MainWindow::on_OpenLogoLed_Button_clicked()
{
    unsigned char send_data[7] = {0x00, 0x07};
    unsigned int send_len = 7;
    unsigned short CrcValue = 0;

    send_data[2] = FUNCTION_CODE_A83T_LOGO;
    send_data[3] = 0x00;
    send_data[4] = 0x01;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    //myCom->write(ui->lineEdit->text().toLatin1().data());
    myCom->write((char *)send_data, send_len);
}

void MainWindow::on_CloseLogoLed_Button_clicked()
{
    unsigned char send_data[7] = {0x00, 0x07};
    unsigned int send_len = 7;
    unsigned short CrcValue = 0;

    send_data[2] = FUNCTION_CODE_A83T_LOGO;
    send_data[3] = 0x00;
    send_data[4] = 0x00;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    //myCom->write(ui->lineEdit->text().toLatin1().data());
    myCom->write((char *)send_data, send_len);
}
