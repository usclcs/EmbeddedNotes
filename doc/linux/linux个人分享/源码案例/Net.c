#include "Gateway.h"
#include "CommQueue.h"
#include "NetApp.h"

#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/route.h>
#include <sys/ioctl.h>

#define SOCKET_RECV_LEN			2048
#define SOCKET_SEND_LEN			2048

QueueComTypeDef tChannalQueue;

static char registerRecord = 0;

//获取当前网关参数
int	getGateway(const int ethNo, unsigned char gw[4])
{
	FILE *fp = fopen("/proc/net/route","r");
	if(NULL == fp){
		printf("getGateway open file:/proc/net/route failure\n");
		return -1;
	}
	char ifname[IFNAMSIZ]={0};	//网卡名称
	snprintf(ifname,IFNAMSIZ,"eth%d",ethNo);
	char buf[256] = {0};
	uint32 gateway = 0;
	while(fgets(buf,255,fp)){
		if (strstr(buf,ifname)){
			sscanf(buf,"%*s%*x%X",&gateway);
			if (gateway){
				char addr[16]={0};
				snprintf(addr,16,"%s",inet_ntoa(*(struct in_addr*)&gateway));
				printf("eth%d:gateway:%08X,%s\n",ethNo,gateway,addr);
				sscanf(addr,"%hhd.%hhd.%hhd.%hhd", &gw[0],&gw[1],&gw[2],&gw[3]);
				break;
			}
		}
	}
	fclose(fp);
	return gateway>0?0:-1;
}

