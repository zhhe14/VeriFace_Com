#include "crc16.h"
#include "mainwindow.h"
#include <QDebug>

HARDWARE::HARDWARE(QObject *parent) :
        QObject(parent)
{
    //this->setWindowFlags(Qt::FramelessWindowHint);      //加这句可以可以让对话框的控制台不显示。
    //struct PortSettings hw_ComSetting={BAUD115200,DATA_8,PAR_NONE,STOP_1,FLOW_OFF,500};
    //declear a struct for cortain the member of mycom;
    //hw_Com = new Posix_QextSerialPort("/dev/ttyUSB0",QextSerialBase::Polling);
    hw_Com = new Posix_QextSerialPort("/dev/ttyS1",QextSerialBase::Polling);
    //define the member of serial ,and sen the number ,declear for it in the constructor
    hw_Com->setBaudRate(BAUD115200); 	//波特率设置，我们设置为115200
    hw_Com->setDataBits(DATA_8);  		//数据位设置，我们设置为8位数据位
    hw_Com->setParity(PAR_NONE);  		//奇偶校验设置，我们设置为无校验
    hw_Com->setStopBits(STOP_1);  		//停止位设置，我们设置为1位停止位
    hw_Com->setFlowControl(FLOW_OFF); 	//数据流控制设置，我们设置为无数据流控制
    hw_Com->open(QIODevice::ReadWrite); //open the serial with the style of ReadWrite.
    //connect(hw_Com,SIGNAL(readyRead()),this,SLOT(readMyCom()));   //这个为中断方式，但linux下只能用查询方式
    //conect the SLOT and SIGNAL,when there are data in the serial buffer,it will read the serial

    hw_info.Open_Door_Continue_Time = 5000;
    hw_info.USE_MINSHENG_READER = 0x00;
    hw_info.USE_IDENTITY_READER = 0x00;
    hw_info.USE_IC_READER = 0x00;
    hw_info.PWM_VALUE = 0x00;

    hw_Com->setTimeout(100);
    readTimer = new QTimer(this);
    readTimer->start(100);
    connect(readTimer,SIGNAL(timeout()),this,SLOT(readMyCom()));

    STM32_ALIVE = false;
    MinSheng_Reader_ALIVE = false;
    Identity_Reader_ALIVE = false;
    IC_Reader_ALIVE = false;
    STM32_INFO_SYNC_FLAG = false;
    //stm32IsAlive();
    A83T_Ready();

    connect(parent, SIGNAL(sendAppHardwareData(u_int8_t *, u_int32_t)), this, SLOT(recvAppHardwareData(u_int8_t *, u_int32_t)));

    aliveTimer = new QTimer(this);
    aliveTimer->start(10000);
    connect(aliveTimer,SIGNAL(timeout()), this, SLOT(checkAlive()));
}

HARDWARE::~HARDWARE()
{

}

bool HARDWARE::stm32IsAlive(void)//检测单片机读头在线情况
{
    unsigned char send_data[5] = {0x00, 0x05};
    unsigned int send_len = 5;
    unsigned short CrcValue = 0;
    QByteArray recv_data;
    u_int32_t recv_len = 0;

    // 发送命令
    send_data[2] = FUNCTION_CODE_STM32_IS_ALIVE;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);

    // 等待然后读取返回数据
    this->readTimer->stop();
    QThread::msleep(200);
    recv_data = hw_Com->readAll(); //读取串口缓冲区的所有数据给临时变量recv_data
    if (!recv_data.isEmpty())
    {
        recv_data=recv_data.toHex();
        //qDebug() << "check stm32IsAlive recv: " << recv_data;
        recv_len = recv_data.length() / 2;
        unsigned char recv_data_ascii[recv_len];
        //转成ASCII形式字符串
        for (u_int i = 0; i < recv_len; i++)
        {
            recv_data_ascii[i] = ((this->ConvertHexChar(*(recv_data.data() + i * 2)) << 4)
                                  | this->ConvertHexChar(*(recv_data.data() + i * 2 + 1)));
        }

        if (recv_len == (u_int32_t)((recv_data_ascii[0] << 8) + recv_data_ascii[1]))
        {
            if (CRC16((unsigned char *)recv_data_ascii, recv_len) == 0)//CRC校验
            {
                if (recv_data_ascii[2] == FUNCTION_CODE_STM32_IS_ALIVE)
                {
                    if (this->hw_info.USE_MINSHENG_READER == 0)
                        this->hw_info.USE_MINSHENG_READER = recv_data_ascii[3];

                    if (this->hw_info.USE_IDENTITY_READER == 0)
                        this->hw_info.USE_IDENTITY_READER = recv_data_ascii[4];

                    if (this->hw_info.USE_IC_READER == 0)
                        this->hw_info.USE_IC_READER = recv_data_ascii[5];

                    STM32_ALIVE = true;
                    MinSheng_Reader_ALIVE = recv_data_ascii[3];
                    Identity_Reader_ALIVE = recv_data_ascii[4];
                    IC_Reader_ALIVE = recv_data_ascii[5];

                    if (this->hw_info.USE_MINSHENG_READER != MinSheng_Reader_ALIVE
                       || this->hw_info.USE_IDENTITY_READER != Identity_Reader_ALIVE
                       || this->hw_info.USE_IC_READER != IC_Reader_ALIVE)
                        qDebug("MS: %d  ID: %d  IC: %d", MinSheng_Reader_ALIVE, Identity_Reader_ALIVE, IC_Reader_ALIVE);

                }
            }
        }
    }
    else
    {
        qDebug() << "check stm32IsAlive no answer" << recv_data;
        qDebug("FUNCTION_CODE_STM32_IS_NOALIVE");
        STM32_ALIVE = false;
        MinSheng_Reader_ALIVE = false;
        Identity_Reader_ALIVE = false;
        IC_Reader_ALIVE = false;
    }

    emit sig_stm32IsAlive(STM32_ALIVE);
    emit sig_Use_Reader_Type(MinSheng_Reader_ALIVE, Identity_Reader_ALIVE, IC_Reader_ALIVE);

    this->readTimer->start(100);

    return STM32_ALIVE;
}

