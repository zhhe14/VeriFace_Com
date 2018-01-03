#ifndef HARDWARE_H
#define HARDWARE_H

#include <QObject>
#include "posix_qextserialport.h"
#include <QTimer>

#pragma pack(1) // 指定1字节对齐
typedef struct
{
    u_int32_t Open_Door_Continue_Time; 	//开门持续时间, 单位为毫秒ms
    u_int8_t USE_MINSHENG_READER;		// 是否使用民生卡读头
    u_int8_t USE_IDENTITY_READER;		// 是否使用身份证读头
    u_int8_t USE_IC_READER;				// 是否使用IC卡读头
    u_int8_t INIT_LIGHT_SENSOR_VALUE;	// 开机光敏初始值
    u_int8_t INIT_PWM_VALUE;			// 开机PWM初始值

} Hardware_Info;
#pragma pack() // 恢复默认对齐方式

class HARDWARE : public QObject
{
    Q_OBJECT
public:
    explicit HARDWARE(QObject *parent = 0);
    ~HARDWARE();
    void stm32IsAlive(void);			//检测单片机是否在线
    void MinSheng_Reader_IsAlive(void);	//检测民生卡读头是否在线
    void Identity_Reader_IsAlive(void);	//检测身份证读头是否在线
    void IC_Reader_IsAlive(void);		//检测IC卡读头是否在线

    void resetStm32(void);	// 复位单片机
    void setLogoLed(short status /*LOGOLED_ON / LOGOLED_OFF*/); 		// 控制开关Logo 灯
    void setInfraredPWM(u_int8_t pwm /*min: 0, max: 100, 0 to close*/);	// 控制红外灯板PWM
    void setCtrlDoor(short status /*DOOR_OPEN / DOOR_CLOSE*/);			// 控制开关锁

    Hardware_Info getStm32Info(void);		// A83T获取单片机参数
    bool getLinghtSensor_Status(void);		// A83T获取光敏状态
    u_int8_t getInfraredPWM(void);			// A83T获取当前红外灯板PWM值
    u_int8_t setStm32Info(Hardware_Info *hard_info); // A83T设置单片机参数

signals:
    void sig_LightSensor_Data(QString str); 	// 发送光敏状态信号
    void sig_MinShengCard_Data(QString str);	// 发送民生卡数据信号
    void sig_IdentityCard_Data(QString str);	// 发送身份证数据信号
    void sig_IC_Data(QString str);				// 发送IC卡数据信号

    void sig_stm32IsAlive(bool alive);				// 单片机是否在线信号
    void sig_MinSheng_Reader_IsAlive(bool alive);	// 民生卡读头是否在线信号
    void sig_Identity_Reader_IsAlive(bool alive);	// 身份证读头是否在线信号
    void sig_IC_Reader_IsAlive(bool alive);			// IC卡读头是否在线信号

private slots:
    void recvHardwareData(u_int8_t *data, u_int32_t len);
    void readMyCom(void);
    void checkAlive(void);

private:
    Posix_QextSerialPort *hw_Com;
    QTimer *aliveTimer;
    QTimer *readTimer;
    int aliveCount;
    u_int8_t STM32_ALIVE;
    u_int8_t STM32_INFO_SYNC_FLAG;
    Hardware_Info hw_info;
    QString asciiToQString(u_int8_t *recv_data, u_int32_t recv_len);
    char ConvertHexChar(char ch);
    u_int8_t * minsheng_packet_valid(u_int8_t * pbySrc);		// 校验民生卡数据
    u_int8_t chk_xor8(u_int8_t *pbySrc, u_int32_t dwDataLen);	// 数据逐字节按位异或
    u_int8_t * identity_packet_valid(u_int8_t * pbySrc);		// 校验身份证数据

