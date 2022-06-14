

#### 1、准备

linux环境变量加载顺序

```
==> /etc/environment（系统级）在系统启动时运行，用于配置与系统运行相关但与用户无关的环境变量
==> /etc/profile（系统级）# 打开/etc/profile文件你会发现，该文件的代码中会加载/etc/bash.bashrc文件，然后检查/etc/profile.d/目录下的.sh文件并加载
==> /etc/bashrc(Ubuntu中是/etc/bash.bashrc)（系统级）
==> /etc/profile.d/*.sh(此项在/etc/profile文件中执行)
==> ~/.bash_profile | ~/.bash_login | ~/.profile # 打开~/.profile文件，会发现该文件中加载了~/.bashrc文件。~/.bash_profile文件只在用户登录的时候读取一次
==> ~/.bashrc # ~/.bashrc在每次打开终端进行一次新的会话时都会读取
==> ~/.bash_logout
系统环境变量 -> 用户自定义环境变量。
在用户环境变量中，系统会首先读取~/.bash_profile（或者~/.profile）文件，如果没有该文件则读取~/.bash_login，根据这些文件中内容再去读取~/.bashrc。
```

软件更新

```
sudo apt-get update -- 从服务器获取软件列表，并在本地保存为文件。
sudo apt-get upgrade -- 本地安装软件与本地软件列表对比，如本地安装版本低，会提示更新。
```

python3.5 工具包下载

```
python3.5-doc -- python文档
python3.5-venv -- python虚拟环境
python3.5-dbg -- python调试工具
python3.5-examples  -- python案例
python3.5-dev -- python类库
python3.5-minimal -- python最小子集
```

python3.5和python3.5m区别？

```
Python实现可以在文件名标签中适当地包含其他标志。例如，在POSIX系统上，这些标志也将有助于文件名：
--with-pydebug（标志：d）
--with-pymalloc（标志：m）
--with-wide-unicode（标志：u）

Pymalloc是由Vladimir Marangozov编写的专用对象分配器，是Python
2.1中新增的一项功能。Pymalloc旨在比系统malloc（）更快，并且对于Python程序典型的分配模式而言，具有较少的内存开销。分配器使用C的malloc（）函数获取较大的内存池，然后从这些池执行较小的内存请求。
```

安装python（python3.5）

```
sudo apt-get update
sudo apt-get install python3.5
```

安装python-dev

```
sudo apt-get install python3.5-dev
```

查找Python.h路径

```
/usr/include/python3.5 -- Python.h路径
```

查找python库（环境变量）路径

```bash
$ python3
Python 3.5.2 (default, Jan 26 2021, 13:30:48) 
[GCC 5.4.0 20160609] on linux
Type "help", "copyright", "credits" or "license" for more information.
import sys
sys.path
['', '/usr/lib/python35.zip', '/usr/lib/python3.5', '/usr/lib/python3.5/plat-x86_64-linux-gnu', '/usr/lib/python3.5/lib-dynload', '/usr/local/lib/python3.5/dist-packages', '/usr/lib/python3/dist-packages']
```

```
#python库路径
'' -- 第1项是空串''，代表当前目录
'/usr/lib/python35.zip'	-- 未知，没有这个文件或文件夹
'/usr/lib/python3.5' -- 未知
'/usr/lib/python3.5/plat-x86_64-linux-gnu' -- 未知
'/usr/lib/python3.5/lib-dynload' -- python依赖动态均衡？？
'/usr/local/lib/python3.5/dist-packages' -- python安装的第三方库包路径
'/usr/lib/python3/dist-packages' -- python安装的第三方库包路径
```



#### 2、头文件

```
/*pylifecycle.h*/
GET:
Py_GetPythonHome();
Py_GetProgramName();
Py_GetPath();	-- 获取python环境变量
Py_GetPrefix();
Py_GetExecPrefix();
Py_GetProgramFullPath();

Py_GetVersion();	-- 获取python版本
Py_GetPlatform();	-- 获取python平台
Py_GetCopyright();	-- 获取python版权信息
Py_GetBuildInfo();	-- 获取python编译信息
Py_GetCompiler();	-- 获取python编译器信息

SET:
Py_SetPythonHome();
Py_SetProgramName();
Py_SetPath();		-- 设置python环境变量

Py_Initialize();	-- 初始化python
Py_Finalize();		-- 结束python
Py_IsInitialized();	-- 判断初始化python结果
```



