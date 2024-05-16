#!/bin/sh

#检查文件夹是否挂载
mountpoint -q /data220
mflag=$?
if [ $mflag != 0 ];then
	echo $(date)" 220未挂载,现在执行挂载" >> mount.log
	mount 192.168.1.220:/data/test /data220
	mountpoint -q /data220
	mflag=$?
	if [ $mflag != 0 ];then
		echo $(date)" 220挂载失败" >> mount.log
		#退出
		#exit 1
	fi
fi