//设置网络参数
static int setNetWork(int ethNo,NetworkConfigTypeDef *config)
{
	printf("setNetWork eth%d ...\n",ethNo);
	struct ifreq ifr;	//ifconfig信息
	struct rtentry rt;	//route信息
	in_addr_t value;
	//创建socket
	int fd = socket(AF_INET,SOCK_DGRAM,0);
	if (fd < 0){
		perror("create socket failed");
		return -1;
	}
	//ifconfig信息
	memset(&ifr,0,sizeof(ifr));
	char ifname[IF_NAMESIZE] = {0};
	sprintf(ifname,"eth%d",ethNo);
	strcpy(ifr.ifr_name,ifname);
	ifr.ifr_addr.sa_family = AF_INET;
	//sockaddr - sockaddr_in
	//sa_data[0]-2 sin_port	sa_data[2]-4 sin_addr	sa_data[6]-8 sin_zero
	//addr检查

	//设置ip参数
	value = config->localAddr;
	memcpy(&ifr.ifr_addr.sa_data[2],&value,4);
	int retval = ioctl(fd,SIOCSIFADDR,&ifr);
	if (retval < 0){
		perror("set addr failed");
		close(fd);
		return -1;
	}
	//mask检查

	//设置mask参数
	value = config->mask;
	memcpy(&ifr.ifr_addr.sa_data[2],&value,4);
	retval = ioctl(fd,SIOCSIFNETMASK,&ifr);
	if (retval < 0){
		perror("set mask failed");
		close(fd);
		return -1;
	}
	//gateway检查

	//route信息
	memset(&rt,0,sizeof(rt));
	rt.rt_dst.sa_family = AF_INET;
	rt.rt_gateway.sa_family = AF_INET;
	value = config->gateway;
	memcpy(&rt.rt_gateway.sa_data[2],&value,4);
	rt.rt_genmask.sa_family = AF_INET;
	rt.rt_flags = RTF_GATEWAY;
	//获取当前gateway信息
	unsigned char gateWay[4] = {0};
	retval = getGateway(ethNo,gateWay);
	//判断网关参数是否修改
	if (!retval && ! memcmp(gateWay,&value,4)){
		printf("%s gateway not changed\n",ifname);
		close(fd);
		return 0;
	}
	//gateway配置信息与当前信息不同
	if(!retval){
		//复制之前gateway信息
		memcpy(&rt.rt_gateway.sa_data[2],gateWay,4);
		ioctl(fd,SIOCDELRT,&rt);
	}
	//设置gatway参数
	memcpy(&rt.rt_gateway.sa_data[2],&value,4);
	if (ioctl(fd,SIOCADDRT,&rt) < 0){
		perror("set gateway failed");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

//设置网络参数
int setNetConfig(void)
{
	printf("setNetConfig ...\n");
	//北向网口eth0，南向网口eth1
    setNetWork(0,&tNorthNetConfig);
    setNetWork(1,&tSouthNetConfig);
}

//注册包上传
int registerUpload(int fd,int flag,unsigned char *src,unsigned short len)
{
	int nsend = 0;
	RegisterConfigTypeDef *p = &tRegisterConfig;
	if(p->enable){
		if(!registerRecord){
			registerRecord = 1;
			memset(src,0,sizeof(src));
			memcpy(src,p->buff,p->len);
			len = p->len;
			nsend = send(fd,src,len,MSG_NOSIGNAL);
			usleep(10*1000);	//延时10ms避免粘包
			if(nsend < 1){
				close(fd);
				flag = 0;
				return 0;
			}
		}
	}
	return 1;
}

//心跳包上传
int heartbeatUpload(int fd,int flag,unsigned char *src,unsigned short len)
{
	int nsend = 0;
	static unsigned int recordTime = 0;
	HeartbeatConfigTypeDef *p =&tHeartbeatConfig;
	unsigned int currentTime = time((time_t*)NULL);
	if((currentTime- recordTime) >= (p->time)){
		recordTime = currentTime;
		if(p->enable){
			memset(src,0,sizeof(src));
			memcpy(src,p->buff,p->len);
			len = p->len;
			nsend = send(fd,src,len,MSG_NOSIGNAL);
			usleep(10*1000);	//延时10ms避免粘包
			if(nsend < 1){
				close(fd);
				flag = 0;
				return 0;
			}
		}
	}
	return 1;
}

//通道数据上传
int channalDataUpload(int fd,int flag,unsigned char *src,unsigned short len)
{
	memset(src,0,len);
	if (PopQueue(&tChannalQueue, src) == 1){
		int nsend = send(fd,src,strlen((char*)src),MSG_NOSIGNAL);
		usleep(10*1000);	//延时10ms避免粘包
		if(nsend < 1){
			close(fd);
			flag = 0;
			return 0;
		}
	}
	return 1;
}

//北向协议回复
//int	northProtocolReply(int fd,int flag,unsigned char *src)
//{
//    if(northProtocolCheak(src)){
//    	northProtocolDecode();
//    }else{
//    	protocoErrorlReport();
//    }
//    int nsend = send(fd,reportBuff,strlen(reportBuff),MSG_NOSIGNAL);
//    memset(src,0,sizeof(src));
//    if(nsend < 1){
//    	close(fd);
//		flag = 0;
//		return 0;
//    }
//    return 1;
//}

//网络接收Accept
static int Accept(int fd, struct sockaddr *addr, socklen_t *len)
{
    int retval;
again:
    if ((retval = accept(fd,addr,len)) < 0){
        if ((errno == ECONNABORTED) || (errno == EINTR)) {
            goto again;
        }else{
            printf("accept failed\n");
        }
    }
    return retval;
}

//北向网口服务端线程
void *threadTcpServerNorth(void *arg)
{
    int listenfd = -1, connectfd = -1;	//监听描述符，连接描述符
    int reUseAddr = -1;			//打开或关闭地址复用功能参数
    pthread_t threadID;			//子线程id
    ConnectConfigTypeDef tClient;	//连接配置
	struct sockaddr_in serverAddr,clientAddr;	//服务端，连接的客户端socket参数
	NetworkConfigTypeDef *config = &tNorthNetConfig;	//北向网口配置

	
	//创建套接字
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    	perror("create socket failed");
        return -1;
    }
    //本地ip配置
    memset(&serverAddr,0, sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = (in_addr_t)config->localAddr;
    serverAddr.sin_port = htons(10086);
    //设置socket参数
    if (setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&reUseAddr,sizeof(reUseAddr)) < 0){
    	perror("setsockopt socket failed");
        close(listenfd);
        return -1;
    }
    //绑定socket
    if (bind(listenfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr)) < 0){
    	perror("bind socket failed");
        close(listenfd);
        return -1;
    }
    //监听套接字
    int optval = listen(listenfd,LISTEN_MAX);
    socklen_t clientAddrlen = sizeof(clientAddr);
    for(;;){
    	clientAddrlen = sizeof(clientAddr);
        connectfd = Accept(listenfd,(struct sockaddr *)&clientAddr,&clientAddrlen);
        if (connectfd < 0){
            close(listenfd);
            listenfd = -1;
            perror("north TcpServer accept failed");
            continue;
        }else{
        	//新建连接线程
        	tClient.num = 0;
        	tClient.fd = connectfd;
            pthread_create(&threadID,NULL,childThreadTcpServerNorth,&tClient);
        }
    }
}

