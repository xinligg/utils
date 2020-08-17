#!/bin/sh

while [ 1 ];do
echo 3 > /proc/sys/vm/drop_caches
sleep 10
done
