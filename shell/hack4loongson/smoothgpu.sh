#!/bin/bash

PROGRAM=x11perf

while [ 1 ];do
	average=$((`getgpu | cut -d' ' -f5|cut -d'.' -f1`))
	echo $average
	if [ $average -ge 95 ]; then 
		processid=`pidof  $PROGRAM`
		sign_symbol=1
		if [ -n "$processid" ]; then
			echo "stop $processid"
			kill -SIGSTOP $processid
			echo "usleep $(($RANDOM*10))"
			usleep $(($RANDOM*10))
			echo "continue $processid"
			kill -SIGCONT $processid
		fi
		if [ $sign_symbol -eq 0 ]; then
			sleep 1
		else
			sign_symbol=0
		fi
	fi
done