bool HARDWARE::MinSheng_Reader_IsAlive()	//检测民生卡读头是否在线
{
    unsigned char send_data[5] = {0x00, 0x05};
    unsigned int send_len = 5;
    unsigned short CrcValue = 0;
#if 1
    QByteArray recv_data;
    u_int32_t recv_len = 0;
    short status = 0xff;
#endif

    send_data[2] = FUNCTION_CODE_MINSHENG_READER_ALIVE;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);

#if 1
    this->readTimer->stop();
    QThread::msleep(200);
    recv_data = hw_Com->readAll(); //读取串口缓冲区的所有数据给临时变量recv_data
    if (!recv_data.isEmpty())
    {
        recv_data=recv_data.toHex();
        //qDebug() << "check MinSheng_Reader_IsAlive recv: " << recv_data;
        recv_len = recv_data.length() / 2;
        unsigned char recv_data_ascii[recv_len];
        //转成ASCII形式字符串
        for (u_int i = 0; i < recv_len; i++)
        {
            recv_data_ascii[i] = ((this->ConvertHexChar(*(recv_data.data() + i * 2)) << 4)
                                  | this->ConvertHexChar(*(recv_data.data() + i * 2 + 1)));
        }

        if (recv_len == (u_int32_t)((recv_data_ascii[0] << 8) + recv_data_ascii[1]))
        {
            if (CRC16((unsigned char *)recv_data_ascii, recv_len) == 0)//CRC校验
            {
                if (recv_data_ascii[2] == FUNCTION_CODE_MINSHENG_READER_ALIVE)
                {

                    status = recv_data_ascii[3] << 8 | recv_data_ascii[4];
                    if (status == 0x0001)
                    {
                        qDebug("FUNCTION_CODE_MINSHENG_READER_ALIVE");
                        MinSheng_Reader_ALIVE = true;
                        emit sig_MinSheng_Reader_IsAlive(true);
                    }
                    else
                    {
                        qDebug("FUNCTION_CODE_MINSHENG_READER_NOALIVE");
                        MinSheng_Reader_ALIVE = false;
                        emit sig_MinSheng_Reader_IsAlive(false);
                    }
                }
            }
        }
    }
    else
    {
        qDebug() << "check MinSheng_Reader_IsAlive no answer" << recv_data;
        qDebug("FUNCTION_CODE_MINSHENG_READER_NOALIVE");
        MinSheng_Reader_ALIVE = false;
        emit sig_MinSheng_Reader_IsAlive(false);
    }

    this->readTimer->start(100);

    return (status != 0xff) ? MinSheng_Reader_ALIVE : false;
#else

    return true;
#endif
}

bool HARDWARE::Identity_Reader_IsAlive()	//检测身份证读头是否在线
{
    unsigned char send_data[5] = {0x00, 0x05};
    unsigned int send_len = 5;
    unsigned short CrcValue = 0;
#if 1
    QByteArray recv_data;
    u_int32_t recv_len = 0;
    short status = 0xff;
#endif

    send_data[2] = FUNCTION_CODE_IDENTITY_READER_ALIVE;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);

#if 1
    this->readTimer->stop();
    QThread::msleep(200);
    recv_data = hw_Com->readAll(); //读取串口缓冲区的所有数据给临时变量recv_data
    if (!recv_data.isEmpty())
    {
        recv_data=recv_data.toHex();
        //qDebug() << "check Identity_Reader_IsAlive recv: " << recv_data;
        recv_len = recv_data.length() / 2;
        unsigned char recv_data_ascii[recv_len];
        //转成ASCII形式字符串
        for (u_int i = 0; i < recv_len; i++)
        {
            recv_data_ascii[i] = ((this->ConvertHexChar(*(recv_data.data() + i * 2)) << 4)
                                  | this->ConvertHexChar(*(recv_data.data() + i * 2 + 1)));
        }

        if (recv_len == (u_int32_t)((recv_data_ascii[0] << 8) + recv_data_ascii[1]))
        {
            if (CRC16((unsigned char *)recv_data_ascii, recv_len) == 0)//CRC校验
            {
                if (recv_data_ascii[2] == FUNCTION_CODE_IDENTITY_READER_ALIVE)
                {

                    status = recv_data_ascii[3] << 8 | recv_data_ascii[4];
                    if (status == 0x0001)
                    {
                        qDebug("FUNCTION_CODE_IDENTITY_READER_ALIVE");
                        Identity_Reader_ALIVE = true;
                        emit sig_Identity_Reader_IsAlive(true);
                    }
                    else
                    {
                        qDebug("FUNCTION_CODE_IDENTITY_READER_NOALIVE");
                        Identity_Reader_ALIVE = false;
                        emit sig_Identity_Reader_IsAlive(false);
                    }
                }
            }
        }
    }
    else
    {
        qDebug() << "check Identity_Reader_IsAlive no answer" << recv_data;
        qDebug("FUNCTION_CODE_IDENTITY_READER_NOALIVE");
        Identity_Reader_ALIVE = false;
        emit sig_Identity_Reader_IsAlive(false);
    }

    this->readTimer->start(100);

    return (status != 0xff) ? Identity_Reader_ALIVE : false;
