#include "ReadFile.h"
#include "Gateway.h"
#define JSMN_HEADER
#include "jsmn.h"

NetworkConfigTypeDef	tNorthNetConfig;		//北向网口配置结构体
NetworkConfigTypeDef	tSouthNetConfig;		//南向网口配置结构体
UartConfigTypeDef 		tUartConfig[RS485_NUM];	//RS485配置结构体
ChannalConfigTypeDef 	tChannalConfig;			//通道配置结构体
RegisterConfigTypeDef 	tRegisterConfig;		//注册包配置结构体
HeartbeatConfigTypeDef 	tHeartbeatConfig;		//心跳包配置结构体
GateWayConfigTypeDef 	tGateWayConfig;			//网关配置结构体

//获取通道设备总数
int getChannalTotalNum(void){
	return	tChannalConfig.totalNum;
}

//解析网络json配置文件
static int jsmnNetFile(NetworkConfigTypeDef *config, char *buff)
{
	jsmn_parser pars;		//jsmn配置
	jsmntok_t token[128];	//最多128个token节点
	jsmn_init(&pars);
	int tokenSize = sizeof(token)/sizeof(token[0]);
	int tokenNum = jsmn_parse(&pars,buff,strlen(buff),token,tokenSize);
	//判断token数量
	if (tokenNum < 0){
		printf("parse JSON failed No:%d\n",tokenNum);
		return -1;
	}
	//判断json是否完整
	if (tokenNum < 1 || token[0].type != JSMN_OBJECT){
		printf("object expected\n");
		return -1;
	}
	//JSON内容解析
	for(int i=0;i<tokenNum;i++){
		//1、netMode
		if(jsoneq(buff, &token[i], "netMode") == 0){
			//1.1、Static
			if(!strncmp(buff + token[i + 1].start,"Static",token[i + 1].end - token[i + 1].start))
				config->netMode = 0;
			//1.2、DHCP
			else if(!strncmp(buff + token[i + 1].start,"DHCP",token[i + 1].end - token[i + 1].start))
				config->netMode = 1;
			//1.3、其他
			else
				config->netMode = 0;
			printf("config->netMode:%d\n",config->netMode);
			i++;
		}
		//2、protocolType
		if(jsoneq(buff,&token[i],"protocolType") == 0){
			//2.1、TCPServer
			if(!strncmp(buff + token[i + 1].start,"TCPServer",token[i + 1].end - token[i + 1].start))
				config->protocolType = 0;
			//2.2、TCPClient
			else if(!strncmp(buff + token[i + 1].start,"TCPClient",token[i + 1].end - token[i + 1].start))
				config->protocolType = 1;
			//2.3、UDPServer
			else if(!strncmp(buff + token[i + 1].start,"UDPServer",token[i + 1].end - token[i + 1].start))
				config->protocolType = 2;
			//2.4、UDPClient
			else if(!strncmp(buff + token[i + 1].start,"UDPClient",token[i + 1].end - token[i + 1].start))
				config->protocolType = 3;
			//2.5、其他
			else
				config->protocolType = 1;
			printf("config->protocolType:%d\n",config->protocolType);
			i++;
		}
		//3、localAddr
		if(jsoneq(buff,&token[i],"localAddr") == 0){
			char ipBuf[IP_STRING_SIZE] = {0};
			memcpy(ipBuf,buff + token[i + 1].start,token[i + 1].end - token[i + 1].start);
			config->localAddr = inet_addr(ipBuf);
			printf("config->localIP:%x\n",htonl(config->localAddr));
			i++;
		}
		//4、localPort
		if(jsoneq(buff,&token[i],"localPort") == 0){
			config->localPort = atoi(buff + token[i + 1].start);
			printf("config->localPort:%d\n",config->localPort);
			i++;
		}
		//5、targetAddr
		if(jsoneq(buff,&token[i],"targetAddr") == 0){
			char ipBuf[IP_STRING_SIZE] = {0};
			memcpy(ipBuf,buff + token[i + 1].start,token[i + 1].end - token[i + 1].start);
			config->targetAddr = inet_addr(ipBuf);
			printf("config->targetAddr:%x\n",htonl(config->targetAddr));
			i++;
		}
		//6、targetPort
		if(jsoneq(buff, &token[i], "targetPort") == 0){
			config->targetPort = atoi(buff + token[i + 1].start);
			printf("config->targetPort:%d\n",config->targetPort);
			i++;
		}
		//7、mask
		if(jsoneq(buff, &token[i],"mask") == 0){
			char ipBuf[IP_STRING_SIZE] = {0};
			memcpy(ipBuf,buff + token[i + 1].start,token[i + 1].end - token[i + 1].start);
			config->mask = inet_addr(ipBuf);
			printf("config->mask:%x\n",htonl(config->mask));
			i++;
		}
		//8、gateway
		if(jsoneq(buff, &token[i], "gateway") == 0){
			char ipBuf[IP_STRING_SIZE] = {0};
			memcpy(ipBuf,buff + token[i + 1].start,token[i + 1].end - token[i + 1].start);
			config->gateway = inet_addr(ipBuf);
			printf("config->gateway:%x\n",htonl(config->gateway));
			i++;
		}
		//9、DNS
		if(jsoneq(buff, &token[i], "DNS") == 0){
			char ipBuf[IP_STRING_SIZE] = {0};
			memcpy(ipBuf,buff + token[i + 1].start,token[i + 1].end - token[i + 1].start);
			config->DNS = inet_addr(ipBuf);
			printf("config->DNS:%x\n",htonl(config->DNS));
			i++;
		}
		//10、linkNumber
		if(jsoneq(buff, &token[i], "linkNumber") == 0){
			printf("linkNumber :%.*s\n", token[i + 1].end - token[i + 1].start,buff + token[i + 1].start);
			i++;
		}
	}
}

