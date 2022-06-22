#### 1、sqlite3软件与开发环境

```bash
sudo apt-get install sqlite3		#安装sqlite3
sudo apt-get install libsqlite3-dev #安装sqlite3开发包
```

注：

1. ubuntu中sqlite3.h路径：/usr/include
2. sqlite3动态库：-lsqlite3

#### 2、sqlite3数据库可视化工具

##### 2.1、sqlitebrowser

```bash
sudo apt-get install sqlitebrowser
------------------------------------------------------------------------------------------
sqlitebrowser	#打开sqlitebrowser
```

##### 2.2、Navicat

Navicat Premium 12

#### 3、sqlite3功能与指令

```bash
.databases #列出数据库的名称及其所依附的文件
.open DATABASE_NAME		#打开数据库文件，若无则新建
.tables		#列出数据库中所有的表
.schema TABLE_NAME	#列出表中字段信息
.quit	#退出
.exit	#退出
------------------------------------------------------------------------------------------
.show	#查看sqlite3配置信息
sqlite> .show
        echo: off
         eqp: off
     explain: auto
     headers: off
        mode: list
   nullvalue: ""
      output: stdout
colseparator: "|"
rowseparator: "\n"
       stats: off
       width: 
```

#### 4、sqlite3语法

1. sqlite3功能：创建、删除、插入、检索、备份、附加、分离；
2. sqlite3数据类型：NULL,INTEGER,REAL,TEXT,BLOB；
3. sqlite3关键词：SELECT,INSERT,UPDATE,DELETE,ALTER,DROP,ANALYZE,FROM,WHERE ,AND/OR,ADD,TO,As,BEGIN,BETWEEN,COMMIT,TRIGGER... ...;

创建表：

```bash
CREATE TABLE company(name INTEGER NOT NULL,value INTEGER);	-- NOT NULL非空，插入一定有值
CREATE TABLE CurTemp.company(name INTEGER NOT NULL,value INTEGER);
```

删除表：

```bash
DROP TABLE company;
```

插入值：

```
INSERT INTO CurTemp.company VALUES (1,2);
INSERT INTO CurTemp.company (name) VALUES (1);
```

检索值：

```
SELECT * FROM CurTemp.company;		#通配符*
SELECT name FROM CurTemp.company;
SELECT value FROM CurTemp.company;
------------------------------------------------------------------------------------------
.header on		#开启字段名显示
.mode column	#输出模式按列
SELECT * FROM CurTemp.company;
```

 附加：

```bash
ATTACH DATABASE 'test.db' as 'CurTemp';
```

分离：

```
DETACH DATABASE 'CurTemp';
```

#### 5、C接口API

```c
sqlite3_open(const char *filename, sqlite3 **ppDb)	//打开数据库文件
sqlite3_exec(sqlite3*, const char *sql, sqlite_callback, void *data, char **errmsg)	//执行指令
sqlite3_close(sqlite3*)	//关闭数据库文件
```

```c
/*原型：int *sqlite3_callback(void*,int,char**,char**)*/
typedef int (*sqlite3_callback)(
void*,    /* Data provided in the 4th argument of sqlite3_exec() */
int,      /* The number of columns in row */
char**,   /* An array of strings representing fields in the row */
char**    /* An array of strings representing column names */
);
```

**实例：**

| ID   | TmlType  | TmlID    | TmlKey   | DevType  | DevID    |
| ---- | -------- | -------- | -------- | -------- | -------- |
| 序号 | 终端类型 | 终端编号 | 终端密钥 | 设备类型 | 设备编号 |

```c
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
	   
       /*测试数据*/
	   printf("NO:%d\n",tmldev[6].ID);
	   printf("TMLTYPE:%.4s\n",tmldev[6].TmlType);
	   printf("TMLID:%.5s\n",tmldev[6].TmlID);
	   printf("TMLKEY:%.12s\n",tmldev[6].TmlKey);
	   printf("DEVTYPE:%.4s\n",tmldev[6].DevType);
	   printf("DEVID:%.5s\n",tmldev[6].DevID);
}
```

