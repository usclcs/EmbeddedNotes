#include "Gateway.h"
#include "Thread.h"
#include <termios.h>

//#define UBUNTU_UART_TEST
#ifndef UBUNTU_UART_TEST
#define RS485_1_NAME 	"/dev/ttyO2"
#define RS485_2_NAME	"/dev/ttyO0"
#define RS485_3_NAME	"/dev/ttyO1"
#else
#define RS485_1_NAME 	"/dev/ttyUSB0"
#define RS485_2_NAME	"/dev/ttyUSB0"
#define RS485_3_NAME	"/dev/ttyUSB0"
#endif


UartComTypeDef tUartCom[RS485_NUM];
fd_set fsRS485[RS485_NUM];
int fdRS485[RS485_NUM];

//获取rs485文件描述符
int getRS485fd(unsigned char n){
	if(n < RS485_NUM)
		return fdRS485[n];
	else
		return -1;
}

//初始化uart
static int initUart(int fd,int baud,char databits,char parity,char stopbits)
{
	struct termios newConifg, oldConifg;
	int speed = 0;	//波特率
	//获取当前uart配置
	if(tcgetattr(fd, &oldConifg)){
		perror("tcgetttr failed");
		return -1;
	}
	//设置终端默认属性
	newConifg = oldConifg;
    cfmakeraw(&newConifg);
    newConifg.c_cflag &= ~CSIZE;
    //设置波特率
	switch(baud){
		case 2400:
			speed = B2400;
		break;
		case 4800:
			speed = B4800;
			break;
		case 9600:
			speed = B9600;
			break;
		case 19200:
			speed = B19200;
			break;
		case 38400:
			speed = B38400;
			break;
		case 57600:
			speed = B57600;
			break;
		default:
		case 115200:
			speed = B115200;
			break;
	}
	//设置输入波特率
	cfsetispeed(&newConifg, speed);
	//设置输出波特率
	cfsetospeed(&newConifg, speed);
	//设置数据长度
	switch(databits){
		case 5:
			newConifg.c_cflag &= ~CSIZE; //屏蔽其它标志位
			newConifg.c_cflag |= CS5;
			break;
		case 6:
			newConifg.c_cflag &= ~CSIZE; //屏蔽其它标志位
			newConifg.c_cflag |= CS6;
			break;
		case 7:
			newConifg.c_cflag &= ~CSIZE; //屏蔽其它标志位
			newConifg.c_cflag |= CS7;
			break;
		default:
		case 8:
			newConifg.c_cflag &= ~CSIZE; //屏蔽其它标志位
			newConifg.c_cflag |= CS8;
			break;
	}
    //设置奇偶校验位
	switch(parity){
		default:
		case 0:	//无校验
			newConifg.c_cflag &= ~PARENB;
			newConifg.c_iflag &= ~INPCK;
			break;
		case 1: //奇校验
			newConifg.c_cflag |= (PARODD | PARENB);
			newConifg.c_iflag |= INPCK;
			break;
		case 2: //偶校验
			newConifg.c_cflag |=  PARENB;
			newConifg.c_cflag &= ~PARODD;
			newConifg.c_iflag |= INPCK;
			break;
	}
	//设置停止位
	switch(stopbits){
		default:
		case 1:
			newConifg.c_cflag &= ~CSTOPB;
			break;
		case 2:
			newConifg.c_cflag |= CSTOPB;
			break;
	}
	//设置等待时间和最小接收字符
	newConifg.c_cc[VTIME] = 1; 	//读取一个字符等待1*(1/10)s
	newConifg.c_cc[VMIN] = 1; 	//读取字符的最少个数为1
	//处理未接收字符
	tcflush(fd, TCIFLUSH); 		//溢出数据可以接收，但不读
	if((tcsetattr(fd, TCSANOW, &newConifg))){
		perror("tcsetattr failed");
		return -1;
	}
	return 0;
}

