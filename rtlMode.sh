#!/bin/sh

if [ $1 = "0" ]
then
	echo "systemctl stop dump1090" | nc -q 1 192.168.1.22 60000
	sleep 3
	echo "systemctl stop rtl_tcp" | nc -q 1 192.168.1.22 60000
else if [ $1 = "10" ]
then
	echo "systemctl stop dump1090" | nc -q 1 192.168.1.22 60000
	sleep 3
	echo "systemctl start rtl_tcp" | nc -q 1 192.168.1.22 60000
else if [ $1 = "20" ]
then
	echo "systemctl stop rtl_tcp" | nc -q 1 192.168.1.22 60000
	sleep 3
	echo "systemctl start dump1090" | nc -q 1 192.168.1.22 60000
fi
fi
fi