    u_int8_t Seaial_RecvData_Deal(u_int8_t *pbuff, u_int plen);	// 处理串口接收到的数据
    u_int8_t Light_Sensor_Deal(u_int8_t *pbuff, u_int plen);	// 处理光敏数据
    u_int8_t Minsheng_Card_Deal(u_int8_t *pbuff, u_int plen);	// 处理民生卡数据
    u_int8_t Identity_Card_Deal(u_int8_t *pbuff, u_int plen);	// 处理身份证数据
    u_int8_t IC_Card_Deal(u_int8_t *pbuff, u_int plen);			// 处理IC 卡数据
};

/***************************************************************************
  宏定义
****************************************************************************/
#define HTON2(x)  ((((x) & 0xff) << 8)| (((x) & 0xff00) >> 8))
#define HTON4(x)  ((HTON2((x) & 0xffff) << 16) | HTON2(((x) & 0xffff0000) >> 16))
#define NTOH2(x)  HTON2(x)
#define NTOH4(x)  HTON4(x)

#define LOGOLED_ON 		0x0001
#define LOGOLED_OFF 	0x0000
#define DOOR_OPEN		0x0001
#define DOOR_CLOSE		0x0000

#define IDENTITY_SYNC_HEAD_LEN		5
#define IDENTITY_CMD_LEN			10

// 同步头
#define MINSHENG_SYNC_HEAD      0x80
#define MINSHENG_LAST_DATA_1	0x00
#define MINSHENG_LAST_DATA_2	0x90

/***************************************************************************
  结构体
****************************************************************************/
#pragma pack(1) // 指定1字节对齐
/* identity card text data struct - 256bytes */
typedef struct
{
    unsigned char name[30];			/* 姓名 Unicode编码*/
    unsigned char gender_code[2];	/* 性别类型*/
    unsigned char national_code[4];	/* 民族类型*/
    unsigned char date_of_birth[16];/* 出生日期YYYYMMDD*/
    unsigned char address[70];		/* 住址*/
    unsigned char identity_num[36];	/* 身份证号*/
    unsigned char idIssued[30];		/* 签发机关*/
    unsigned char start_date[16];	/* 有效期起始日期*/
    unsigned char end_date[16];		/* 有效期截止日期*/
    unsigned char res[36];			/* 预留*/
}IDENTITY_Textinfo_PACKET;		/* 身份证文字信息结构体 */

/* receive data struct -1295bytes*/
typedef struct
{
    unsigned char sync_head[5];		/* 帧头 0xAA, 0xAA, 0xAA, 0x96, 0x69 */
    unsigned short data_len;		/* 数据帧的有效长度 */
    unsigned char sw1;				/* 居民身份证返回的状态参数1 */
    unsigned char sw2;				/* 居民身份证返回的状态参数2 */
    unsigned char sw3;				/* SAM_A的操作状态 */
    unsigned char unknow[4];		/* 预留 */
    IDENTITY_Textinfo_PACKET  text_info;	/* 身份证文字信息*/
    unsigned char photo[1024];		/* 身份证相片*/
    unsigned char chk_sum;			/* 数据帧中除帧头和校验和之外的数据逐字节按位异或的结果 */
}IDENTITY_PACKET;	/* 读身份证信息结构体 */

/* identity card user information - 1244bytes */
typedef struct
{
    unsigned char name[30];			/* 姓名 Unicode编码*/
    unsigned char gender_code[2];	/* 性别类型*/
    unsigned char national_code[4];	/* 民族类型*/
    unsigned char date_of_birth[16];/* 出生日期YYYYMMDD*/
    unsigned char address[70];		/* 住址*/
    unsigned char identity_num[36];	/* 身份证号*/
    unsigned char idIssued[30];		/* 签发机关*/
    unsigned char start_date[16];	/* 有效期起始日期*/
    unsigned char end_date[16];		/* 有效期截止日期*/
    unsigned char photo[1024];		/* 身份证相片*/
} IDENTITY_USER_INFO;