//读取北向网口json配置文件
int readNorthNetFile(void)
{
	printf("readNorthNetFile ...\n");
	FILE *fp = NULL;
	char buff[1024] = {0};
	fp = fopen(NORTH_NETWORK_FILE,READ);
	if(NULL == fp){
		perror("fopen NORTH_NETWORK_FILE failed");
		return -1;
	}
	//定位文件末尾
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	rewind(fp);		//重新定位到文件开头
	int len = fread(buff,size,1,fp);
	fclose(fp);
	//对读取文件内容进行json解析
	jsmnNetFile(&tNorthNetConfig,buff);
}

//读取南向网口json配置文件
int readSouthNetFile(void)
{
	printf("readSouthNetFile ...\n");
	FILE *fp = NULL;
	char buff[1024] = {0};
	fp = fopen(SOUTH_NETWORK_FILE,READ);
	if(NULL == fp){
		perror("fopen SOUTH_NETWORK_FILE failed");
		return -1;
	}
	printf("fopen SOUTH_NETWORK_FILE success\n");
	//定位文件末尾
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	rewind(fp);		//重新定位到文件开头
	int len = fread(buff,size,1,fp);
	fclose(fp);
	//对读取文件内容进行json解析
	jsmnNetFile(&tSouthNetConfig,buff);
}

