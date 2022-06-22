#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define 	NUM			22

typedef struct{
	int ID;
	char TmlType[4];
	char TmlID[5];
	char TmlKey[12];
	char DevType[4];
	char DevID[5];
}tmldev_t;

tmldev_t tmldev[NUM];

static int sqlcallback(void *data,int argc,char**argv,char**col)
{
	for(int i=0;i<argc;i++){
		printf("col:%s val:%s\n",col[i],argv[i]?argv[i]:"NULL");
	}
	/*回调函数中取值,先取数据中序号,字段0为序号*/
	int no = (int)atol(argv[0]);
	if(no<NUM){
		tmldev[no].ID = no;
		memcpy(tmldev[no].TmlType,argv[1],4);
		memcpy(tmldev[no].TmlID,argv[2],5);
		memcpy(tmldev[no].TmlKey,argv[3],12);
		memcpy(tmldev[no].DevType,argv[4],4);
		memcpy(tmldev[no].DevID,argv[5],5);
	}
	return 0;
}


int main(void)
{
	/*定义数据库文件描述符*/
	sqlite3 *db;
	/*打开数据库文件*/
	int ret = sqlite3_open("tml.db", &db);
	   /*检查数据打开返回*/
	if(ret){
		/*打印错误信息,错误号*/
		printf("Can't open database:%s\n",sqlite3_errmsg(db));
		exit(0);
	   }
	   /*新建SQL语句*/
	   char *sql = "SELECT * from TmlDev";
	   /*执行SQL语句*/
	   char *errmsg = 0;
	   ret = sqlite3_exec(db,sql,sqlcallback,0,&errmsg);
	   /*检查执行指令返回*/
	   if(ret != SQLITE_OK){
	         printf("SQL error:%s\n",errmsg);
	         sqlite3_free(errmsg);
	      }
	   /*关闭数据库文件*/
	   sqlite3_close(db);

	   printf("NO:%d\n",tmldev[6].ID);
	   printf("TMLTYPE:%.4s\n",tmldev[6].TmlType);
	   printf("TMLID:%.5s\n",tmldev[6].TmlID);
	   printf("TMLKEY:%.12s\n",tmldev[6].TmlKey);
	   printf("DEVTYPE:%.4s\n",tmldev[6].DevType);
	   printf("DEVID:%.5s\n",tmldev[6].DevID);
}