/* 民生卡读卡信息结构体*/
typedef struct			/* Total: 95 bytes*/
{
    unsigned char  ccid_cmd;		/* CCID 指令*/
    unsigned int   Abdata_len;		/* Abdata 长度*/
    unsigned char  card_type;		/* 卡类型0: 13.56M卡1: IEC7816 PSAM卡*/
    unsigned char  result_num;		/* 结果号*/
    unsigned char  slot_status;		/* 卡槽状态*/
    unsigned char  slot_err_info;	/* 卡槽错误信息*/
    unsigned char  para_sum;			/* 参数的累加和*/
    unsigned char  card_len;			/* 卡号长度*/
    unsigned char  card[10];			/* 卡号*/
    unsigned char  name[30];			/* 姓名*/
    unsigned char  certificate_type[2];	/* 证件类型*/
    unsigned char  validity_period[4];	/* 证件有效期*/
    unsigned char  identity_num[32];	/* 身份证号*/
    unsigned char  res[6];				/* 剩余字节*/
//卡号长度(1字节)+ 卡号 + 姓名(30字节GBK) + 证件类型(2字节)
//+证件有效期(4字节)//+ 证件号码(32字节GBK）+ 00 04 00 28 90 00
}MINSHENG_Info_PACKET;

/* minsheng card user information - 78 bytes */
typedef struct			/* Total: 78 bytes*/
{
    unsigned char  card[10];			/* 卡号*/
    unsigned char  name[30];			/* 姓名*/
    unsigned char  certificate_type[2];	/* 证件类型*/
    unsigned char  validity_period[4];	/* 证件有效期*/
    unsigned char  identity_num[32];	/* 身份证号*/
//卡号 + 姓名(30字节GBK) + 证件类型(2字节) +证件有效期(4字节) + 证件号码(32字节GBK)
}MINSHENG_USER_INFO;
#pragma pack() // 恢复默认对齐方式

typedef enum A83T_FUNCTION_CODE
{
    FUNCTION_CODE_A83T_RESET_POS = 0x00,			//A83T复位单片机
    FUNCTION_CODE_STM32_IS_ALIVE = 0x01,			//检测单片机是否在线
    FUNCTION_CODE_MINSHENG_READER_ALIVE = 0x02,		//检测民生卡读头是否在线
    FUNCTION_CODE_IDENTITY_READER_ALIVE = 0x03,		//检测身份证读头是否在线
    FUNCTION_CODE_IC_READER_ALIVE = 0x04,			//检测IC卡读头是否在线
    FUNCTION_CODE_STM32_UP_INFO = 0x05,				//STM32上传单片机参数
    FUNCTION_CODE_A83T_SET_STM32_INFO = 0x06,		//A83T设置单片机参数
    FUNCTION_CODE_STM32_OPENDOOR = 0x07,			//STM32上传开门动作
    FUNCTION_CODE_A83T_GET_STM32_INFO = 0x08,		// A83T获取单片机参数
    FUNCTION_CODE_A83T_GET_LIGHT_SENSOR = 0x09,		// A83T获取光敏状态
    FUNCTION_CODE_A83T_GET_INFRARED_PWM = 0x0A,		// A83T获取当前红外灯板PWM值
    FUNCTION_CODE_A83T_GET_DOOR_STATUS = 0x0B,		// A83T获取当前锁状态
    FUNCTION_CODE_A83T_LOGO = 0x20,			//A83T控制LOGO灯
    FUNCTION_CODE_A83T_INFRARED = 0x21,		//A83T控制红外灯板
    FUNCTION_CODE_A83T_LOCK = 0x22,			//A83T控制锁
    FUNCTION_CODE_A83T_LIGHT_SENSOR = 0x23,	//STM32上传光敏状态
    FUNCTION_CODE_A83T_MINSHENG_CARD = 0x24,//STM32上传民生卡信息
    FUNCTION_CODE_A83T_IDENTITY_CARD = 0x25,//STM32上传身份证信息
    FUNCTION_CODE_A83T_IC_CARD = 0x26,		//STM32上传IC卡号
}enum_A83T_FUNCTION_CODE;//与A83T 通信时的功能码定义

#endif // HARDWARE_H
