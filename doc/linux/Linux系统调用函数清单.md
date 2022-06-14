# 一、进程控制 

1. fork 创建一个新进程

2. clone 按指定条件创建子进程
3. execve 运行可执行文件
4. exit 中止进程
5. _exit 立即中止当前进程
6. getdtablesize 进程所能打开的最大文件数
7. getpgid 获取指定进程组标识号
8. setpgid 设置指定进程组标志号
9. getpgrp 获取当前进程组标识号
10. setpgrp 设置当前进程组标志号
11. getpid 获取进程标识号
12. getppid 获取父进程标识号
13. getpriority 获取调度优先级
14. setpriority 设置调度优先级
15. modify_ldt 读写进程的本地描述表
16. nanosleep 使进程睡眠指定的时间
17. nice 改变分时进程的优先级
18. pause 挂起进程，等待信号
19. personality 设置进程运行域
20. prctl 对进程进行特定操作
21. ptrace 进程跟踪
22. sched_get_priority_max 取得静态优先级的上限
23. sched_get_priority_min 取得静态优先级的下限
24. sched_getparam 取得进程的调度参数
25. sched_getscheduler 取得指定进程的调度策略
26. sched_rr_get_interval 取得按RR算法调度的实时进程的时间片长度
27. sched_setparam 设置进程的调度参数
28. sched_setscheduler 设置指定进程的调度策略和参数
29. sched_yield 进程主动让出处理器,并将自己等候调度队列队尾
30. vfork 创建一个子进程，以供执行新程序，常与execve等同时使用
31. wait 等待子进程终止
32. wait3 参见wait
33. waitpid 等待指定子进程终止
34. wait4 参见waitpid
35. capget 获取进程权限
36. capset 设置进程权限
37. getsid 获取会晤标识号
38. setsid 设置会晤标识号  



# 二、文件系统控制

## 1、文件读写操作

1. fcntl 文件控制
2. open 打开文件
3. creat 创建新文件
4. close 关闭文件描述字
5. read 读文件
6. write 写文件
7. readv 从文件读入数据到缓冲数组中
8. writev 将缓冲数组里的数据写入文件
9. pread 对文件随机读
10. pwrite 对文件随机写
11. lseek 移动文件指针
12. _llseek 在64位地址空间里移动文件指针
13. dup 复制已打开的文件描述字
14. dup2 按指定条件复制文件描述字
15. flock 文件加/解锁
16. poll I/O多路转换
17. truncate 截断文件
18. ftruncate 参见truncate
19. umask 设置文件权限掩码
20. fsync 把文件在内存中的部分写回磁盘  



## 2、文件系统操作

1. access 确定文件的可存取性
2. chdir 改变当前工作目录
3. fchdir 参见chdir
4. chmod 改变文件方式
5. fchmod 参见chmod
6. chown 改变文件的属主或用户组
7. fchown 参见chown
8. lchown 参见chown
9. chroot 改变根目录
10. stat 取文件状态信息
11. lstat 参见stat
12. fstat 参见stat
13. statfs 取文件系统信息
14. fstatfs 参见statfs
15. readdir 读取目录项
16. getdents 读取目录项
17. mkdir 创建目录
18. mknod 创建索引节点
19. rmdir 删除目录
20. rename 文件改名
21. link 创建链接
22. symlink 创建符号链接
23. unlink 删除链接
24. readlink 读符号链接的值
25. mount 安装文件系统
26. umount 卸下文件系统
27. ustat 取文件系统信息
28. utime 改变文件的访问修改时间
29. utimes 参见utime
30. quotactl 控制磁盘配额  



# 三、系统控制

1. ioctl I/O总控制函数
2. _sysctl 读/写系统参数
3. acct 启用或禁止进程记账
4. getrlimit 获取系统资源上限
5. setrlimit 设置系统资源上限
6. getrusage 获取系统资源使用情况
7. uselib 选择要使用的二进制函数库
8. ioperm 设置端口I/O权限
9. iopl 改变进程I/O权限级别
10. outb 低级端口操作
11. reboot 重新启动
12. swapon 打开交换文件和设备
13. swapoff 关闭交换文件和设备
14. bdflush 控制bdflush守护进程
15. sysfs 取核心支持的文件系统类型
16. sysinfo 取得系统信息
17. adjtimex 调整系统时钟
18. alarm 设置进程的闹钟
19. getitimer 获取计时器值
20. setitimer 设置计时器值
21. gettimeofday 取时间和时区
22. settimeofday 设置时间和时区
23. stime 设置系统日期和时间
24. time 取得系统时间
25. times 取进程运行时间
26. uname 获取当前UNIX系统的名称、版本和主机等信息
27. vhangup 挂起当前终端
28. nfsservctl 对NFS守护进程进行控制
29. vm86 进入模拟8086模式
30. create_module 创建可装载的模块项
31. delete_module 删除可装载的模块项
32. init_module 初始化模块
33. query_module 查询模块信息
34. *get_kernel_syms 取得核心符号,已被query_module代替  