//设置rs485配置
int setRS485Config(void)
{
	printf("setNetConfig ...\n");
	UartConfigTypeDef *config = &tUartConfig;
	//这里分开写是因为RS485接口的序号顺序与uart的接口顺序不同
	fdRS485[0] = open(RS485_1_NAME, O_RDWR | O_NOCTTY);
	if(fdRS485[0] < 0){
		perror("RS485_1 Open failed");
		return -1;
	}else{
		printf("RS485_1 Open success\n");
	}
	fdRS485[1] = open(RS485_2_NAME, O_RDWR | O_NOCTTY);
	if(fdRS485[1] < 0){
		printf("RS485_2 Open failed\n");
		return -1;
	}else{
		printf("RS485_2 Open success\n");
	}
	fdRS485[2] = open(RS485_3_NAME, O_RDWR | O_NOCTTY);
	if(fdRS485[2] < 0){
		printf("RS485_3 Open failed\n");
		return -1;
	}else{
		printf("RS485_3 Open success\n");
	}
	for(int i=0;i<RS485_NUM;i++){
		int retval = initUart(fdRS485[i],(config+i)->baudRate,(config+i)->dataBit,(config+i)->checkBit,(config+i)->stopBit);
		if(retval < 0){
			printf("RS485_%d init failed\n",i+1);
			return -1;
		}else{
			printf("RS485_%d init success\n",i+1);
		}
	}
	return 0;
}

//串口发送
int uartSend(int fd, unsigned char *buff, int len)
{
	int ret = write(fd, buff, len);
	if(ret == len){
		return ret;
	}else{
		tcflush(fd, TCOFLUSH); 		//TCOFLUSH刷新写入的数据但不传送
		return -1;
	}
}

//串口接收
int uartRecv(int fd,fd_set fs,unsigned char *buff,int len,int boud)
{
	int ret, fsRet;
	struct timeval tv;
	FD_ZERO(&fs); 		//清空集合
	FD_SET(fd, &fs); 	//将一个给定的文件描述符加入集合之中
	tv.tv_sec = 4;
	tv.tv_usec = 0;		//监听时间
	//使用select实现串口的多路通信
	fsRet = select(fd + 1, &fs, NULL, NULL, &tv); //如果select返回值大于0，说明文件描述符发生了变化
	if(fsRet){
		if(FD_ISSET(fd, &fs)) {
			int time = 10 + 1.0 * UART_MAX_RECV_LEN * 1000 * 10 / (boud);
					usleep(time*1000);	//延时一段时间
					ret = read(fd, buff, len);
				}
//		printf("uartRecv read len %d\r\n",len);
		return ret;
	}else{
		tcflush(fd, TCIFLUSH);
		return 0;
	}
}

//当时在调试RS485接口时出了问题，所以分别写了3个RS485接收线程函数
//RS485-1接收线程
void *thread1RS485Recv(void *arg)
{
	int len = 0;
	unsigned char buf[1024]={0};
	UartComTypeDef *comm = &tUartCom;
	UartConfigTypeDef *config = &tUartConfig;
	while(1){
		//串口开始接收，使能判断
		if(1){
			memset(buf,0,sizeof(buf));
			len = uartRecv(fdRS485[0],fsRS485[0],buf,UART_MAX_RECV_LEN,config->baudRate);
			if(len > 0){
				comm->recvEndSign = 1;
				comm->recvLen = len;
				memset(comm->recvBuff,0,sizeof(comm->recvBuff));
				memcpy(comm->recvBuff,buf,len);
			}
		}
	}
}
//RS485-2接收线程
void *thread2RS485Recv(void *arg)
{
	int len = 0;
	unsigned char buf[1024]={0};
	UartComTypeDef *comm = &tUartCom[1];
	UartConfigTypeDef *config = &tUartConfig[1];
	while(1){
		//串口开始接收,使能判断
		if(1){
			memset(buf,0,sizeof(buf));
			len = uartRecv(fdRS485[1],fsRS485[1],buf,UART_MAX_RECV_LEN,config->baudRate);
			if(len > 0){
				comm->recvEndSign = 1;
				comm->recvLen = len;
				memset(comm->recvBuff,0,sizeof(comm->recvBuff));
				memcpy(comm->recvBuff,buf,len);
			}
		}
	}
}
//RS485-3接收线程
void *thread3RS485Recv(void *arg)
{
	int len = 0;
	unsigned char buf[1024]={0};
	UartComTypeDef *comm = &tUartCom[2];
	UartConfigTypeDef *config = &tUartConfig[2];
	while(1){
		//串口开始接收,使能判断
		if(1){
			memset(buf,0,sizeof(buf));
			len = uartRecv(fdRS485[2],fsRS485[2],buf,UART_MAX_RECV_LEN,config->baudRate);
			if(len > 0){
				comm->recvEndSign = 1;
				comm->recvLen = len;
				memset(comm->recvBuff,0,sizeof(comm->recvBuff));
				memcpy(comm->recvBuff,buf,len);
			}
		}
	}
}