```
/*abstract.h*/
PyObject_Call();
PyObject_CallObject();
PyObject_CallFunction();
PyObject_CallMethod();

```



```
/*cevall.h*/
PyEval_CallObject();
PyEval_CallFunction();
PyEval_CallMethod();

PyEval_SaveThread();			-- 保存当前线程状态，释放GIL锁
PyEval_RestoreThread();			-- 恢复当前线程状态

PyEval_InitThreads();			-- 初始化python多线程支持
_PyEval_FiniThreads();			-- 结束python多线程支持
PyEval_ThreadsInitialized();	-- 判断是否开启python多线程支持

```



```
/*pystate.h*/
PyGILState_Ensure();	-- 获取GIL锁
PyGILState_Release();	-- 释放GIL锁
PyGILState_Check();		-- 判断是否获取GIL锁
```



```
/*import.h*/
GET:
PyImport_GetModuleDict();

ADD:
PyImport_AddModuleObject();
PyImport_AddModule();

IMPORT:
PyImport_ImportModule();	-- 导入python模块
PyImport_Import();

RELOAD:
PyImport_ReloadModule();
```



```
/*object.h*/
PyObject_GetAttrString();	-- 
```



```
/*moduleobject.h*/
PyModule_GetDict();
PyModule_GetNameObject();
PyModule_GetName();
PyModule_GetFilename();
PyModule_GetFilenameObject();
```



```
/*pythonrun.h*/
PyRun_SimpleString();	-- 引入常用的路径
PyRun_SimpleFile();		-- 执行python文件
```



执行python文件

```c
/*获取GIL，确保为当前线程控制GIL（一种锁，python特有的全局解释锁）*/
PyGILState_STATE state = PyGILState_Ensure();
/*打开python文件*/
FILE* fp = fopen("/home/ciei/Desktop/files/code/printf.py", "r");
/*执行python文件内容*/
PyRun_SimpleFile(fp,"printf");
/*释放GIL，将GIL的控制权交还给系统*/
 PyGILState_Release(state);
```

printf.py示例：

```python
#!/usr/bin/python
# -*- coding:UTF-8 -*-
from time import *
while True:
    print("hello world")
    sleep(5)
```



执行python函数

```c
/*获取GIL，确保为当前线程控制GIL（一种锁，python特有的全局解释锁）*/
PyGILState_STATE state = PyGILState_Ensure();
/*调用python*/
PyObject * pModule = NULL;
PyObject * pValue = NULL;
PyObject * pFunc = NULL;
pModule = PyImport_ImportModule("sayhello");		//加载模块
pFunc = PyObject_GetAttrString(pModule, "say");   	//获取函数
PyEval_CallObject(pFunc, NULL);           			//执行函数
/*释放GIL，将GIL的控制权交还给系统*/
 PyGILState_Release(state);
```

sayhello.py示例：

```python
#!/usr/bin/python
# -*- coding:UTF-8 -*-
from time import *
def say():
	while True:
		print("123")
		sleep(2)

def main():
    say()
if __name__ == '__main__':
    main()
```



#### 3、流程

初始化python

```
初始化：
设置python环境变量Py_SetPath();
初始化python解释器及模块Py_Initialize();
初始化多线程，使python支持多线程，PyEval_InitThreads();
在开启多线程之前，释放锁以获取线程全局锁，PyEval_ReleaseLock();

结束：
释放GIL状态，PyGILState_Ensure(); 
释放python模块，Py_Finalize(); 
```



释放PyObject*内存，Py_DECREF();	Py_XDECREF();

```c
	PyObject *pDict = NULL;           	//python模块字典
	PyObject *pModule = NULL;    		//python模块

...

	/*释放模块申请的内存*/ 
    if(pModule){  
		Py_DECREF(pModule);
 	}
	/*释放模块字典申请的内存*/
 	if(pDict){  
 		Py_DECREF(pDict);
    }
```

