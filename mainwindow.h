#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "posix_qextserialport.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void readMyCom();
    void HandleLineEditclicked();
    void receiveData(QString data);
    void on_SendPWM_Button_clicked();
    void on_OpenDoor_Button_clicked();
    void on_CloseDoor_Button_clicked();
    void on_OpenLogoLed_Button_clicked();
    void on_CloseLogoLed_Button_clicked();

private:
    Ui::MainWindow *ui;
    Posix_QextSerialPort *myCom;
    unsigned char * minsheng_packet_valid(unsigned char * pbySrc); // 校验民生卡数据
    unsigned char chk_xor8(unsigned char *pbySrc, u_int32_t dwDataLen);/* 数据逐字节按位异或*/

    unsigned char Seaial_RecvData_Deal(unsigned char *pbuff, u_int plen);
    unsigned char Minsheng_Card_Deal(unsigned char *pbuff, u_int plen);
    unsigned char Identity_Card_Deal(unsigned char *pbuff, u_int plen);
    unsigned char IC_Card_Deal(unsigned char *pbuff, u_int plen);
};

// 同步头
#define MINSHENG_SYNC_HEAD                         0x80
#define MINSHENG_LAST_DATA_1	0x00
#define MINSHENG_LAST_DATA_2	0x90

/* identity card text data struct - 256bytes */
typedef struct
{
    unsigned char name[30];			/* 姓名 Unicode编码*/
    unsigned char gender_code[2];	/* 性别类型*/
    unsigned char national_code[4];	/* 民族类型*/
    unsigned char date_of_birth[16];	/* 出生日期YYYYMMDD*/
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
    unsigned char unknow[4];			/* 预留 */
    IDENTITY_Textinfo_PACKET  text_info;	/* 身份证文字信息*/
    unsigned char photo[1024];		/* 身份证相片*/
    unsigned char chk_sum;			/* 数据帧中除帧头和校验和之外的数据逐字节按位异或的结果 */
}IDENTITY_PACKET;	/* 读身份证信息结构体 */

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
    unsigned char  identity_num[32];		/* 身份证号*/
    unsigned char  res[6];				/* 剩余字节*/
//卡号长度(1字节)+ 卡号 + 姓名(30字节GBK) + 证件类型(2字节)
//+证件有效期(4字节)//+ 证件号码(32字节GBK）+ 00 04 00 28 90 00
}MINSHENG_Info_PACKET;

typedef enum A83T_FUNCTION_CODE
{
    FUNCTION_CODE_A83T_RESET_POS = 0x00,	//A83T复位单片机
    FUNCTION_CODE_A83T_LOGO = 0x20,			//A83T控制LOGO灯
    FUNCTION_CODE_A83T_INFRARED = 0x21,		//A83T控制红外灯板
    FUNCTION_CODE_A83T_LOCK = 0x22,			//A83T控制锁
    FUNCTION_CODE_A83T_LIGHT_SENSOR = 0x23,	//STM32上传光敏状态
    FUNCTION_CODE_A83T_MINSHENG_CARD = 0x24,//STM32上传民生卡信息
    FUNCTION_CODE_A83T_IDENTITY_CARD = 0x25,//STM32上传身份证信息
    FUNCTION_CODE_A83T_IC_CARD = 0x26,		//STM32上传IC 卡号
}enum_A83T_FUNCTION_CODE;//与A83T 通信时的功能码定义

#endif // MAINWINDOW_H