//解析串口json配置文件
static int jsmnUartFile(UartConfigTypeDef *config, char *buff)
{
	jsmn_parser pars;		//jsmn配置
	jsmntok_t token[128];	//最多128个token节点
	jsmn_init(&pars);
	int tokenSize = sizeof(token)/sizeof(token[0]);
	int tokenNum = jsmn_parse(&pars,buff,strlen(buff),token,tokenSize);
	//判断token数量
	if (tokenNum < 0){
		printf("parse JSON failed No:%d\n",tokenNum);
		return -1;
	}
	//判断json是否完整
	if (tokenNum < 1 || token[0].type != JSMN_OBJECT){
		printf("object expected\n");
		return -1;
	}
	//数组成员数量
	for(int i=0;i<tokenNum;i++){
		if(jsoneq(buff,&token[i],"RS485") == 0){
			//判断"RS485"是否为数组
			if (token[i + 1].type != JSMN_ARRAY){
				return -1;
			}
		}
		//RS485数组有3个元素，1个为1个json对象，json对象中有5个元素
		if(token[i].type == JSMN_OBJECT && token[i].size == 5){
			//把数组的对象复制出来再进行解析
			char json[1024] = {0};
			memcpy(json,buff + token[i].start,token[i].end - token[i].start);
			//对元素中json对象进行二次解析
			jsmn_parser ps;		//jsmn配置
			jsmntok_t tk[128];	//最多128个token节点
			jsmn_init(&ps);
			int size = sizeof(tk)/sizeof(tk[0]);
			int num = jsmn_parse(&ps,json,strlen(json),tk,size);
			//判断token数量
			if (num < 0){
				printf("parse JSON failed No:%d\n",num);
				return -1;
			}
			//判断json是否完整
			if (num < 1 || tk[0].type != JSMN_OBJECT){
				printf("object expected\n");
				return -1;
			}
			int addr = 0;		///UartConfigTypeDef结构体数组地址
			for(int j=0;j<num;j++){
				//1、No
				if(jsoneq(json,&tk[j],"No") == 0){
					addr = atoi(json + tk[j + 1].start);
					(config+addr)->uartNumber = addr;
					printf("tUartConfig[%d]:config->uartNumber:%d\n",addr,(config+addr)->uartNumber);
					j++;
				}
				//2、baud
				else if(jsoneq(json,&tk[j],"baud") == 0){
					(config+addr)->baudRate = atoi(json + tk[j + 1].start);
					printf("tUartConfig[%d]:config->baudRate:%d\n",addr,(config+addr)->baudRate);
					j++;
				}
				//3、cheakBit
				else if(jsoneq(json,&tk[j],"cheakBit") == 0){
					(config+addr)->checkBit = atoi(json + tk[j + 1].start);
					printf("tUartConfig[%d]:config->checkBit:%d\n",addr,(config+addr)->checkBit);
					j++;
				}
				//4、length
				else if(jsoneq(json,&tk[j],"length") == 0){
					(config+addr)->dataBit = atoi(json + tk[j + 1].start);
					printf("tUartConfig[%d]:config->dataBit:%d\n",addr,(config+addr)->dataBit);
					j++;
				}
				//5、stopBit
				else if(jsoneq(json,&tk[j],"stopBit") == 0){
					(config+addr)->stopBit = atoi(json + tk[j + 1].start);
					printf("tUartConfig[%d]:config->stopBit:%d\n",addr,(config+addr)->stopBit);
					j++;
				}
			}
		}
	}
}

//读取串口json配置文件
int readUartFile(void)
{
	printf("readUartFile ...\n");
	FILE *fp = NULL;
	char buff[1024] = {0};
	fp = fopen(UART_CONFIG_FILE,READ);
	if(NULL == fp){
		perror("fopen UART_CONFIG_FILE failed");
		return -1;
	}
	printf("fopen UART_CONFIG_FILE success\n");
	//定位文件末尾
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	rewind(fp);		//重新定位到文件开头
	int len = fread(buff,size,1,fp);
	fclose(fp);
	//对读取文件内容进行json解析
	jsmnUartFile(&tUartConfig,buff);
}

//解析通道json配置文件

