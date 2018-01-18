#ifndef HARDWARE_COMMON_H
#define HARDWARE_COMMON_H

#include <QObject>

#define Version_LEN		23   	//单片机版本号字符串长度
#define LOGOLED_ON 		0x0001
#define LOGOLED_OFF 	0x0000
#define DOOR_OPEN		0x0001
#define DOOR_CLOSE		0x0000
#define PWM_CLOSE		0x0000

#pragma pack(1) // 指定1字节对齐
typedef struct
{
    char			Version[Version_LEN];		// 版本号
    unsigned int 	Open_Door_Continue_Time; 	//开门持续时间, 单位为毫秒ms
    unsigned char 	USE_MINSHENG_READER;		// 是否使用民生卡读头
    unsigned char 	USE_IDENTITY_READER;		// 是否使用身份证读头
    unsigned char 	USE_IC_READER;				// 是否使用IC卡读头
    unsigned char 	USE_485;					// 是否使用485
    unsigned char 	LIGHT_SENSOR_VALUE;			// 光敏值
    unsigned char 	PWM_VALUE;					// 红外PWM值
} Hardware_Info; // 单片机的参数

typedef struct
{
    unsigned int 	Open_Door_Continue_Time; 	//开门持续时间, 单位为毫秒ms
    unsigned char 	INIT_PWM_VALUE;				// 开机PWM初始值
} Hardware_Set_Info; // 设置单片机参数

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
} IDENTITY_USER_INFO; // 身份证用户信息

/* minsheng card user information - 78 bytes */
typedef struct			/* Total: 78 bytes*/
{
    unsigned char  card[10];			/* 卡号*/
    unsigned char  name[30];			/* 姓名*/
    unsigned char  certificate_type[2];	/* 证件类型*/
    unsigned char  validity_period[4];	/* 证件有效期*/
    unsigned char  identity_num[32];	/* 身份证号*/
//卡号 + 姓名(30字节GBK) + 证件类型(2字节) +证件有效期(4字节) + 证件号码(32字节GBK)
}MINSHENG_USER_INFO; // 民生卡用户信息

#pragma pack() // 恢复默认对齐方式

#endif // HARDWARE_COMMON_H
