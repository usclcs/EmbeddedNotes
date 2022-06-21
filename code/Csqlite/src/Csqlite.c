#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct{
	int No;
	char TmlType[4];
	char TmlID[5];
	char TmlKey[12];
	char DevType[4];
	char DevID[5];
}tmldev_t;

tmldev_t tmldev[8]={0};
char buff[10];

static int callback(void *data, int argc, char **argv, char **azColName){
	for(int i=0;i<argc;i++){
		printf("%s = %s\n",azColName[i],argv[i]?argv[i]:"NULL");
	}
	int value = (int)atol(argv[0]);
	if(value < 8){
		tmldev[value].No = value;
		memcpy(tmldev[value].TmlType,argv[1],4);
	}

	return 0;
}

int main(void)
{
   sqlite3 *db;

   int rc = sqlite3_open("tml.db", &db);
   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }

   /* Create SQL statement */
   char *sql = "SELECT * from TmlDev";
   /**/
   char *zErrMsg = 0;
   rc = sqlite3_exec(db,sql,callback,0,&zErrMsg);
      if( rc != SQLITE_OK ){
         fprintf(stderr, "SQL error: %s\n",zErrMsg);
         sqlite3_free(zErrMsg);
      }else{
         fprintf(stdout, "Operation done successfully\n");
      }
   sqlite3_close(db);
   printf("NO:%d\n",tmldev[7].No);
   printf("TMLTYPE:%.4s\n",tmldev[6].TmlType);
}