//北向网口服务端子线程
void *childThreadTcpServerNorth(void *arg)
{
    ConnectConfigTypeDef client = *(ConnectConfigTypeDef*)arg;	//连接配置
    char sendBuf[SOCKET_SEND_LEN] = {0},recvBuf[SOCKET_RECV_LEN]={0};
    int sendLen = 0,recvLen = 0;	//收发缓存，长度
    fd_set fs;		//句柄配置
    struct timeval tv;		//select监听时间

    while(1){
        //2、监听回复
        tv.tv_sec = 4;
        tv.tv_usec = 0;
        FD_ZERO(&fs);
        FD_SET(client.fd, &fs);
        int retval = select(client.fd + 1, &fs, NULL, NULL, &tv);
        if(retval < 0){
        	//监听错误
        	close(client.fd);
        	 perror("child Thread TcpServer North select failed");
        }else if(!retval){
        	//其他
        }else{
        	//有接收
            if(FD_ISSET(client.fd,&fs)){
            	recvLen = recv(client.fd,recvBuf,255, MSG_NOSIGNAL);
                if(recvLen <= 0){
                	//接收错误
                     close(client.fd);
                     perror("close child Thread TcpServer North recv");
                     break;
                 }
                printf("recv %s\r\nlen=%d\r\n",recvBuf,recvLen);
                //接收回复
                send(client.fd,recvBuf,strlen(recvBuf),MSG_NOSIGNAL);
                usleep(1000);
            }
        }
    }
}