# 四、内存管理

1. brk 改变数据段空间的分配
2. sbrk 参见brk
3. mlock 内存页面加锁
4. munlock 内存页面解锁
5. mlockall 调用进程所有内存页面加锁
6. munlockall 调用进程所有内存页面解锁
7. mmap 映射虚拟内存页
8. munmap 去除内存页映射
9. mremap 重新映射虚拟内存地址
10. msync 将映射内存中的数据写回磁盘
11. mprotect 设置内存映像保护
12. getpagesize 获取页面大小
13. sync 将内存缓冲区数据写回硬盘
14. cacheflush 将指定缓冲区中的内容写回磁盘  



# 五、网络管理

1. getdomainname 取域名
2. setdomainname 设置域名
3. gethostid 获取主机标识号
4. sethostid 设置主机标识号
5. gethostname 获取本主机名称
6. sethostname 设置主机名称  



# 六、socket控制

1. socketcall socket系统调用
2. socket 建立socket
3. bind 绑定socket到端口
4. connect 连接远程主机
5. accept 响应socket连接请求
6. send 通过socket发送信息
7. sendto 发送UDP信息
8. sendmsg 参见send
9. recv 通过socket接收信息
10. recvfrom 接收UDP信息
11. recvmsg 参见recv
12. listen 监听socket端口
13. select 对多路同步I/O进行轮询
14. shutdown 关闭socket上的连接
15. getsockname 取得本地socket名字
16. getpeername 获取通信对方的socket名字
17. getsockopt 取端口设置
18. setsockopt 设置端口参数
19. sendfile 在文件或端口间传输数据
20. socketpair 创建一对已联接的无名socket  



# 七、用户管理

1. getuid 获取用户标识号
2. setuid 设置用户标志号
3. getgid 获取组标识号
4. setgid 设置组标志号
5. getegid 获取有效组标识号
6. setegid 设置有效组标识号
7. geteuid 获取有效用户标识号
8. seteuid 设置有效用户标识号
9. setregid 分别设置真实和有效的的组标识号
10. setreuid 分别设置真实和有效的用户标识号
11. getresgid 分别获取真实的,有效的和保存过的组标识号
12. setresgid 分别设置真实的,有效的和保存过的组标识号
13. getresuid 分别获取真实的,有效的和保存过的用户标识号
14. setresuid 分别设置真实的,有效的和保存过的用户标识号
15. setfsgid 设置文件系统检查时使用的组标识号
16. setfsuid 设置文件系统检查时使用的用户标识号
17. getgroups 获取后补组标志清单
18. setgroups 设置后补组标志清单

# 八、进程间通信
1. ipc 进程间通信总控制调用


## 1、信号
1. sigaction 设置对指定信号的处理方法
2. sigprocmask 根据参数对信号集中的信号执行阻塞/解除阻塞等操作
3. sigpending 为指定的被阻塞信号设置队列
4. sigsuspend 挂起进程等待特定信号
5. signal 参见signal
6. kill 向进程或进程组发信号
7. *sigblock 向被阻塞信号掩码中添加信号,已被sigprocmask代替
8. *siggetmask 取得现有阻塞信号掩码,已被sigprocmask代替
9. *sigsetmask 用给定信号掩码替换现有阻塞信号掩码,已被sigprocmask代替
10. *sigmask 将给定的信号转化为掩码,已被sigprocmask代替
11. *sigpause 作用同sigsuspend,已被sigsuspend代替
12. sigvec 为兼容BSD而设的信号处理函数,作用类似sigaction
13. ssetmask ANSI C的信号处理函数,作用类似sigaction  

## 2、消息
1. msgctl 消息控制操作
2. msgget 获取消息队列
3. msgsnd 发消息
4. msgrcv 取消息  



## 3、管道
1. pipe 创建管道  




## 4、信号量
1. semctl 信号量控制
2. semget 获取一组信号量
3. semop 信号量操作  



## 5、共享内存

1. shmctl 控制共享内存
2. shmget 获取共享内存
3. shmat 连接共享内存
4. shmdt 拆卸共享内存