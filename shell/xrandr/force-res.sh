#!/bin/sh

if [ $# -lt 2 ] || [ $# -gt 3 ]; then
	echo "Usage: $0 X Y <output>"
	echo "e.g.: $0 1440 900"
	exit 1
fi

if [ $# -eq 2 ]; then
	echo "Change VGA1 by default"
	output=VGA1
else
	output=$3
fi

X=$1
Y=$2
mode="$1x$2_75"
mode_line=`cvt ${X} ${Y} 75 | awk '{$1 = ""; $2 = ""; print}' | tail -1`

xrandr --newmode "$mode" $mode_line
xrandr --addmode $output $mode
xrandr --output $output --mode "$mode" --pos 0x0 --output LVDS1 --mode 1366x768 --pos 0x0