//北向网口客户端线程
void *threadTcpClientNorth(void *arg)
{
    int connectRecord = 0;		//连接记录
    int connectfd = -1;		//连接文件描述符
    struct sockaddr_in  clientAddr;		//客户端socket参数
    struct timeval tv;		//监听时间
    fd_set fs;		//select参数
    unsigned char sendBuf[SOCKET_SEND_LEN] = {0},recvBuf[SOCKET_RECV_LEN] = {0};
    int sendLen = 0,recvLen = 0;	//收发缓存，长度
    int fdState = 0,retval = 0;	//文件描述符状态 函数返回retval
	NetworkConfigTypeDef *config = &tNorthNetConfig;	//北向网口配置

    while(1){
        if(0 == connectRecord){
        	//创建套接字
            if ((connectfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
            	perror("create socket failed");
                return -1;
            }
            //配置ip参数
            memset(&clientAddr,0,sizeof(struct sockaddr_in));
            clientAddr.sin_family = AF_INET;
            clientAddr.sin_addr.s_addr = (in_addr_t)config->targetAddr;
            clientAddr.sin_port = htons(config->targetPort);
            //文件描述符状态
            fdState = fcntl(connectfd,F_GETFL,0);
            fdState &= ~O_NONBLOCK;
            fcntl(connectfd,F_SETFL,fdState);
            //连接ip
            retval = connect(connectfd, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
            if (retval < 0) {
                close(connectfd);
                //无法连接到目的ip
                perror("disconnect socket");
                connectfd = -1;
                sleep(1);
                continue;
             }else{
            	 connectRecord = 1;
            	 printf("connect socket success\n");
            }
        }else{
        	//0、注册机制
        	if(!registerUpload(connectfd,connectRecord,sendBuf,sendLen)){
        		printf("registerUpload failed\r\n");
        	}
			//1、保活机制
			if(!heartbeatUpload(connectfd,connectRecord,sendBuf,sendLen)){
				printf("heartbeatUpload failed\r\n");
			}
			//2、主动上传
			if(!channalDataUpload(connectfd,connectRecord,sendBuf,sizeof(sendBuf))){
				printf("channalDataUpload failed\r\n");
			}
			//select参数设置
            tv.tv_sec = 0;
            tv.tv_usec = 40*1000;	//指令监听40ms
            FD_ZERO(&fs);
            FD_SET(connectfd, &fs);
            retval = select(connectfd + 1,&fs,NULL,NULL,&tv);
            if(retval < 0){
            	//接收异常
            	perror("connectfd + 1 select");
                close(connectfd);
                //清除标志参数
              	registerRecord = 0;
              	connectRecord = 0;
              	ClearQueue(&tChannalQueue);
                continue;
            }else if(!retval){
            	//无接收
            }else{
            	//接收成功
                if(FD_ISSET(connectfd,&fs)){
                	recvLen = recv(connectfd,recvBuf,1024, MSG_NOSIGNAL);
                    if(recvLen == 0){
                        close(connectfd);
                        continue;
                    }
                    //3、监听指令回复
//                  northProtocolReply(connectfd,connectRecord,recvBuf);
                }
            }
        }
    }
}

//北向网口UDP客户端线程
//void *thread_UdpClient_North(void *arg)
//{
//    int  cliconnfd = -1,ret,flags;
//    int connectflag = 0;
//    struct sockaddr_in  cliaddr;
//    struct timeval tv;
//    fd_set fs_read;
//    char send_buf[1024] = {0},recv_buf[1024] = {0};
//    int nsend = 0,retval,nrecv,len;
//    NetConfigTypeDef *tmp = &tNorthConfig;
//
//    if ((cliconnfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
//        return -1;
//    }
//    memset(&cliaddr, 0, sizeof(cliaddr));
//    cliaddr.sin_family = AF_INET;
//    cliaddr.sin_addr.s_addr = inet_addr(tmp->targetIP);
//    cliaddr.sin_port = htons(tmp->targetPort);
//    len = sizeof(cliaddr);
//    while(1){
//    	//0、注册机制
//    	//1、主动上传
//    	memcpy(send_buf,"upd_client_test1\r\n",sizeof("upd_client_test1\r\n"));
//        nsend = sendto(cliconnfd,send_buf,strlen(send_buf),0,(struct sockaddr*)&cliaddr,sizeof(cliaddr));
//        if(nsend < 1){
//            continue;
//        }
//        tv.tv_sec = 4;
//        tv.tv_usec = 0;
//        FD_ZERO(&fs_read);
//        FD_SET(cliconnfd, &fs_read);
//        retval = select(cliconnfd + 1, &fs_read, NULL, NULL, &tv);
//        if(retval < 0){
//            close(cliconnfd);
//            connectflag = 0;
//            continue;
//        }else if(!retval){
//            printf("recv timeout\n");
//        }else{
//            if(FD_ISSET(cliconnfd,&fs_read)){
//                nrecv = recvfrom(cliconnfd, recv_buf, 1024, 0,(struct sockaddr*)&cliaddr,&len);
//                printf("recv %s len=%d\n", recv_buf, nrecv);
//                if(nrecv == 0){
//                    close(cliconnfd);
//                    connectflag = 0;
//                    continue;
//                }
//                //3、监听指令回复
//                sendto(cliconnfd,recv_buf,strlen(recv_buf),0,(struct sockaddr*)&cliaddr,sizeof(cliaddr));
//            }
//        }
//    }
//}

//北向网口UDP服务端线程
//这个有问题，UDP SERVER
//void *thread_UdpServer_North(void *arg)
//{
//    socklen_t cliaddr_len;
//    struct sockaddr_in servaddr,cliaddr;
//    int listenfd = -1, connfd = -1, optval = 1;
//    int yes = -1;
//    pthread_t ntid;
//    net_addr_t cliNetAddr;
//    char send_buf[1024] = {"1223e43"},recv_buf[1024] = {0};
//    int nsend = 0,retval,nrecv;
//    NetConfigTypeDef *tmp = &tNorthConfig;
//
//    if ((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
//        return -1;
//    }
//
//    memset(&servaddr, 0, sizeof(struct sockaddr_in));
//    servaddr.sin_family = AF_INET;
//    servaddr.sin_addr.s_addr = inet_addr(tmp->localIP);
//    servaddr.sin_port = htons(tmp->localPort);
//
//    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
//        printf("errno2=%d\n", errno);
//        close(listenfd);
//        return -1;
//    }
//
//    cliaddr_len = sizeof(cliaddr);
//    for(;;){
//        nrecv = recvfrom(listenfd, recv_buf, 1024, 0,(struct sockaddr*)&cliaddr,&cliaddr_len);
//        if (connfd < 0) {
//            close(listenfd);
//            listenfd = -1;
//            printf("netapp accept error\n");
//            continue;
//        }else{
//            cliNetAddr.num = 1;
//            cliNetAddr.connect_id = listenfd;
//           // thread_dataExchange(&cliNetAddr);
//            //err = pthread_create(ntid,NULL,thread_dataExchange,&cliNetAddr);
//        }
//    }
//}

//南向网口服务端线程
//void *thread_TcpServer_South(void *arg)
//{
//
//    int listenfd = -1, connfd = -1, optval = 1;
//    int yes = -1;
//    pthread_t ntid;
//    net_addr_t cliNetAddr;
//    socklen_t cliaddr_len;
//	struct sockaddr_in servaddr,cliaddr;
//	NetConfigTypeDef *tmp = &tSouthConfig;
//
//    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//    	printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
//        return -1;
//    }
//    memset(&servaddr, 0, sizeof(struct sockaddr_in));
//    servaddr.sin_family = AF_INET;
//    servaddr.sin_addr.s_addr = inet_addr(tmp->localIP);
//    servaddr.sin_port = htons(tmp->localPort);
//
//    if (setsockopt(listenfd, SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
//        close(listenfd);
//        return -1;
//    }
//    if (bind(listenfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
//        printf("errno1=%d\n", errno);
//        close(listenfd);
//        return -1;
//    }
//    optval = listen(listenfd, 20);
//
//    cliaddr_len = sizeof(cliaddr);
//    for(;;){
//        cliaddr_len = sizeof(cliaddr);
//        connfd = Accept(listenfd, (struct sockaddr *)&cliaddr,&cliaddr_len);
//        if (connfd < 0) {
//            close(listenfd);
//            listenfd = -1;
//            printf("netapp accept error\n");
//            continue;
//        }else{
//            cliNetAddr.num = 0;		//tmp.linkNumber;
//            cliNetAddr.connect_id = connfd;
//            pthread_create(&ntid,NULL,childThread_TcpServer_South,&cliNetAddr);
//        }
//    }
//}

//南向网口客户端线程
//void *thread_TcpClient_South(void *arg)
//{
//
//    int  cliconnfd = -1,retval,flags;
//    int connectflag = 0;
//    struct sockaddr_in  cliaddr;
//    struct timeval tv;
//    fd_set fs_read;
//    char send_buf[1024] = {0},recv_buf[1024] = {0};
//    int nsend = 0,ret,nrecv;
//    NetConfigTypeDef *tmp = &tSouthConfig;
//
//    while(1){
//        if(0 == connectflag){
//            if ((cliconnfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
//                return -1;
//            }
//            memset(&cliaddr, 0, sizeof(struct sockaddr_in));
//            cliaddr.sin_family = AF_INET;
//            cliaddr.sin_addr.s_addr = inet_addr(tmp->targetIP);
//            cliaddr.sin_port = htons(tmp->targetPort);
//            flags = fcntl(cliconnfd,F_GETFL,0);
//            flags &= ~O_NONBLOCK;
//            fcntl(cliconnfd,F_SETFL,flags);
//            retval = connect(cliconnfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
//            if (retval < 0) {
//                close(cliconnfd);
//                cliconnfd = -1;
//                usleep(10000);
//                continue;
//             }else{
//                connectflag = 1;
//            }
//        }else{
//
//        	//0、注册机制
//
//        	//1、主动上传
//        	memcpy(send_buf,"client_test2",strlen("client_test2"));
//        	nsend = send(cliconnfd,send_buf,strlen(send_buf),MSG_NOSIGNAL);
//            if(nsend < 1){
//                close(cliconnfd);
//                connectflag = 0;
//                continue;
//            }
//
//            tv.tv_sec = 4;
//            tv.tv_usec = 0;
//            FD_ZERO(&fs_read);
//            FD_SET(cliconnfd, &fs_read);
//            retval = select(cliconnfd + 1, &fs_read, NULL, NULL, &tv);
//            if(retval < 0){
//                close(cliconnfd);
//                connectflag = 0;
//                continue;
//            }else if(!retval){
//                printf("recv timeout\n");
//            }else{
//                if(FD_ISSET(cliconnfd,&fs_read)){
//                    nrecv = recv(cliconnfd, recv_buf, 1024, MSG_NOSIGNAL);
//                    printf("recv %s len=%d\n", recv_buf, nrecv);
//
//                    if(nrecv == 0){
//                        close(cliconnfd);
//                        connectflag = 0;
//                        continue;
//                    }
//                    //3、监听指令回复
//                    send(cliconnfd,recv_buf,strlen(recv_buf),MSG_NOSIGNAL);
//                }
//            }
//        }
//    }
//}

//南向网口服务端子线程
//void *childThread_TcpServer_South(void *arg)
//{
//    net_addr_t netaddr = *(net_addr_t*)arg;
//    char send_buf[1024] = {0},recv_buf[1024]={0};
//    unsigned char outhex[500];
//    int nsend = 0,retval,nrecv,sendflag = 0;
//    struct timeval tv;
//    fd_set fs_read;
//    int z = 0,number;
//
//    while(1){
//    	//0、注册机制
//
//    	//1、主动上传
////        if(netaddr.num == 100){
////        	sleep(1);
////        }else{
////            memset(send_buf,0,1024);
////            if(device_command[netaddr.num].command_content[z].type == 1){
////                memcpy(send_buf,device_command[netaddr.num].command_content[z].buf,device_command[netaddr.num].command_content[z].length*2-2);
////                sscanfHexArray(send_buf,&number,outhex);
////                nsend = send(netaddr.connect_id,outhex,number,MSG_NOSIGNAL);
////            }else{
//////               memcpy(send_buf,device_command[netaddr.num].command_content[z].buf,device_command[netaddr.num].command_content[z].length);
////            	memcpy(send_buf,"tset1",sizeof("tset1"));
////                nsend = send(netaddr.connect_id,send_buf,strlen(send_buf),MSG_NOSIGNAL);
////                usleep(1000);
////            }
////        }
////        if(nsend < 1){
////            close(netaddr.connect_id);
////            break;
////        }
//
//        //2、监听回复
//        tv.tv_sec = 4;
//        tv.tv_usec = 0;
//        FD_ZERO(&fs_read);
//        FD_SET(netaddr.connect_id, &fs_read);
//        retval = select(netaddr.connect_id + 1, &fs_read, NULL, NULL, &tv);
//        if(retval < 0){
//        	close(netaddr.connect_id);
//        }else if(!retval){
////        	printf("recv timeout\n");
//            sendflag++;
//        }else{
//            if(FD_ISSET(netaddr.connect_id,&fs_read)){
//                nrecv = recv(netaddr.connect_id, recv_buf, 255, MSG_NOSIGNAL);
//                printf("recv %s\r\nlen=%d\r\n",recv_buf,nrecv);
//                if(nrecv <= 0){
//                    close(netaddr.connect_id);
//                    break;
//                }
//
//                //3、监听指令回复
//                send(netaddr.connect_id,recv_buf,strlen(recv_buf),MSG_NOSIGNAL);
//                usleep(1000);
//                sendflag = 0;
////                z++;
////                if(z >= device_command[netaddr.num].commandNum){
////                    sleep(device_command[netaddr.num].commandTime);
////                    z = 0;
////                }
//            }
//        }
//    }
//}


SocketParaTypeDef tUdpClientSouth;
int sendBuffUdpClientSouth(const char *src,const int len)
{
	char buff[1024]={0};
	memcpy(buff,src,len);
	int retval = sendto(tUdpClientSouth.fd,buff,len,0,(struct sockaddr*)&tUdpClientSouth.addr,sizeof(struct sockaddr_in));
	return retval;
}
int recvBuffUdpClientSouth(char dst[1024])
{
	int retval = -1;
	if(tUdpClientSouth.recvEndSign){
		tUdpClientSouth.recvEndSign = 0;
		retval = strlen(tUdpClientSouth.recvBuff);
		memcpy(dst,tUdpClientSouth.recvBuff,retval);
	}
	else
		return -1;
	return retval;
}

//南向网口UDP客户端线程
void *threadUdpClientSouth(void *arg)
{
    int  connectfd  = -1;			//连接文件描述符
    struct sockaddr_in  clientAddr;	//客户端socket参数
    struct timeval tv;				//监听时间
    fd_set fs;					//select参数
    char sendBuf[SOCKET_SEND_LEN] = {0},recvBuf[SOCKET_RECV_LEN] = {0};
    int sendLen = 0,recvLen = 0;	//收发缓存，长度
    int retval;			//函数返回rertval
    NetworkConfigTypeDef *config = &tSouthNetConfig;

    //创建套接字
    if ((connectfd = socket(AF_INET,SOCK_DGRAM,0)) == -1){
    	perror("create socket failed");
        return -1;
    }
    //配置ip参数
    memset(&clientAddr,0,sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = (in_addr_t)config->targetAddr;
    clientAddr.sin_port = htons(config->targetPort);

    //socket参数
    tUdpClientSouth.fd = connectfd;
    memcpy(&tUdpClientSouth.addr,&clientAddr,sizeof(clientAddr));
    while(1){
    	//0、注册机制

    	//1、主动上传
//    	memcpy(sendBuf,"upd_client_test1\r\n",sizeof("upd_client_test1\r\n"));
//    	sendLen = sendto(connectfd,sendBuf,strlen(sendBuf),0,(struct sockaddr*)&clientAddr,sizeof(clientAddr));
//        if(sendLen < 1){
//            continue;
//        }

//    	sendLen = sendBuffUdpClientSouth("hello world!",strlen("hello world!"));
//		if(sendLen < 1){
//			continue;
//		}

        //select参数设置
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&fs);
        FD_SET(connectfd, &fs);
        retval = select(connectfd + 1, &fs, NULL, NULL, &tv);
        if(retval < 0){
        	perror("connectfd + 1 select");
            close(connectfd);
            continue;
        }else if(!retval){
            //无接收
        }else{
            if(FD_ISSET(connectfd,&fs)){
            	memset(recvBuf,0,sizeof(recvBuf));
            	recvLen = recvfrom(connectfd,recvBuf,1024,0,(struct sockaddr*)&clientAddr,sizeof(clientAddr));
            	tUdpClientSouth.recvEndSign = 1;
            	memcpy(tUdpClientSouth.recvBuff,recvBuf,strlen(recvBuf));
//                printf("recv %s len=%d\n", recvBuf, strlen(recvBuf));
                if(recvLen == 0){
                    close(connectfd);
                    continue;
                }
                //3、监听指令回复
// 				sendto(connectfd,recvBuf,strlen(recvBuf),0,(struct sockaddr*)&clientAddr,sizeof(clientAddr));
            }
        }
    }
}



//南向网口UDP服务端线程
//这个有问题，UDP SERVER
//void *thread_UdpServer_South(void *arg)
//{
//    socklen_t cliaddr_len;
//    struct sockaddr_in servaddr,cliaddr;
//    int listenfd = -1, connfd = -1, optval = 1;
//    int yes = -1;
//    pthread_t ntid;
//    net_addr_t cliNetAddr;
//    char send_buf[1024] = {"1223e43"},recv_buf[1024] = {0};
//    int nsend = 0,ret,nrecv;
//    NetConfigTypeDef *tmp = &tSouthConfig;
//
//    if ((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
//        return -1;
//    }
//
//    memset(&servaddr, 0, sizeof(struct sockaddr_in));
//    servaddr.sin_family = AF_INET;
//    servaddr.sin_addr.s_addr = inet_addr(tmp->localIP);
//    servaddr.sin_port = htons(tmp->localPort);
//
//    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
//        printf("errno2=%d\n", errno);
//        close(listenfd);
//        return -1;
//    }
//
//    cliaddr_len = sizeof(cliaddr);
//    for(;;){
//        nrecv = recvfrom(listenfd, recv_buf, 1024, 0,(struct sockaddr*)&cliaddr,&cliaddr_len);
//        if (connfd < 0) {
//            close(listenfd);
//            listenfd = -1;
//            printf("netapp accept error\n");
//            continue;
//        }else{
//            cliNetAddr.num = 1;
//            cliNetAddr.connect_id = listenfd;
//           // thread_dataExchange(&cliNetAddr);
//            //err = pthread_create(ntid,NULL,thread_dataExchange,&cliNetAddr);
//        }
//    }
//}