#else

    return true;
#endif
}

bool HARDWARE::IC_Reader_IsAlive()		//检测IC卡读头是否在线
{
    unsigned char send_data[5] = {0x00, 0x05};
    unsigned int send_len = 5;
    unsigned short CrcValue = 0;
#if 1
    QByteArray recv_data;
    u_int32_t recv_len = 0;
    short status = 0xff;
#endif

    send_data[2] = FUNCTION_CODE_IC_READER_ALIVE;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);

#if 1
    this->readTimer->stop();
    QThread::msleep(200);
    recv_data = hw_Com->readAll(); //读取串口缓冲区的所有数据给临时变量recv_data
    if (!recv_data.isEmpty())
    {
        recv_data=recv_data.toHex();
        //qDebug() << "check IC_Reader_ALIVE recv: " << recv_data;
        recv_len = recv_data.length() / 2;
        unsigned char recv_data_ascii[recv_len];
        //转成ASCII形式字符串
        for (u_int i = 0; i < recv_len; i++)
        {
            recv_data_ascii[i] = ((this->ConvertHexChar(*(recv_data.data() + i * 2)) << 4)
                                  | this->ConvertHexChar(*(recv_data.data() + i * 2 + 1)));
        }

        if (recv_len == (u_int32_t)((recv_data_ascii[0] << 8) + recv_data_ascii[1]))
        {
            if (CRC16((unsigned char *)recv_data_ascii, recv_len) == 0)//CRC校验
            {
                if (recv_data_ascii[2] == FUNCTION_CODE_IC_READER_ALIVE)
                {

                    status = recv_data_ascii[3] << 8 | recv_data_ascii[4];
                    if (status == 0x0001)
                    {
                        qDebug("FUNCTION_CODE_IC_READER_ALIVE");
                        emit sig_IC_Reader_IsAlive(true);
                        IC_Reader_ALIVE = true;
                    }
                    else
                    {
                        qDebug("FUNCTION_CODE_IC_READER_NOALIVE");
                        emit sig_IC_Reader_IsAlive(false);
                        IC_Reader_ALIVE = false;
                    }
                }
            }
        }
    }
    else
    {
        qDebug() << "check IC_Reader_ALIVE no answer" << recv_data;
        qDebug("FUNCTION_CODE_IC_READER_NOALIVE");
        IC_Reader_ALIVE = false;
        emit sig_IC_Reader_IsAlive(false);
    }

    this->readTimer->start(100);

    return (status != 0xff) ? IC_Reader_ALIVE : false;
#else

    return true;
#endif
}

void HARDWARE::checkAlive()
{
    if (STM32_ALIVE == false)
    {
        qDebug() << "Send A83T_Ready";
        this->A83T_Ready();
        return ;
    }

    if (STM32_INFO_SYNC_FLAG == false)
    {
        this->hw_info = this->getStm32Info();
        if (this->hw_info.LIGHT_SENSOR_VALUE != 0xff)
        {
            Stm32_Version = QString(QLatin1String(this->hw_info.Version));
            qDebug() << "Version:" << Stm32_Version;
            qDebug() << "Open_Door_Continue_Time:" << this->hw_info.Open_Door_Continue_Time;
            qDebug() << "USE_MINSHENG_READER:" << this->hw_info.USE_MINSHENG_READER;
            qDebug() << "USE_IDENTITY_READER:" << this->hw_info.USE_IDENTITY_READER;
            qDebug() << "USE_IC_READER:" << this->hw_info.USE_IC_READER;
            qDebug() << "LIGHT_SENSOR_VALUE:" << this->hw_info.LIGHT_SENSOR_VALUE;
            qDebug() << "PWM_VALUE:" << this->hw_info.PWM_VALUE;
            STM32_INFO_SYNC_FLAG = true;
            emit sig_Use_Reader_Type(hw_info.USE_MINSHENG_READER, hw_info.USE_IDENTITY_READER, hw_info.USE_IC_READER);

            qDebug() << "LinghtSensor_Status: " << this->getLinghtSensor_Status();
            qDebug() << "InfraredPWM val: " << this->getInfraredPWM();
        }
        //return ;
    }

    this->stm32IsAlive();
}

void HARDWARE::A83T_Ready(void) // A83T Ready
{
    unsigned char send_data[5] = {0x00, 0x05};
    unsigned int send_len = 5;
    unsigned short CrcValue = 0;

    send_data[2] = FUNCTION_CODE_A83T_READY;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);
}

void HARDWARE::resetStm32()// 复位单片机
{
    unsigned char send_data[5] = {0x00, 0x05};
    unsigned int send_len = 5;
    unsigned short CrcValue = 0;

    send_data[2] = FUNCTION_CODE_A83T_RESET_POS;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);
}

