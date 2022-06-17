#!/bin/sh
insmod /lib/modules/5.15.5/extra/eduv7.ko 
echo 0x5ca1ab1e > /dev/eduv7-0 
cat /dev/eduv7-0 