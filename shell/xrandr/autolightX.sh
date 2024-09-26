#!/bin/sh

FIRSTCHECK=1

while [ 1 ];do
	for i in `cat /sys/class/drm/*/status`;do
		if [ $i = "connected" ];then
			if [[ ${FIRSTCHECK} -eq 0 ]];then
				DISPLAY=:0 xrandr --auto
			fi
			exit 0
		fi
	done
	FIRSTCHECK=0
	sleep 1
done
