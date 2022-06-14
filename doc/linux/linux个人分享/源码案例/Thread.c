#include "Gateway.h"
#include "Thread.h"
#include "NetApp.h"
#include "ReadFile.h"
#include "ChannalTask.h"

#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>

#define		STACK_SIZE		20480U
#define		CHANNAL_TASK(n)		n

unsigned char threadTaskNo = 0;

extern channal_task_t channal_task[CHANNAL_MAX];
extern void (*pTask[PROGRAM_PTR_NUM])(unsigned char);

int create_thread(void *(*start_routine)(void *), int prio)
{
	struct sched_param param;
	pthread_attr_t attr={0};
	pthread_t thread;
	int ret;

	/* Initialize pthread attributes (default values) */
	ret = pthread_attr_init(&attr);
	if (ret) {
		printf("init pthread attributes failed\n");
		goto out;
	}
	/* Set a specific stack size PTHREAD_STACK_MIN */
	ret = pthread_attr_setstacksize(&attr, STACK_SIZE);
	if (ret) {
		printf("pthread setstacksize failed\n");
		goto out;
	}
	/* Set scheduler policy and priority of pthread */
	ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
	if (ret) {
		printf("pthread setschedpolicy failed\n");
		goto out;
	}
	param.sched_priority = prio;
	ret = pthread_attr_setschedparam(&attr, &param);
	if (ret) {
		printf("pthread setschedparam failed\n");
		goto out;
	}
	/* Use scheduling parameters of attr */
	ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (ret) {
		printf("pthread setinheritsched failed\n");
		goto out;
	}

	/* Create a pthread with specified attributes */
	ret = pthread_create(&thread, &attr, start_routine, NULL);
	if (ret) {
		printf("create pthread failed %d\n",ret);
		goto out;
	}
#if 0
	/* Join the thread and wait until it is done */
	ret = pthread_join(thread, NULL);
	if (ret)
		printf("join pthread failed: %m\n");
#endif
out:
	return ret;
}

int createNorthNetThread(void)
{
    int ret=0;
    NetConfigTypeDef *ntConfig = &tNorthConfig;
    //1、选择不同的北向网口协议类型
    printf("tNorthConfig protocolType:%d\r\n",ntConfig->protocolType);

    if(!ntConfig->protocolType){
		ret = create_thread(threadTcpServerNorth,NORTH_RECV_PRIOR);
		if (ret){
			printf("create threadTcpServerNorth failed\n");
			return 0;
		}else{
			printf("create threadTcpServerNorth success\n");
		}
    }else if(1 == ntConfig->protocolType){
		ret = create_thread(threadTcpClientNorth,NORTH_RECV_PRIOR);
		if (ret){
			printf("create threadTcpClientNorth error\n");
			return 0;
		}else{
			printf("create threadTcpClientNorth success\n");
		}
    }else if(2 == ntConfig->protocolType){
		ret = create_thread(thread_UdpServer_North,NORTH_RECV_PRIOR);
		if (ret){
			printf("create thread_UdpServer_North error\n");
			return 0;
		}else{
			printf("create thread_UdpServer_North success\n");
		}
    }else if(3 == ntConfig->protocolType){
		ret = create_thread(thread_UdpClient_North,NORTH_RECV_PRIOR);
		if (ret){
			printf("create thread_UdpClient_North error\n");
			return 0;
		}else{
			printf("create thread_UdpClient_North success\n");
		}
    }else{
		ret = create_thread(threadTcpClientNorth,NORTH_RECV_PRIOR);
		if (ret){
			printf("create threadTcpClientNorth error\n");
			return 0;
		}else{
			printf("create threadTcpClientNorth success\n");
		}
    }
    return 1;
}

int createSourthNetThread(void)
{
    int ret=0;
    NetConfigTypeDef *ntConfig = &tSouthConfig;
    //1、选择不同的南向网口协议类型
    printf("tSouthConfig protocolType :%d\r\n",ntConfig->protocolType);

    if(!ntConfig->protocolType){
		ret = create_thread(thread_TcpServer_South,SOUTH_RECV_PRIOR);
		if (ret){
			printf("create threadTcpServerNorth failed\n");
			return 0;
		}else{
			printf("create threadTcpServerNorth success\n");
		}
    }else if(1 == ntConfig->protocolType){
		ret = create_thread(thread_TcpClient_South,SOUTH_RECV_PRIOR);
		if (ret){
			printf("create thread_TcpClient_South error\n");
			return 0;
		}else{
			printf("create thread_TcpClient_South success\n");
		}
    }else if(2 == ntConfig->protocolType){
		ret = create_thread(thread_UdpServer_South,SOUTH_RECV_PRIOR);
		if (ret){
			printf("create thread_UdpServer_South error\n");
			return 0;
		}else{
			printf("create thread_UdpServer_South success\n");
		}
    }else if(3 == ntConfig->protocolType){
		ret = create_thread(thread_UdpClient_South,SOUTH_RECV_PRIOR);
		if (ret){
			printf("create thread_UdpClient_South error\n");
			return 0;
		}else{
			printf("create thread_UdpClient_South success\n");
		}
    }else{
		ret = create_thread(thread_TcpClient_South,SOUTH_RECV_PRIOR);
		if (ret){
			printf("create thread_TcpClient_South error\n");
			return 0;
		}else{
			printf("create thread_TcpClient_South success\n");
		}
    }
    return 1;
}

int createSouthUartThread(void)
{
	int ret = create_thread(thread1RS485Recv,SOUTH_UART1_PRIOR);
	if (ret) {
		printf("create thread1RS485Recv failed\n");
		return 0;
	}else{
		printf("create thread1RS485Recv success\n");
	}
	ret = create_thread(thread2RS485Recv,SOUTH_UART2_PRIOR);
	if (ret) {
		printf("create thread2RS485Recv failed\n");
		return 0;
	}else{
		printf("create thread2RS485Recv success\n");
	}
	ret = create_thread(thread3RS485Recv,SOUTH_UART3_PRIOR);
	if (ret) {
		printf("create thread3RS485Recv failed\n");
		return 0;
	}else{
		printf("create thread3RS485Recv success\n");
	}
	return 1;
}

static void *threadChannal(void *arg)
{
	channal_task_t *p = &channal_task;
	channal_task_t *channal = p+threadTaskNo;

	printf("threadChannal Channal Value : %d\r\n",threadTaskNo);
	(*pTask[channal->programPoint])(CHANNAL_TASK(threadTaskNo));
}

int creatChannalThread(unsigned char n)
{
	ChannalConfigTypeDef *p = &tChannalConfig;
	ChannalConfigTypeDef *config = p+n;

	threadTaskNo = n;
	if(config->enable){
		int ret = create_thread(threadChannal,CHANNAL_TASK_PRIOR(n));
		if(ret){
			printf("create threadChannal[%d] error\n",n+1);
			return 0;
		}else{
			printf("create threadChannal[%d] success\n",n+1);
		}
		return 1;
	}
}