//解析通道json配置文件
static int jsmnChannalFile(ChannalConfigTypeDef *config,char *buff)
{
	jsmn_parser pars;		//jsmn配置
	jsmntok_t token[256];	//最多256个token节点
	jsmn_init(&pars);
	int tokenSize = sizeof(token)/sizeof(token[0]);
	int tokenNum = jsmn_parse(&pars,buff,strlen(buff),token,tokenSize);
	//判断token数量
	if (tokenNum < 0){
		printf("parse JSON failed No:%d\n",tokenNum);
		return -1;
	}
	//判断json是否完整
	if (tokenNum < 1 || token[0].type != JSMN_OBJECT){
		printf("object expected\n");
		return -1;
	}
	//数组成员数量
	for(int i=0;i<tokenNum;i++){
		if(jsoneq(buff,&token[i],"num") == 0){
			config->totalNum = atoi(buff + token[i + 1].start);
			printf("tChannalConfig:config->totalNum:%d\n",config->totalNum);
			i++;
		}
		else if(jsoneq(buff,&token[i],"config") == 0){
			//判断"RS485"是否为数组
			if (token[i + 1].type != JSMN_ARRAY){
				return -1;
			}
		}
		//"config"数组有10个元素，1个为1个json对象，json对象中有7个元素
		else if(token[i].type == JSMN_OBJECT && token[i].size == 7){
			//把数组的对象复制出来再进行解析
			char json[1024] = {0};
			memcpy(json,buff + token[i].start,token[i].end - token[i].start);
			//对元素中json对象进行二次解析
			jsmn_parser ps;		//jsmn配置
			jsmntok_t tk[128];	//最多128个token节点
			jsmn_init(&ps);
			int size = sizeof(tk)/sizeof(tk[0]);
			int num = jsmn_parse(&ps,json,strlen(json),tk,size);
			//判断token数量
			if (num < 0){
				printf("parse JSON failed No:%d\n",num);
				return -1;
			}
			//判断json是否完整
			if (num < 1 || tk[0].type != JSMN_OBJECT){
				printf("object expected\n");
				return -1;
			}
			int addr = 0;		///ChannalConfigTypeDef结构体数组地址
			for(int j=0;j<num;j++){
				//1、No
				if(jsoneq(json,&tk[j],"No") == 0){
					addr = atoi(json + tk[j + 1].start);
					printf("tChannalConfig.device[%d]\n",addr);
					j++;
				}
				//2、state
				if(jsoneq(json,&tk[j],"state") == 0){
					//2.1、off
					if(!strncmp(json + tk[j + 1].start,"off",tk[j + 1].end - tk[j + 1].start))
						(config->device + addr)->enable= 0;
					//2.2、on
					else if(!strncmp(json + tk[j + 1].start,"on",tk[j + 1].end - tk[j + 1].start))
						(config->device + addr)->enable= 1;
					//1.3、其他
					else
						(config->device + addr)->enable= 0;
					printf("tChannalConfig.device[%d]->enable:%d\n",addr,(config->device + addr)->enable);
					j++;
				}
				//3、type
				if(jsoneq(json,&tk[j],"type") == 0){
					memcpy((config->device + addr)->deviceType,json + tk[j + 1].start,tk[j + 1].end - tk[j + 1].start);
					printf("tChannalConfig.device[%d]->deviceType:%.4s\n",addr,(config->device + addr)->deviceType);
					j++;
				}
				//4、number
				if(jsoneq(json,&tk[j],"number") == 0){
					memcpy((config->device + addr)->deviceNo,json + tk[j + 1].start,tk[j + 1].end - tk[j + 1].start);
					printf("tChannalConfig.device[%d]->deviceNo:%.5s\n",addr,(config->device + addr)->deviceNo);
					j++;
				}
				//5、interFace
				if(jsoneq(json,&tk[j],"interFace") == 0){
					//5.1、net
					if(!strncmp(json + tk[j + 1].start,"net",tk[j + 1].end - tk[j + 1].start))
						(config->device + addr)->portType= 0;
					//5.2、com1
					else if(!strncmp(json + tk[j + 1].start,"com1",tk[j + 1].end - tk[j + 1].start))
						(config->device + addr)->portType= 1;
					//5.3、com2
					else if(!strncmp(json + tk[j + 1].start,"com2",tk[j + 1].end - tk[j + 1].start))
						(config->device + addr)->portType= 2;
					//5.4、com3
					else if(!strncmp(json + tk[j + 1].start,"com3",tk[j + 1].end - tk[j + 1].start))
						(config->device + addr)->portType= 3;
					printf("tChannalConfig.device[%d]->portType:%d\n",addr,(config->device + addr)->portType);
					j++;
				}
				//6、time
				if(jsoneq(json,&tk[j],"time") == 0){
					(config->device + addr)->frequency = atoi(json + tk[j + 1].start);
					printf("tChannalConfig.device[%d]->frequency:%d\n",addr,(config->device + addr)->frequency);
					j++;
				}
				//7、addr
				if(jsoneq(json,&tk[j],"addr") == 0){
					(config->device + addr)->deviceAddr = atoi(json + tk[j + 1].start);
					printf("tChannalConfig.device[%d]->deviceAddr:%d\n",addr,(config->device + addr)->deviceAddr);
					j++;
				}
			}
		}
	}
}

//读取通道json配置文件
int readChannalFile(void)
{
	printf("readChannalFile ...\n");
	FILE *fp = NULL;
	char buff[3072] = {0};
	fp = fopen(CHANNAL_CONFIG_FILE,READ);
	if(NULL == fp){
		perror("fopen CHANNAL_CONFIG_FILE failed");
		return -1;
	}
	printf("fopen CHANNAL_CONFIG_FILE success\n");
	//定位文件末尾
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	rewind(fp);		//重新定位到文件开头
	int len = fread(buff,size,1,fp);
	fclose(fp);
	//对读取文件内容进行json解析
	jsmnChannalFile(&tChannalConfig,buff);
}

