#include "Gateway.h"

FlashTypeDef  tLedFlash[LED_NUM]={
	{1,0,2,0},{1,0,2,0},{1,0,2,0},{1,0,2,0},{1,0,2,0},{1,0,2,0}
};

//导出gpio
static void exportGpio(int addr)
{
    FILE * fp =fopen(LED_EXPORT,WRITE);
    if(fp == NULL){
    	 perror("exportGpio failed");
    }else{
        fprintf(fp,"%d",addr);
    }
    fclose(fp);
}

//删除gpio
static void unexportGpio(int addr)
{
    FILE * fp =fopen(LED_UNEXPORT,WRITE);
    if(fp == NULL){
    	 perror("unexportGpio failed");
    }else{
        fprintf(fp,"%d",addr);
    }
    fclose(fp);
}

//设置gpio方向
static void setGpioDirection(int number,char *direction)
{
    char file[128]={0};
    snprintf(file,sizeof(file),LED_DIRECTION,number);
    FILE * fp =fopen(file,WRITE);
    if (fp == NULL){
    	perror("setGpioDirection failed");
    }
    else{
        fprintf(fp,"%s",direction);
    }
    fclose(fp);
}

//设置gpio触发
static void setGpioEdge(int number,char *edge)
{
    char file[128]={0};
    snprintf(file,sizeof(file),LED_EDGE,number);
    FILE * fp =fopen(file,WRITE);
    if (fp == NULL){
    	perror("setGpioEdge failed");
    }
    else{
        fprintf(fp,"%s",edge);
    }
    fclose(fp);
}

//设置gpio电平值
void setGpioValue(int number,int value)
{
	char file[128]={0};
    snprintf(file,sizeof(file),LED_VALUE,number);
	FILE *fp = fopen(file,WRITE);
	if (fp == NULL){
		 perror("setGpioValue failed");
	}
    else{
    	fprintf(fp, "%d", value);
    }
	fclose(fp);
}

//获取gpio的值
int getGpioValue(int number)
{
   char file[128]={0};
   char valueString[3]={0};

   snprintf(file,sizeof(file),LED_VALUE,number);
   int fd = open(file,O_RDONLY);
   if (fd < 0){
       perror("openGpio failed");
       return -1;
   }
   if (read(fd,valueString,3) < 0){
       perror("readGpio failed");
       return -1;
   }
   close(fd);
   return (atoi(valueString));
}

//初始化led
int initLed(void)
{
	printf("initLed ...\n");
	for(int i=0;i<LED_NUM;i++){
		exportGpio(LED_ADDR(i));
		setGpioDirection(LED_ADDR(i),LED_DIRECTION_OUT);
		setGpioValue(LED_ADDR(i),LED_VALUE_OFF);
	}
}

//翻转led
void flashLed(void)
{
	for(int i=0;i<LED_NUM;i++){
		if(tLedFlash[i].enable){
			if(tLedFlash[i].cnt >= tLedFlash[i].cycle * 5)	//半周期
				setGpioValue(LED_ADDR(i),LED_VALUE_ON);
			if(tLedFlash[i].cnt >= tLedFlash[i].cycle * 10){	//全周期清零计数器
				tLedFlash[i].cnt = 0;
				setGpioValue(LED_ADDR(i),LED_VALUE_OFF);
			}
		}
	}
}
