新建表格

```sqlite
CREATE TABLE TmlDev(No INTEGER NOT NULL, TmlType TEXT NOT NULL,TmlID TEXT NOT NULL,TmlKey TEXT NOT NULL,DevType TEXT NOT NULL,DevID TEXT NOT NULL);
```



插入数据

```sqlite
INSERT INTO TmlDev VALUES ('1','5110','10001','123456789012','1C01','00001');
```



读取表

```sqlite
SELECT * FROM TmlDev;
```



Csqlit3

打开数据库文件

```c
 ret = sqlite3_open("tml.db",&db);
```





| 序号 | 终端类型 | 终端编号 | 终端密钥     | 设备类型 | 设备编号 |
| ---- | -------- | -------- | ------------ | -------- | -------- |
| 1    | 5110     | 10001    | 123456789012 | 1C01     | 00001    |