void HARDWARE::setLogoLed(short status /*LOGOLED_ON | LOGOLED_OFF*/)// 控制开关Logo 灯
{
    unsigned char send_data[7] = {0x00, 0x07};
    unsigned int send_len = 7;
    unsigned short CrcValue = 0;

    send_data[2] = FUNCTION_CODE_A83T_LOGO;
    send_data[3] = 0x00;
    if (status == LOGOLED_ON)
        send_data[4] = 0x01;
    else if (status == LOGOLED_OFF)
        send_data[4] = 0x00;

    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);
}

void HARDWARE::setInfraredPWM(u_int8_t pwm /*min: 0, max: 100, 0 to close*/)// 控制红外灯板PWM
{
    unsigned char send_data[20] = {0x00, 0x07, 0x21};
    unsigned int send_len = 7;
    unsigned short CrcValue = 0;

    if (pwm > 100)
        return;

    send_data[3] = 0x00;
    send_data[4] = pwm & 0xff;
    //qDebug("pwm %02x %02x\n", send_data[3], send_data[4]);
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);
}

void HARDWARE::setCtrlDoor(short status /*DOOR_OPEN | DOOR_CLOSE*/)	// 控制开关锁
{
    unsigned char send_data[7] = {0x00, 0x07};
    unsigned int send_len = 7;
    unsigned short CrcValue = 0;

    send_data[2] = FUNCTION_CODE_A83T_LOCK;
    send_data[3] = 0x00;
    if (status == DOOR_OPEN)
        send_data[4] = 0x01;
    else if (status == DOOR_CLOSE)
        send_data[4] = 0x00;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);
}

Hardware_Info HARDWARE::getStm32Info(void)	// A83T获取单片机参数
{
    unsigned char send_data[5] = {0x00, 0x05};
    unsigned int send_len = 5;
    unsigned short CrcValue = 0;
    u_int32_t recv_len = 0;
    QByteArray recv_data;
    Hardware_Info temp_info;

    temp_info.LIGHT_SENSOR_VALUE = 0xff;

    send_data[2] = FUNCTION_CODE_A83T_GET_STM32_INFO;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);

    this->readTimer->stop();
    QThread::msleep(200);
    recv_data = hw_Com->readAll(); //读取串口缓冲区的所有数据给临时变量recv_data
    if (!recv_data.isEmpty())
    {
        recv_data=recv_data.toHex();
        qDebug() << "getStm32Info recv: " << recv_data;
        recv_len = recv_data.length() / 2;
        unsigned char recv_data_ascii[recv_len];
        //转成ASCII形式字符串
        for (u_int i = 0; i < recv_len; i++)
        {
            recv_data_ascii[i] = ((this->ConvertHexChar(*(recv_data.data() + i * 2)) << 4)
                                  | this->ConvertHexChar(*(recv_data.data() + i * 2 + 1)));
        }

        if (recv_len == (u_int32_t)((recv_data_ascii[0] << 8) + recv_data_ascii[1]))
        {
            if (CRC16((unsigned char *)recv_data_ascii, recv_len) == 0)//CRC校验
            {
                if (recv_data_ascii[2] == FUNCTION_CODE_A83T_GET_STM32_INFO)
                {
                    memcpy(&temp_info, &recv_data_ascii[3], sizeof(Hardware_Info));
                    //qDebug() << "get Stm32Info OK";
                }
            }
        }
    }
    else
    {
        qDebug() << "getStm32Info recv none" << recv_data;
    }

    this->readTimer->start(100);

    return temp_info;
}

bool HARDWARE::getLinghtSensor_Status(void)	// A83T获取光敏状态
{
    unsigned char send_data[5] = {0x00, 0x05};
    unsigned int send_len = 5;
    unsigned short CrcValue = 0;
    u_int32_t recv_len = 0;
    QByteArray recv_data;
    bool Light_Sensor_Status = 0;

    send_data[2] = FUNCTION_CODE_A83T_GET_LIGHT_SENSOR;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);

    this->readTimer->stop();
    QThread::msleep(200);
    recv_data = hw_Com->readAll(); //读取串口缓冲区的所有数据给临时变量recv_data
    if (!recv_data.isEmpty())
    {
        recv_data=recv_data.toHex();
        //qDebug() << "getLinghtSensor_Status recv: " << recv_data;
        recv_len = recv_data.length() / 2;
        unsigned char recv_data_ascii[recv_len];
        //转成ASCII形式字符串
        for (u_int i = 0; i < recv_len; i++)
        {
            recv_data_ascii[i] = ((this->ConvertHexChar(*(recv_data.data() + i * 2)) << 4)
                                  | this->ConvertHexChar(*(recv_data.data() + i * 2 + 1)));
        }

        if (recv_len == (u_int32_t)((recv_data_ascii[0] << 8) + recv_data_ascii[1]))
        {
            if (CRC16((unsigned char *)recv_data_ascii, recv_len) == 0)//CRC校验
            {
                //qDebug() << "crc16 OK";
                if (recv_data_ascii[2] == FUNCTION_CODE_A83T_GET_LIGHT_SENSOR)
                {
                    Light_Sensor_Status = recv_data_ascii[3];
                    //qDebug() << "get Light_Sensor_Status OK";
                }
            }
        }
    }
    else
        qDebug() << "getLinghtSensor_Status recv nothing";


    this->readTimer->start(100);

    return Light_Sensor_Status;
}

