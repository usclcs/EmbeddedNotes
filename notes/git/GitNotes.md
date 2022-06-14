1、初始化git

```bash
git init
```

2、设置git名字

```bash
git config --global user.name "lics"
```

3、设置git邮箱

```bash
git config --global user.email "usclcs@163.com"
```

4、git忽略文件配置  .gitignore

```bash
# Keil Generated output files in the sub-directories .\Listings and .\Objects
*.lst
*.o
*.d
*.crf
*.lnp
*.axf
*.htm
*.build_log.htm
*.dep
*.iex
*.i
*.bin
*.hex
#Keil Project screen layout file
*.uvguix.*
*.uvgui.*
#JLINK file
JLinkLog.txt
```

5、禁用行尾自动转换

```bash
git config --global core.autocrlf false
```



6、

```bash
git remote add origin git@192.168.20.111:lics/gittestproject.git
```



**其他：**

查看本地分支 

```bash
git branch
```

查看远程分支 

```bash
git branch -r
```

查看本地和远程分支 

```bash
git branch -a
```



创建分支

```bash
git branch <name>
```

切换分支

```bash
git switch <name>
```

合并分支

```bash
git merge <name>
```

抓取分支

```bash
git fetch origin <remote name>:<local name>
```

推送分支

```bash
git push origin <name>
```



删除本地分支

```bash
git branch -d <name>
```

强制删除本地分支

```bash
git branch -D <name>
```

删除git远程库分支

```bash
 git push origin --delete <name>
```





```bash

git add .
#提交更新
git commit -m <remarks>
#移除文件(不删除磁盘中文件)
git rm --cached <fileName>

```





```bash
#查看提交日志
git log
#提交更新
git commit -m <remarks>
#撤销操作
git commit --amend
#移除文件(不删除磁盘中文件)
git rm --cached <fileName>

```





```bash
#打标签
git tag <tagName>
#补标签
git tag -a <tagName> <commitNumber>
#推送标签
git push origin <tagName>
#删除标签
git tag -d <tagName>
#删除远程标签
git push origin --delete <tagName>
```





查看gitconfig信息

```bash
git config --list --show-origin  
```

查看当前文件状态

```bash
git config --list --show-origin  
```



回退commit

```bash
git reset --hard 
```

