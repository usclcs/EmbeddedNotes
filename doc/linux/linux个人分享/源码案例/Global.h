#ifndef _GATEWAY_H
#define _GATEWAY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

//版本号
#define GATEWAY_VERSION		"1.1.001"

//标准IO文件操作
#define	WRITE				"w"				//标准文件写
#define	READ				"r"				//标准文件读

//数量
#define RS485_NUM				3			//RS485数量
#define LED_NUM					6			//LED数量
#define ADC_NUM					4			//ADC数量

//通道最大值
#define CHANNAL_MAX  			10			//通道最大数量
#define LISTEN_MAX				20			//select最大数量

//缓存最大长度
#define REGISTER_MAX_SIZE		1024		//注册包最大长度
#define HEARTBEAT_MAX_SIZE		128			//心跳包最大长度
#define UART_MAX_RECV_LEN		128			//串口最大接收长度

//缓存大小
#define REPORT_BUFF_SIZE		2048		//终端报告缓存大小
#define COMMAND_BUFF_SIZE		1024		//设备指令缓存大小
#define READ_BUFF_SIZE			1024		//文件读取缓存大小
#define IP_STRING_SIZE			20			//IP字符串大小
#define IP_ARRAY_SIZE			4			//IP数组大小

//类型编号
#define DEVICE_TYPE_SIZE		4			//设备类型大小
#define DEVICE_NO_SIZE			5			//设备编号大小
#define DEVICE_KEY_SIZE			12			//设备密钥大小

//Json浮点打印精度
#define	CJSON_DOUBLE_2			100
#define	CJSON_DOUBLE_1			10

//程序指针号数量
#define PROGRAM_PTR_NUM			200

//文件路径
#define CONFIG_FILE_PATH   	"/home/waygate/inifile/"
//文件名
#define NORTH_NETWORK_FILE				CONFIG_FILE_PATH"NorthNet.json"
#define SOUTH_NETWORK_FILE				CONFIG_FILE_PATH"SouthNet.json"
#define UART_CONFIG_FILE				CONFIG_FILE_PATH"Uart.json"
#define	CHANNAL_CONFIG_FILE				CONFIG_FILE_PATH"Channal.json"
#define REGISTER_CONFIG_FILE			CONFIG_FILE_PATH"Register.json"
#define HEARTBEAT_CONFIG_FILE			CONFIG_FILE_PATH"Heartbeat.json"
#define GATEWAY_CONFIG_FILE				CONFIG_FILE_PATH"GateWay.json"
//LED
#define LED_PATH		"/sys/class/gpio/"
#define LED_EXPORT		LED_PATH"export"
#define LED_UNEXPORT	LED_PATH"unexport"
#define LED_DIRECTION	LED_PATH"gpio%d/direction"
#define LED_VALUE		LED_PATH"gpio%d/value"
#define LED_EDGE		LED_PATH"gpio%d/edge"
#define LED_ACTIVE_LOW	LED_PATH"gpio%d/active_low"
#define LED_ADDR(n)			75+n
#define LED_DIRECTION_OUT	"out"		//输出
#define LED_DIRECTION_IN	"in"		//输入
#define LED_VALUE_ON		0			//亮
#define LED_VALUE_OFF		1			//灭
#define LED_EDGE_NONE		"none"		//非中断引脚
#define LED_EDGE_RISING		"rising"	//上升沿触发
#define LED_EDGE_FALING		"falling"	//下降沿触发
#define LED_EDGE_BOTH		"both"		//边沿触发


//线程优先级
#define NORTH_RECV_PRIOR	59
#define SOUTH_RECV_PRIOR	58
//串口接收线程优先级范围57-55
#define	SOUTH_UART_PRIOR(n)	57-n
#define SOUTH_UART1_PRIOR	57
#define SOUTH_UART2_PRIOR	56
#define SOUTH_UART3_PRIOR	55
//通道任务线程优先级范围49-40
#define CHANNAL_TASK_PRIOR(n)		49-n


//声明
typedef 	FILE 	*FILE_PTR;
typedef unsigned char 	uint8;
typedef unsigned short 	uint16;
typedef unsigned int 	uint32;
typedef volatile unsigned short vuint16;

//NET
typedef struct{
	char netMode;							//网络模式
	char protocolType;						//协议类型
	unsigned short localPort;				//本地端口
	unsigned short targetPort;				//目标端口
	unsigned int localAddr;					//本地地址
	unsigned int targetAddr;				//目标地址
	unsigned int mask;						//子网掩码
	unsigned int gateway;					//网关
	unsigned int DNS;						//DNS
	char linkNumber;						//连接编号	(用于服务端记录客户端连接编号)
}NetworkConfigTypeDef;
NetworkConfigTypeDef	tNorthNetConfig;
NetworkConfigTypeDef	tSouthNetConfig;

typedef struct{
	int fd;
	struct sockaddr_in addr;
	char recvBuff[1024];
	char recvEndSign;
}SocketParaTypeDef;

int setNetConfig(void);
//UART
typedef struct{
	unsigned short uartNumber;		//串口编号
	unsigned int   baudRate;		//波特率
	unsigned short checkBit;		//校验位
	unsigned short dataBit;			//字长
	unsigned short stopBit;			//停止位
}UartConfigTypeDef;
UartConfigTypeDef tUartConfig[RS485_NUM];

//CHANNAL
typedef struct{
	char enable;  	   							//查询使能
	char deviceType[DEVICE_TYPE_SIZE];			//设备类型
	char deviceNo[DEVICE_NO_SIZE];				//设备编号
	char portType;								//通讯接口类型
	unsigned int frequency;						//数据采集频率
	unsigned int deviceAddr;					//设备协议地址
}DeviceConfigTypeDef;
typedef struct{
	unsigned int totalNum;						//当前通道总数
	DeviceConfigTypeDef device[CHANNAL_MAX];	//通道设备配置
}ChannalConfigTypeDef;
ChannalConfigTypeDef tChannalConfig;

