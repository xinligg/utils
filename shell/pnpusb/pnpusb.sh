#!/bin/sh
 
bind_usb() {
  echo -n "$1" | tee /sys/bus/pci/drivers/ehci-pci/bind
  echo -n "$1" | tee /sys/bus/pci/drivers/xhci_hcd/bind
}
 
unbind_usb() {
  echo -n "$1" | tee /sys/bus/pci/drivers/ehci-pci/unbind
  echo -n "$1" | tee /sys/bus/pci/drivers/xhci_hcd/unbind
}
 
DEVICE_NUM=`lspci |grep USB|awk -F' ' '{print$1}'|wc -l`
 
for ((i=1; i <= $DEVICE_NUM; i++))
do
        DEVICE=`lspci |grep USB|awk -F' ' '{print "0000:"$1}'|sed -n ${i}p`
        unbind_usb $DEVICE
        bind_usb $DEVICE
done
