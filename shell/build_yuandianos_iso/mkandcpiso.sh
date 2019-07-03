#!/bin/bash

TIME=`date +%Y%m%d%H%M%S`
ISODIR="/data/samba/iso/linux/YuandianOS/${TIME}"


[ -f $ISODIR ] || mkdir $ISODIR
mv YuandianOS*.iso YuandianOS*.iso.md5sum $ISODIR
chown samba.samba $ISODIR -R