//解析注册包json配置文件
static int jsmnRegisterFile(RegisterConfigTypeDef *config,char *buff)
{
	jsmn_parser pars;		//jsmn配置
	jsmntok_t token[64];	//最多64个token节点
	jsmn_init(&pars);
	int tokenSize = sizeof(token)/sizeof(token[0]);
	int tokenNum = jsmn_parse(&pars,buff,strlen(buff),token,tokenSize);
	//判断token数量
	if (tokenNum < 0){
		printf("parse JSON failed No:%d\n",tokenNum);
		return -1;
	}
	//判断json是否完整
	if (tokenNum < 1 || token[0].type != JSMN_OBJECT){
		printf("object expected\n");
		return -1;
	}
	//数组成员数量
	for(int i=0;i<tokenNum;i++){
		//1、state
		if(jsoneq(buff,&token[i],"state") == 0){
			//1.1、off
			if(!strncmp(buff + token[i + 1].start,"off",token[i + 1].end - token[i + 1].start))
				config->enable= 0;
			//1.2、on
			else if(!strncmp(buff + token[i + 1].start,"on",token[i + 1].end - token[i + 1].start))
				config->enable= 1;
			//1.3、其他
			else
				config->enable= 0;
			printf("tRegisterConfig:config->enable:%d\n",config->enable);
			i++;
		}
		//2、type
		if(jsoneq(buff,&token[i],"type") == 0){
			//1.1、ASCII
			if(!strncmp(buff + token[i + 1].start,"ASCII",token[i + 1].end - token[i + 1].start))
				config->type= 0;
			//1.2、HEX
			else if(!strncmp(buff + token[i + 1].start,"HEX",token[i + 1].end - token[i + 1].start))
				config->type= 1;
			//1.3、其他
			else
				config->type= 0;
			printf("tRegisterConfig:config->type:%d\n",config->type);
			i++;
		}
		//3、buff
		if(jsoneq(buff,&token[i],"buff") == 0){
			memcpy(config->buff,buff + token[i + 1].start,token[i + 1].end - token[i + 1].start);
			printf("tRegisterConfig:config->buff:%s\n",config->buff);
			i++;
		}
		//4、length
		if(jsoneq(buff,&token[i],"length") == 0){
			config->len = atoi(buff + token[i + 1].start);
			printf("tRegisterConfig:config->len:%d\n",config->len);
			i++;
		}
	}
}

//读取注册包json配置文件
int readRegisterFile(void)
{
	printf("readRegisterFile ...\n");
	FILE *fp = NULL;
	char buff[1024] = {0};
	fp = fopen(REGISTER_CONFIG_FILE,READ);
	if(NULL == fp){
		perror("fopen REGISTER_CONFIG_FILE failed");
		return -1;
	}
	printf("fopen REGISTER_CONFIG_FILE success\n");
	//定位文件末尾
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	rewind(fp);		//重新定位到文件开头
	int len = fread(buff,size,1,fp);
	fclose(fp);
	//对读取文件内容进行json解析
	jsmnRegisterFile(&tRegisterConfig,buff);
}

//解析心跳包json配置文件
static int jsmnHeartbeatFile(HeartbeatConfigTypeDef *config,char *buff)
{
	jsmn_parser pars;		//jsmn配置
	jsmntok_t token[64];	//最多64个token节点
	jsmn_init(&pars);
	int tokenSize = sizeof(token)/sizeof(token[0]);
	int tokenNum = jsmn_parse(&pars,buff,strlen(buff),token,tokenSize);
	//判断token数量
	if (tokenNum < 0){
		printf("parse JSON failed No:%d\n",tokenNum);
		return -1;
	}
	//判断json是否完整
	if (tokenNum < 1 || token[0].type != JSMN_OBJECT){
		printf("object expected\n");
		return -1;
	}
	//数组成员数量
	for(int i=0;i<tokenNum;i++){
		//1、state
		if(jsoneq(buff,&token[i],"state") == 0){
			//1.1、off
			if(!strncmp(buff + token[i + 1].start,"off",token[i + 1].end - token[i + 1].start))
				config->enable= 0;
			//1.2、on
			else if(!strncmp(buff + token[i + 1].start,"on",token[i + 1].end - token[i + 1].start))
				config->enable= 1;
			//1.3、其他
			else
				config->enable= 0;
			printf("tHeartbeatConfig:config->enable:%d\n",config->enable);
			i++;
		}
		//2、type
		if(jsoneq(buff,&token[i],"type") == 0){
			//1.1、ASCII
			if(!strncmp(buff + token[i + 1].start,"ASCII",token[i + 1].end - token[i + 1].start))
				config->type= 0;
			//1.2、HEX
			else if(!strncmp(buff + token[i + 1].start,"HEX",token[i + 1].end - token[i + 1].start))
				config->type= 1;
			//1.3、其他
			else
				config->type= 0;
			printf("tHeartbeatConfig:config->type:%d\n",config->type);
			i++;
		}
		//3、buff
		if(jsoneq(buff,&token[i],"buff") == 0){
			memcpy(config->buff,buff + token[i + 1].start,token[i + 1].end - token[i + 1].start);
			printf("tHeartbeatConfig:config->buff:%s\n",config->buff);
			i++;
		}
		//4、length
		if(jsoneq(buff,&token[i],"length") == 0){
			config->len = atoi(buff + token[i + 1].start);
			printf("tHeartbeatConfig:config->len:%d\n",config->len);
			i++;
		}
		//5、time
		if(jsoneq(buff,&token[i],"time") == 0){
			config->time = atoi(buff + token[i + 1].start);
			printf("tHeartbeatConfig:config->time:%d\n",config->time);
			i++;
		}
	}
}