typedef struct{
	unsigned char programPoint;			//函数指针
}ChannalTaskTypeDef;
ChannalTaskTypeDef tChannalTask[CHANNAL_MAX];

int getChannalTotalNum(void);
void setProgramPoint(void);
void setChannalConfig(void);

pthread_mutex_t uartChannalLock[CHANNAL_MAX];

//REGISTER
typedef struct{
	char enable;							//使能
	char type;								//数据类型：ASCII,HEX
	unsigned char buff[REGISTER_MAX_SIZE];	//缓存
	unsigned short len;						//长度
}RegisterConfigTypeDef;
RegisterConfigTypeDef tRegisterConfig;

//HEARTBEAT
typedef struct{
	char	enable;							//使能
	char 	type;							//数据类型：ASCII,HEX
	unsigned char buff[HEARTBEAT_MAX_SIZE];	//缓存
	unsigned short len;						//长度
	unsigned int time;						//心跳时间
}HeartbeatConfigTypeDef;
HeartbeatConfigTypeDef tHeartbeatConfig;

//GATEWAY
typedef struct{
	char type[DEVICE_TYPE_SIZE];		//类型
	char number[DEVICE_NO_SIZE];		//编号
	char key[DEVICE_KEY_SIZE];			//密钥
}GateWayConfigTypeDef;
GateWayConfigTypeDef tGateWayConfig;

//READFILE
int readNorthNetFile(void);		//读取北向网口信息
int readSouthNetFile(void);		//读取南向网口信息
int readUartFile(void);			//读取串口信息
int readChannalFile(void);		//读取通道信息
int readRegisterFile(void);		//读取注册包信息
int readHeartbeatFile(void);	//读取心跳包信息
int readGateWayFile(void);		//读取网关信息

//THREAD
int createNorthNetThread(NetworkConfigTypeDef *config);
int createSouthNetThread(NetworkConfigTypeDef *config);
int createUartThread(void);
int creatChannalThread(unsigned char n);
void *threadTcpServerNorth(void *arg);
void *threadTcpClientNorth(void *arg);
void *childThreadTcpServerNorth(void *arg);
void *threadUdpClientSouth(void *arg);

int sendBuffUdpClientSouth(const char *src,const int len);
int recvBuffUdpClientSouth(char dst[1024]);
//协议相关
extern int deviceConfigReply(void);
extern int channalConfigReply(void);
extern int	northNetReply(void);
extern int southNetReply(void);
extern int southUartReply(void);
extern int configRecvReport(void);
extern int protocoErrorlReport(void);
extern char reportBuff[REPORT_BUFF_SIZE];
extern int northProtocolCheak(const unsigned char *data);
extern int northProtocolDecode(void);
//通道任务相关
int pressureProcces(unsigned char n);
int tempHumiProcces(unsigned char n);
int adc1Procces(unsigned char n);
int adc2Procces(unsigned char n);
int adc3Procces(unsigned char n);
int adc4Procces(unsigned char n);
int GWSTempHumiProcces(unsigned char n);


int deviceErrorUpload(DeviceConfigTypeDef *config);
int dataErrorUpload(DeviceConfigTypeDef *config);

typedef struct{
    int fd;
    int num;
}ConnectConfigTypeDef;

//LED
typedef struct{
	char enable;	//使能标志
	int cnt;		//计时器
	int cycle;		//周期时间
	int duration;	//持续次数
}FlashTypeDef;
FlashTypeDef  tLedFlash[LED_NUM];

int initLed(void);
int getGpioValue(int n);
void setGpioValue(int n, int value);
void flashLed(void);

//UART
typedef struct{
	unsigned char recvBuff[1024];		//接收缓存
	unsigned char sendBuff[1024];		//发送缓存
	unsigned short recvLen;				//接收长度
	unsigned short sendLen;				//发送长度
	unsigned short recvDivideTime;		//接收分隔时间
	unsigned short sendWaitTime;		//发送等待时间
	unsigned char recvEndSign;			//接收结束标志
	unsigned char sendEndSign;			//发送结束标志
}UartComTypeDef;
UartComTypeDef tUartCom[RS485_NUM];

int getRS485fd(unsigned char n);	//获取RS485文件描述符
int setRS485Config(void);	//RS485参数设置
int uartSend(int fd, unsigned char *buff, int len);		//串口发送
int uartRecv(int fd, fd_set fs,unsigned char *buff,int len,int boud);	//串口接收
void *thread1RS485Recv(void *arg);	//RS485-1接收线程
void *thread2RS485Recv(void *arg);	//RS485-2接收线程
void *thread3RS485Recv(void *arg);	//RS485-3接收线程


//QUEUE
#define SYN_MAX_BUF     		100
#define MAX_COMM_BUF_LEN        2048
typedef struct{
    unsigned short front;		//队列头
    unsigned short rear;		//队列尾
    unsigned short num;
    char data_buf[SYN_MAX_BUF][MAX_COMM_BUF_LEN];	//缓存
}QueueComTypeDef;
QueueComTypeDef tChannalQueue;
short PushQueue(QueueComTypeDef *p_Queue, unsigned char recvbuf[],unsigned short recvlen);
unsigned char PopQueue(QueueComTypeDef *p_Queue, unsigned char recvbuf[]);
unsigned char ClearQueue(QueueComTypeDef *p_Queue);

//TIME
uint32 GWSTempHumiTime;
#endif
