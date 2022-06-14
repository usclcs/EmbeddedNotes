@echo off 
rem eth 为网卡名称，可在网络连接中查询，如"WLAN" 
set eth="Network4" 
rem ip 为你想更改的IP 
set ip=192.168.1.111 
rem gw 为网关地址 
set gw=192.168.1.1 
rem netmasks 为子网掩码 
set netmasks=255.255.255.0 
rem dns 服务器地址 
set dns=255.255.255.0 

echo changing ip to %ip% 
rem 
netsh interface ip set address %eth% static %ip% %netmasks% %gw% > nul 
netsh interface ip set dns %eth% static 192.168.3.1 > nul 

echo......................... ......................... .........................
echo local IP ipconfig
ipconfig 
echo......................... ......................... .........................
echo Successful!!!! 
rem pause 
close 