u_int8_t HARDWARE::getInfraredPWM(void)		// A83T获取当前红外灯板PWM值
{
    unsigned char send_data[5] = {0x00, 0x05};
    unsigned int send_len = 5;
    unsigned short CrcValue = 0;
    u_int32_t recv_len = 0;
    QByteArray recv_data;
    u_int8_t temp_pwm = 0;

    send_data[2] = FUNCTION_CODE_A83T_GET_INFRARED_PWM;
    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);

    this->readTimer->stop();
    QThread::msleep(200);
    recv_data = hw_Com->readAll(); //读取串口缓冲区的所有数据给临时变量recv_data
    if (!recv_data.isEmpty())
    {
        recv_data=recv_data.toHex();
        //qDebug() << "get InfraredPWM recv: " << recv_data;
        recv_len = recv_data.length() / 2;
        unsigned char recv_data_ascii[recv_len];
        //转成ASCII形式字符串
        for (u_int i = 0; i < recv_len; i++)
        {
            recv_data_ascii[i] = ((this->ConvertHexChar(*(recv_data.data() + i * 2)) << 4)
                                  | this->ConvertHexChar(*(recv_data.data() + i * 2 + 1)));
        }

        if (recv_len == (u_int32_t)((recv_data_ascii[0] << 8) + recv_data_ascii[1]))
        {
            if (CRC16((unsigned char *)recv_data_ascii, recv_len) == 0)//CRC校验
            {
                //qDebug() << "crc16 OK";
                if (recv_data_ascii[2] == FUNCTION_CODE_A83T_GET_INFRARED_PWM)
                {
                    memcpy(&temp_pwm, &recv_data_ascii[3], sizeof(temp_pwm));
                    //qDebug() << "get InfraredPWM OK";
                }
            }
        }
    }
    else
        qDebug() << "get InfraredPWM recv nothing";

    this->readTimer->start(100);

    return temp_pwm;
}

u_int8_t HARDWARE::setStm32Info(Hardware_Set_Info *hw_set_info) // A83T设置单片机参数
{
    unsigned char send_data[5 + sizeof(Hardware_Set_Info)] = { 0 };
    unsigned int send_len = 0;
    unsigned short CrcValue = 0;
    u_int32_t recv_len = 0;
    QByteArray recv_data;
    u_int8_t ret = 0;

    qDebug() << "setStm32Info";
    send_data[0] = 0x00;
    send_data[1] = 5 + sizeof(Hardware_Set_Info);
    send_data[2] = FUNCTION_CODE_A83T_SET_STM32_INFO;
    send_len += 3;
    memcpy(&send_data[3], (u_int8_t *)hw_set_info, sizeof(Hardware_Set_Info));
    send_len += sizeof(Hardware_Set_Info);
    send_len += 2;

    CrcValue = CRC16(send_data, send_len - 2);//计算CRC
    send_data[send_len -2] = (unsigned char )CrcValue;
    send_data[send_len -1] = CrcValue >> 8;
    hw_Com->write((char *)send_data, send_len);

    this->readTimer->stop();
    QThread::msleep(200);
    recv_data = hw_Com->readAll(); //读取串口缓冲区的所有数据给临时变量recv_data
    if (!recv_data.isEmpty())
    {
        recv_data=recv_data.toHex();
        qDebug() << "get InfraredPWM recv: " << recv_data;
        recv_len = recv_data.length() / 2;
        unsigned char recv_data_ascii[recv_len];
        //转成ASCII形式字符串
        for (u_int i = 0; i < recv_len; i++)
        {
            recv_data_ascii[i] = ((this->ConvertHexChar(*(recv_data.data() + i * 2)) << 4)
                                  | this->ConvertHexChar(*(recv_data.data() + i * 2 + 1)));
        }

        if (recv_len == (u_int32_t)((recv_data_ascii[0] << 8) + recv_data_ascii[1]))
        {
            if (CRC16((unsigned char *)recv_data_ascii, recv_len) == 0)//CRC校验
            {
                //qDebug() << "crc16 OK";
                if (recv_data_ascii[2] == FUNCTION_CODE_A83T_SET_STM32_INFO)
                {
                    ret = recv_data_ascii[4];
                    if (ret == 0x00)
                        qDebug() << "set stm32info Failed";
                }
            }
        }
    }
    else
        qDebug() << "set stm32info recv nothing";

    this->readTimer->start(100);

    return ret;
}

void HARDWARE::recvAppHardwareData(u_int8_t *data, u_int32_t len) //接收APP数据
{
    qDebug() << "HARDWARE send serial data";
    hw_Com->write((char *)data, len);
}

QString HARDWARE::asciiToQString(u_int8_t *recv_data, u_int32_t recv_len) // 16进制转字符串
{
    QString str;
    for (u_int i = 0; i < recv_len; i++)
    {
        str += (recv_data[i] >> 4) > 9 ?  (recv_data[i] >> 4) - 10 + 'a' : (recv_data[i] >> 4) + '0';
        str += (recv_data[i] & 0xf) > 9 ?  (recv_data[i] & 0xf)  - 10 + 'a' : (recv_data[i] & 0xf) + '0';
    }
    //qDebug() << "ascii: " << str;

    return str;
}