//读取心跳包json配置文件
int readHeartbeatFile(void)
{
	printf("readHeartbeatFile ...\n");
	FILE *fp = NULL;
	char buff[1024] = {0};
	fp = fopen(HEARTBEAT_CONFIG_FILE,READ);
	if(NULL == fp){
		perror("fopen HEARTBEAT_CONFIG_FILE failed");
		return -1;
	}
	printf("fopen HEARTBEAT_CONFIG_FILE success\n");
	//定位文件末尾
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	rewind(fp);		//重新定位到文件开头
	int len = fread(buff,size,1,fp);
	fclose(fp);
	//对读取文件内容进行json解析
	jsmnHeartbeatFile(&tHeartbeatConfig,buff);
}
//读取注册包文件

//读取网关json配置文件
static int jsmnGateWayFile(GateWayConfigTypeDef *config,char *buff)
{
	jsmn_parser pars;		//jsmn配置
	jsmntok_t token[64];	//最多64个token节点
	jsmn_init(&pars);
	int tokenSize = sizeof(token)/sizeof(token[0]);
	int tokenNum = jsmn_parse(&pars,buff,strlen(buff),token,tokenSize);
	//判断token数量
	if (tokenNum < 0){
		printf("parse JSON failed No:%d\n",tokenNum);
		return -1;
	}
	//判断json是否完整
	if (tokenNum < 1 || token[0].type != JSMN_OBJECT){
		printf("object expected\n");
		return -1;
	}
	//数组成员数量
	for(int i=0;i<tokenNum;i++){
		//1、type
		if(jsoneq(buff,&token[i],"type") == 0){
			memcpy(config->type,buff + token[i + 1].start,token[i + 1].end - token[i + 1].start);
			printf("tGateWayConfig->type:%.4s\n",config->type);
			i++;
		}
		//2、number
		else if(jsoneq(buff,&token[i],"number") == 0){
			memcpy(config->number,buff + token[i + 1].start,token[i + 1].end - token[i + 1].start);
			printf("tGateWayConfig->number:%.5s\n",config->number);
			i++;
		}
		//3、key
		else if(jsoneq(buff,&token[i],"key") == 0){
			memcpy(config->key,buff + token[i + 1].start,token[i + 1].end - token[i + 1].start);
			printf("tGateWayConfig->key:%.12s\n",config->key);
			i++;
		}
	}
}

//读取网关json配置文件
int readGateWayFile(void)
{
	printf("readGateWayFile ...\n");
	FILE *fp = NULL;
	char buff[1024] = {0};
	fp = fopen(GATEWAY_CONFIG_FILE,READ);
	if(NULL == fp){
		perror("fopen GATEWAY_CONFIG_FILE failed");
		return -1;
	}
	printf("fopen GATEWAY_CONFIG_FILE success\n");
	//定位文件末尾
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	rewind(fp);		//重新定位到文件开头
	int len = fread(buff,size,1,fp);
	fclose(fp);
	//对读取文件内容进行json解析
	jsmnGateWayFile(&tGateWayConfig,buff);
}
