docker查看启动的容器

```bash
$ sudo docker ps
CONTAINER ID   IMAGE           COMMAND       CREATED          STATUS         PORTS 
240f22e326f5   ubuntu:latest   "/bin/bash"   52 minutes ago   Up 4 minutes         
```



输出详情介绍：

**CONTAINER ID:** 容器 ID。

**IMAGE:** 使用的镜像。

**COMMAND:** 启动容器时运行的命令。

**CREATED:** 容器的创建时间。

**STATUS:** 容器状态。

状态有7种：

- created（已创建）
- restarting（重启中）
- running 或 Up（运行中）
- removing（迁移中）
- paused（暂停）
- exited（停止）
- dead（死亡）

**PORTS:** 容器的端口信息和使用的连接类型（tcp\udp）。

**NAMES:** 自动分配的容器名称。



docker查看本地的镜像

```bash
$ sudo docker images
REPOSITORY                     TAG       IMAGE ID       CREATED        SIZE
busybox                        latest    829374d342ae   5 days ago     1.24MB
ubuntu                         latest    17c3d5693947   2 months ago   3.22GB
```

各个选项说明:

- **REPOSITORY：**表示镜像的仓库源
- **TAG：**镜像的标签
- **IMAGE ID：**镜像ID
- **CREATED：**镜像创建时间
- **SIZE：**镜像大小



docker查看所有容器

```bash
$ sudo docker ps -a
CONTAINER ID   IMAGE           COMMAND       CREATED          STATUS                    
b9a598568e49   ubuntu:latest   "/bin/bash"   45 minutes ago   Exited (127) 7 minutes ago
62de3ec6be08   ubuntu:latest   "/bin/bash"   57 minutes ago   Exited (0) 45 minutes ago 
4d284a0bddee   ubuntu:latest   "/bin/bash"   4 hours ago      Exited (0) 57 minutes ago 
38f930572d5c   ubuntu:latest   "/bin/bash"   5 hours ago      Exited (130) 4 hours ago  
0e24d1276a20   ubuntu:latest   "/bin/bash"   5 hours ago      Exited (127) 5 hours ago  
240f22e326f5   ubuntu:latest   "/bin/bash"   6 hours ago      Up 6 hours                
8414455bab46   ubuntu:latest   "/bin/bash"   18 hours ago     Exited (0) 6 hours ago    
28e56eda9cad   ubuntu:latest   "/bin/bash"   19 hours ago     Exited (100) 18 hours ago 
```



docker启动容器

```bash
$ sudo docker run -it 27941809078c /bin/bash
root@c1f13b126111:/# 

#后台运行
$ sudo docker run -itd 27941809078c /bin/bash
```

docker进入容器内部

```bash
$ sudo docker exec -it 6d8a8610dbe3 /bin/bash
root@6d8a8610dbe3:/# 
```

docker停止容器

```bash
$ sudo docker stop 5bec01ee167a
5bec01ee167a
```

docker重启已停止的容器

```bash
$ sudo docker start 14f91350e8ae
14f91350e8ae
```

docker容器文件映射

```bash
docker run -it -v /home/ciei/Desktop/files/code:/home  /bin/bash
```

docker容器端口映射

```bash
sudo docker run -it -v -p /home/ciei/Desktop/files/code:/home 8000:8000 3687eb5ea744 /bin/bash
```

docker容器主机模式

```bash
sudo docker run --name mypthon4 -it -v /home/ciei/Desktop/files/code:/home --network=host 3687eb5ea744 /bin/bash
```

docker导出容器

```bash
$ sudo docker export 6d8a8610dbe3 >mypython
$ ls
Desktop    Downloads          examples.desktop  Music     Pictures  Public     Videos
Documents  eclipse-workspace  linux             mypython  program   Templates  vscode

```

docker导入容器

```bash
$ sudo docker import mypython
sha256:f677fae406d4c77c3e0533001cd9d05a575314a5106404dcc4191c39168e2d3d
```

docker给镜像加名称

```bash
$ sudo docker tag 3a9fe775420e mypython
```

docker删除镜像

```bash
$ sudo docker rmi 1a623ae1b8be
Deleted: sha256:1a623ae1b8be3a53efa7d07e09dceadf6084bc3d6a3a1516e4112d05dfb9fc50
Deleted: sha256:da87c1821ce1bb1ca2ca15da8367c40d3905de4b2241fa80e65808cea8fe419a
```

docker退出容器

```cmd
Ctrl-D				#直接退出

Ctrl-P+Q			#退出容器后台运行
```