char HARDWARE::ConvertHexChar(char ch) //转成ASCII形式字符串
{
    if ((ch >= '0') && (ch <= '9'))
        return ch-0x30;
    else if ((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if ((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10;
    else return (-1);
}

void HARDWARE::readMyCom()  //The function for read
{
    u_int recv_len = 0;
    QByteArray recv_data;

#if 0
    int num1 = 0, num2 = 0;
    // 判断是否接收完成
    do {
        num1 = hw_Com->bytesAvailable(); //第一次查看串口缓冲区的数据
        QThread::msleep(1);
        num2 = hw_Com->bytesAvailable(); //第二次查看串口缓冲区的数据
        QThread::msleep(1);
    } while (num1 != num2);
#endif
    //read all data int the buffer ,and send it to the recv_dataorary variable "recv_data"
    recv_data = hw_Com->readAll(); //读取串口缓冲区的所有数据给临时变量recv_data

    if (!recv_data.isEmpty())
    {
        this->aliveTimer->stop();
        //qDebug("cnt: %d", cnt++);
        recv_data=recv_data.toHex();
        recv_len = recv_data.length() / 2;
        if (recv_len < 50)
            qDebug() << "recv: " << recv_data;
        unsigned char recv_data_ascii[recv_len];
        //转成ASCII形式字符串
        for (u_int i = 0; i < recv_len; i++)
        {
            recv_data_ascii[i] = ((this->ConvertHexChar(*(recv_data.data() + i * 2)) << 4)
                                  | this->ConvertHexChar(*(recv_data.data() + i * 2 + 1)));
            //qDebug("%02x ", recv_data_ascii[i]);
        }
        this->aliveTimer->start();
        this->Seaial_RecvData_Deal(recv_data_ascii, recv_len);
    }
}

u_int8_t HARDWARE::Seaial_RecvData_Deal(u_int8_t *pbuff, u_int plen) // 处理串口接收到的数据
{
    unsigned char cmd = 0xff;
    short status = 0xff;
    qDebug("Seaial_RecvData_Deal");

    cmd = pbuff[2];

    if (plen == (u_int)((pbuff[0] << 8) + pbuff[1]))
    {
        if (cmd != FUNCTION_CODE_A83T_IDENTITY_CARD)
        {
            if (CRC16((unsigned char *)pbuff, plen) != 0)//CRC校验
            {
                qDebug() << "crc16 Failed";
                return 1;
            }

            switch (cmd)
            {
            case FUNCTION_CODE_A83T_READY:
            {
                status = pbuff[3] << 8 | pbuff[4];
                if (status == 0x0001)
                {
                    STM32_ALIVE = true;
                    qDebug("FUNCTION_CODE_STM32_IS_READY");
                    emit sig_stm32IsAlive(true);
                }
                else
                {
                    qDebug("FUNCTION_CODE_STM32_IS_NOREADY");
                    STM32_ALIVE = false;
                    emit sig_stm32IsAlive(false);
                }
            }
                break;

            case FUNCTION_CODE_STM32_IS_ALIVE:				//检测单片机是否在线
            {
                if (this->hw_info.USE_MINSHENG_READER == 0)
                    this->hw_info.USE_MINSHENG_READER = pbuff[3];

                if (this->hw_info.USE_IDENTITY_READER == 0)
                    this->hw_info.USE_IDENTITY_READER = pbuff[4];

                if (this->hw_info.USE_IC_READER == 0)
                    this->hw_info.USE_IC_READER = pbuff[5];

                MinSheng_Reader_ALIVE = pbuff[3];
                Identity_Reader_ALIVE = pbuff[4];
                IC_Reader_ALIVE = pbuff[5];

                if (this->hw_info.USE_MINSHENG_READER != MinSheng_Reader_ALIVE
                        || this->hw_info.USE_IDENTITY_READER != Identity_Reader_ALIVE
                        || this->hw_info.USE_IC_READER != IC_Reader_ALIVE)
                    qDebug("MS: %d  ID: %d  IC: %d", MinSheng_Reader_ALIVE, Identity_Reader_ALIVE, IC_Reader_ALIVE);

                qDebug("FUNCTION_CODE_STM32_IS_ALIVE");
                STM32_ALIVE = true;
                emit sig_stm32IsAlive(true);
                emit sig_Use_Reader_Type(MinSheng_Reader_ALIVE, Identity_Reader_ALIVE, IC_Reader_ALIVE);
            }
                break;

            case FUNCTION_CODE_MINSHENG_READER_ALIVE:	//检测民生卡读头是否在线
            {
                status = pbuff[3] << 8 | pbuff[4];
                if (status == 0x0001)
                {
                    qDebug("FUNCTION_CODE_MINSHENG_READER_ALIVE");
                    emit sig_MinSheng_Reader_IsAlive(true);
                }
                else
                {
                    qDebug("FUNCTION_CODE_MINSHENG_READER_NOALIVE");
                    emit sig_MinSheng_Reader_IsAlive(false);
                }
            }
                break;

            case FUNCTION_CODE_IDENTITY_READER_ALIVE:		//检测身份证读头是否在线
            {
                status = pbuff[3] << 8 | pbuff[4];
                if (status == 0x0001)
                {
                    qDebug("FUNCTION_CODE_IDENTITY_READER_ALIVE");
                    emit sig_Identity_Reader_IsAlive(true);
                }
                else
                {
                    qDebug("FUNCTION_CODE_IDENTITY_READER_NOALIVE");
                    emit sig_Identity_Reader_IsAlive(false);
                }
            }
                break;

            case FUNCTION_CODE_IC_READER_ALIVE:				//检测IC卡读头是否在线
            {
                status = pbuff[3] << 8 | pbuff[4];
                if (status == 0x0001)
                {
                    qDebug("FUNCTION_CODE_IC_READER_ALIVE");
                    emit sig_IC_Reader_IsAlive(true);
                }
                else
                {
                    qDebug("FUNCTION_CODE_IC_READER_NOALIVE");
                    emit sig_IC_Reader_IsAlive(false);
                }
            }
                break;

            case FUNCTION_CODE_STM32_UP_INFO:				//STM32上传单片机参数
            {
                qDebug("FUNCTION_CODE_STM32_UP_INFO");
                memcpy(&this->hw_info, &pbuff[3], sizeof(Hardware_Info));

                Stm32_Version = QString(QLatin1String(this->hw_info.Version));
                qDebug() << "Version:" << Stm32_Version;
                qDebug() << "Open_Door_Continue_Time:" << this->hw_info.Open_Door_Continue_Time;
                qDebug() << "USE_MINSHENG_READER:" << this->hw_info.USE_MINSHENG_READER;
                qDebug() << "USE_IDENTITY_READER:" << this->hw_info.USE_IDENTITY_READER;
                qDebug() << "USE_IC_READER:" << this->hw_info.USE_IC_READER;
                qDebug() << "LIGHT_SENSOR_VALUE:" << this->hw_info.LIGHT_SENSOR_VALUE;
                qDebug() << "PWM_VALUE:" << this->hw_info.PWM_VALUE;
                STM32_INFO_SYNC_FLAG = true;
                emit sig_Use_Reader_Type(hw_info.USE_MINSHENG_READER, hw_info.USE_IDENTITY_READER, hw_info.USE_IC_READER);
            }
                break;

            case FUNCTION_CODE_A83T_SET_STM32_INFO:			//A83T设置单片机参数
            {
                qDebug("FUNCTION_CODE_A83T_SET_STM32_INFO");
            }
                break;

            case FUNCTION_CODE_STM32_OPENDOOR:				//STM32上传开门动作
            {
                qDebug("FUNCTION_CODE_STM32_OPENDOOR");
            }
                break;

            case FUNCTION_CODE_A83T_GET_STM32_INFO:			// A83T获取单片机参数
            {
                qDebug("FUNCTION_CODE_A83T_GET_STM32_INFO");
                memcpy(&this->hw_info, &pbuff[3], sizeof(Hardware_Info));

                Stm32_Version = QString(QLatin1String(this->hw_info.Version));
                qDebug() << "Version:" << Stm32_Version;
                qDebug() << "Open_Door_Continue_Time: " << this->hw_info.Open_Door_Continue_Time;
                qDebug() << "USE_MINSHENG_READER:" << this->hw_info.USE_MINSHENG_READER;
                qDebug() << "USE_IDENTITY_READER:" << this->hw_info.USE_IDENTITY_READER;
                qDebug() << "USE_IC_READER:" << this->hw_info.USE_IC_READER;
                qDebug() << "LIGHT_SENSOR_VALUE:" << this->hw_info.LIGHT_SENSOR_VALUE;
                qDebug() << "PWM_VALUE:" << this->hw_info.PWM_VALUE;
                STM32_INFO_SYNC_FLAG = true;
            }
                break;

            case FUNCTION_CODE_A83T_GET_LIGHT_SENSOR:        	// A83T获取光敏状态
            {
                qDebug("FUNCTION_CODE_A83T_GET_LIGHT_SENSOR");
            }
                break;

            case FUNCTION_CODE_A83T_GET_INFRARED_PWM:        	// A83T获取当前红外灯板PWM值
            {
                qDebug("FUNCTION_CODE_A83T_GET_INFRARED_PWM");
            }
                break;

            case FUNCTION_CODE_A83T_GET_DOOR_STATUS:        	// A83T获取当前锁状态
            {
                qDebug("FUNCTION_CODE_A83T_GET_DOOR_STATUS");
            }
                break;

            case FUNCTION_CODE_A83T_LOGO:						//A83T控制LOGO灯
            {
                qDebug("FUNCTION_CODE_A83T_LOGO");
            }
                break;

            case FUNCTION_CODE_A83T_INFRARED:					//A83T控制红外灯板
            {
                qDebug("FUNCTION_CODE_A83T_INFRARED");
            }
                break;

            case FUNCTION_CODE_A83T_LOCK:						//A83T控制锁
            {
                qDebug("FUNCTION_CODE_A83T_LOCK");
            }
                break;

            case FUNCTION_CODE_A83T_LIGHT_SENSOR:   			//STM32上传光敏状态
            {
                qDebug("FUNCTION_CODE_A83T_LIGHT_SENSOR");
                this->Light_Sensor_Deal(&pbuff[3], 2);
            }
                break;

            case FUNCTION_CODE_A83T_MINSHENG_CARD:				//STM32上传民生卡信息
            {
                qDebug("FUNCTION_CODE_A83T_MINSHENG_CARD");
                this->Minsheng_Card_Deal(&pbuff[3], plen - 5);
            }
                break;

            case FUNCTION_CODE_A83T_IC_CARD: 					//STM32上传IC 卡号
            {
                qDebug("FUNCTION_CODE_A83T_IC_CARD");
                this->IC_Card_Deal(&pbuff[3], 4);
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
            qDebug("FUNCTION_CODE_A83T_IDENTITY_CARD");
            if (CRC16((unsigned char *)crc_buf, sizeof(crc_buf)) != 0)//CRC校验
            {
                qDebug() << "crc16 Failed";
                return 2;
            }

            this->Identity_Card_Deal(&pbuff[3], plen - 5);
        }
    }

    return 0;
}

u_int8_t HARDWARE::Light_Sensor_Deal(u_int8_t *pbuff, u_int plen)	// 处理光敏数据
{
    u_int8_t Light_Sensor_Status = pbuff[1];
    QString str = this->asciiToQString(pbuff, plen);
    qDebug() << "Light_Sensor_Status: " << Light_Sensor_Status;
    this->hw_info.LIGHT_SENSOR_VALUE = Light_Sensor_Status;
    emit sig_LightSensor_Data(str);

    return Light_Sensor_Status;
}

u_int8_t HARDWARE::Minsheng_Card_Deal(u_int8_t *pbuff, u_int plen) 	// 处理民生卡数据
{
    qDebug("Minsheng_Card_Deal");
    u_int8_t MinSheng_Data[100];

    memcpy(MinSheng_Data, pbuff, plen);
    if (minsheng_packet_valid(MinSheng_Data) != NULL)
    {
        //qDebug() << "MinSheng_Data valid OK";
        QString str = this->asciiToQString(&MinSheng_Data[8], sizeof(MINSHENG_USER_INFO));
        emit sig_MinShengCard_Data(str);
    }
    else
        qDebug() << "MinSheng_Data valid Failed";

    return 0;
}

u_int8_t HARDWARE::Identity_Card_Deal(u_int8_t *pbuff, u_int plen) // 处理身份证数据
{
    qDebug("Identity_Card_Deal");
    u_int8_t Identity_Data[1296];

    memcpy(Identity_Data, pbuff, plen);
    if (this->identity_packet_valid(Identity_Data) != NULL)
    {
        //qDebug() << "Identity_Data valid OK";
        QString str = this->asciiToQString(&Identity_Data[14], sizeof(IDENTITY_USER_INFO));
        emit sig_IdentityCard_Data(str);
    }
    else
        qDebug() << "Identity_Data valid Failed";

    return 0;
}

u_int8_t HARDWARE::IC_Card_Deal(u_int8_t *pbuff, u_int plen)	// 处理IC卡数据
{
    qDebug("IC_Card_Deal");
    u_int8_t IC_Data[5] = {0};

    memcpy(IC_Data, pbuff, plen);

    if ((IC_Data[0] != 0x00) || (IC_Data[1] != 0x00) || (IC_Data[2] != 0x00) || (IC_Data[3] != 0x00))
    {
        //qDebug() << "IC_Data valid OK";
        QString str = this->asciiToQString(&IC_Data[0], plen);
        qDebug() << "IC_Data:" << str;
        emit sig_IC_Data(str);
    }
    else
        qDebug() << "IC_Data valid Failed";

    return 0;
}

/* Determine whether the data is correct */
u_int8_t * HARDWARE::minsheng_packet_valid(u_int8_t * pbySrc)  // 校验民生卡数据
{
    MINSHENG_Info_PACKET * pstrucProc = (MINSHENG_Info_PACKET *)pbySrc;

    if ((MINSHENG_LAST_DATA_2 == pstrucProc->res[4])
                    && (MINSHENG_LAST_DATA_1 == pstrucProc->res[5]))
    {
        //qDebug("return true");
        return (u_int8_t *)(1);
    }

    //qDebug("return null");

    return NULL;
}

/* 数据逐字节按位异或*/
u_int8_t HARDWARE::chk_xor8(u_int8_t *pbySrc, u_int32_t dwDataLen)
{
    u_int8_t   byChecksum = pbySrc[0];
    u_int32_t  i = 0;

    for (i = 1;i < dwDataLen;i++)
    {
        byChecksum ^= pbySrc[i];
        //if (i % 32 == 0)
        //    QThread::msleep(1);
    }
    return byChecksum;
}

/* Identity data verify chk_sum */
u_int8_t * HARDWARE::identity_packet_valid(u_int8_t * pbySrc)
{
    u_int8_t head_identity_dummy[IDENTITY_SYNC_HEAD_LEN] = { 0xAA, 0xAA, 0xAA, 0x96, 0x69 };
    IDENTITY_PACKET * pstrucProc = (IDENTITY_PACKET *)pbySrc;

    if (memcmp(pstrucProc->sync_head, head_identity_dummy, IDENTITY_SYNC_HEAD_LEN) == 0)
    {
        u_int8_t packet_chk_sum = this->chk_xor8(&pbySrc[5], (HTON2(pstrucProc->data_len) - 1 + 2));
        //rt_kprintf("calc packet_chk_sum = %02X", packet_chk_sum);
#if 0
        for (rt_uint32_t i = 0; i < 256; i++)
            rt_kprintf("%02X", pbySrc[14 + i]);
#endif
        if (packet_chk_sum == pstrucProc->chk_sum)
        {
            //qDebug("return true");
            return (u_int8_t *)(& pstrucProc->text_info);
        }
    }

    //qDebug("return null");

    return NULL;
}