调用流程

```
1：必须首先调用Py_Initialize()，初始化python运行所需模块。
2：接着调用Py_IsInitialized()，检查初始化是否成功
3：调用PyRun_SimpleString()，引入常用的路径
4：调用PyImport_ImportModule()，加载python模块，引入py文件
5：调用PyModule_GetDict()，获取模块字典
6：调用PyObject_GetAttrString()  PyDict_GetItemString()，获取相应的方法或者类
7：调用PyEval_CallObject()  PyObject_CallMethod() 调用相应的方法
8：调用Py_DECREF() 释放python api创建的对象
9：调用Py_Finalize() 释放python模块
```

python结果判断

```
判断是否完成初始化，Py_IsInitialized()，0-fail，1-success；
判断是否开启python多线程支持，PyEval_ThreadsInitialized()，0-fail，1-success；
```



c代码示例：

```c
#include <stdio.h>
#include <stdlib.h>
#include <Python.h>

PyThreadState* initPython(void)
{
	/* 设置 Python 的 PATH */
	Py_SetPath(L"/usr/lib/python35.zip:"
				"/usr/lib/python3.5:"
				"/usr/lib/python3.5/plat-x86_64-linux-gnu:"
				"/usr/lib/python3.5/lib-dynload:"
				"/usr/local/lib/python3.5/dist-packages:"
				"/usr/lib/python3/dist-packages:"
				"/home/ciei/Desktop/files/code:");
	/* 初始化 Python 解释器 */
    Py_Initialize();
	/* 开启多线程支持, 参考下文链接中ashhadul islam回答的第2种方法 */
	/* https://stackoverflow.com/a/42667657/18856782*/
	PyEval_InitThreads();
	/* 保存当前线程状态, 释放 GIL 锁 */
	return PyEval_SaveThread();
}

int main(void)
{
    /* 初始化 Python*/
	initPython();
/*获取安装的python信息*/
#if 0
	printf("python home: %s\n", Py_GetPythonHome());
	printf("program name: %s\n", Py_GetProgramName());
	printf("get path: %s\n", Py_GetPath());
	printf("get prefix: %s\n", Py_GetPrefix());
	printf("Py_GetBuildInfo: %s\n", Py_GetBuildInfo());
	printf("Py_GetCompiler: %s\n", Py_GetCompiler());
#endif
    /* 获得 Python 线程锁 */
    PyObject * pModule = NULL;
    PyObject * pValue = NULL;
    PyObject * pFunc = NULL;

/*方案1*/
#if 0
	PyGILState_STATE state =  PyGILState_Ensure();
	char szFile[] = "/home/ciei/Desktop/files/code/sayhello.py";	//python文件路径
	FILE* fp = fopen(szFile, "r");		//打开python文件
	PyRun_SimpleFile(fp,"sayhello");		//执行python文件
	PyGILState_Release(state); 			//释放GIL锁
	fclose(fp);
#endif

/*方案2*/
#if 0
	PyGILState_STATE state =  PyGILState_Ensure();
	pModule = PyImport_ImportModule("sayhello");		//加载模块
	pFunc = PyObject_GetAttrString(pModule, "say");   	//获取函数
	PyEval_CallObject(pFunc, NULL);           			//执行函数
    PyGILState_Release(state); 							//释放GIL锁
#endif

/*方案3*/
#if 1
	PyGILState_STATE state =  PyGILState_Ensure();
	pModule = PyImport_ImportModule("sayhello");		//加载模块
    pValue = PyObject_CallMethod(pModule,"say","");		//执行方法
    PyGILState_Release(state); 							//释放GIL锁
#endif
	return EXIT_SUCCESS;
}
```

python代码示例：

```python
#!/usr/bin/python
# -*- coding:UTF-8 -*-
from time import *
def say():
	while True:
		print("123")
		sleep(2)

def main():
    say()
if __name__ == '__main__':
    main()
```

路径：/home/ciei/Desktop/files/code



**资料：**

/*https://docs.python.org/zh-cn/3.5/c-api/index.html*/

/*https://blog.csdn.net/qq_33339479/article/details/81432575*/
